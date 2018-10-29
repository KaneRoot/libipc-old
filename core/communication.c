#include "communication.h"
#include "utils.h"
#include "error.h"
#include "event.h"
#include <unistd.h>

#include <assert.h>
#include <stdio.h>
#include <errno.h>

void service_path (char *path, const char *sname, int index, int version)
{
    assert (path != NULL);
    assert (sname != NULL);
    memset (path, 0, PATH_MAX);
    snprintf (path, PATH_MAX, "%s/%s-%d-%d", RUNDIR, sname, index, version);
}

int ipc_server_init (char **env
        , struct ipc_service *srv, const char *sname)
{
    if (srv == NULL)
        return IPC_ERROR_WRONG_PARAMETERS;

    // TODO
    //      use env parameters
    //      it will be useful to change some parameters transparently
    //      ex: to get resources from other machines, choosing the
    //          remote with environment variables

    env = env;

    // gets the service path
    service_path (srv->spath, sname, srv->index, srv->version);

    int ret = usock_init (&srv->service_fd, srv->spath);
	if (ret < 0) {
		handle_err ("ipc_server_init", "usock_init ret < 0");
		return -1;
	}

	return 0;
}

int ipc_server_accept (struct ipc_service *srv, struct ipc_client *p)
{
    assert (srv != NULL);
    assert (p != NULL);

    int ret = usock_accept (srv->service_fd, &p->proc_fd);
	if (ret < 0) {
		handle_err ("ipc_server_accept", "usock_accept < 0");
		return -1;
	}

    return 0;
}

// empty the srv structure
int ipc_server_close (struct ipc_service *srv)
{
    usock_close (srv->service_fd);
	int ret = usock_remove (srv->spath);
	ipc_service_empty (srv);
    return ret;
}

int ipc_server_close_client (struct ipc_client *p)
{
    return usock_close (p->proc_fd);
}

int ipc_server_read (const struct ipc_client *p, struct ipc_message *m)
{
    return ipc_message_read (p->proc_fd, m);
}

int ipc_server_write (const struct ipc_client *p, const struct ipc_message *m)
{
    return ipc_message_write (p->proc_fd, m);
}

int ipc_application_connection (char **env
        , struct ipc_service *srv, const char *sname)
{
    // TODO
    //      use env parameters
    //      it will be useful to change some parameters transparently
    //      ex: to get resources from other machines, choosing the
    //          remote with environment variables

    env = env;

    assert (srv != NULL);
    assert (sname != NULL);

    if (srv == NULL) {
        return -1;
    }

    // gets the service path
    service_path (srv->spath, sname, srv->index, srv->version);

    int ret = usock_connect (&srv->service_fd, srv->spath);
	if (ret < 0) {
		handle_err ("ipc_application_connection", "usock_connect ret <= 0");
		return -1;
	}

    return 0;
}

// close the socket
int ipc_application_close (struct ipc_service *srv)
{
    return usock_close (srv->service_fd);
}

int ipc_application_read (struct ipc_service *srv, struct ipc_message *m)
{   
    return ipc_message_read (srv->service_fd, m);
}

int ipc_application_write (struct ipc_service *srv, const struct ipc_message *m)
{
    return ipc_message_write (srv->service_fd, m);
}


/*calculer le max filedescriptor*/
static int get_max_fd_from_ipc_clients_ (struct ipc_clients *clients)
{
    int i;
    int max = 0;

    for (i = 0; i < clients->size; i++ ) {
        if (clients->clients[i]->proc_fd > max) {
            max = clients->clients[i]->proc_fd;
        } 
    }

    return max;
}

static int get_max_fd_from_ipc_services_ (struct ipc_services *services)
{
    int i;
    int max = 0;

    for (i = 0; i < services->size; i++ ) {
        if (services->services[i]->service_fd > max) {
            max = services->services[i]->service_fd;
        } 
    }

    return max;
}

/*
 * ipc_server_select prend en parametre
 *  * un tableau de client qu'on écoute
 *  * le service qui attend de nouvelles connexions
 *  * un tableau de client qui souhaitent parler
 *
 *  0 = OK
 * -1 = error
 */

int ipc_server_select (struct ipc_clients *clients, struct ipc_service *srv
        , struct ipc_clients *active_clients, int *new_connection)
{
	*new_connection = 0;
    assert (clients != NULL);
    assert (active_clients != NULL);

    // delete previous read active_clients array
    ipc_clients_free (active_clients);

    int i, j;
    /* master file descriptor list */
    fd_set master;
    fd_set readf;

    /* maximum file descriptor number */
    int fdmax;
    /* listening socket descriptor */
    int listener = srv->service_fd;

    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&readf);
    /* add the listener to the master set */
    FD_SET(listener, &master);

    for (i=0; i < clients->size; i++) {
        FD_SET(clients->clients[i]->proc_fd, &master);
    }

    /* keep track of the biggest file descriptor */
    fdmax = get_max_fd_from_ipc_clients_ (clients) > srv->service_fd ? get_max_fd_from_ipc_clients_ (clients) : srv->service_fd; 

	readf = master;
	if(select(fdmax+1, &readf, NULL, NULL, NULL) == -1) {
		perror("select");
		return -1;
	}

	for (i = 0; i <= fdmax; i++) {
		if (FD_ISSET(i, &readf)) {
			if (i == listener) {
				*new_connection = 1;
			} else {
				for(j = 0; j < clients->size; j++) {
					if(i == clients->clients[j]->proc_fd ) {
						ipc_client_add (active_clients, clients->clients[j]);
					}
				}
			}
		}
	}

	return 0;
}

/*
 * ipc_application_select prend en parametre
 *  * un tableau de server qu'on écoute
 *  * le service qui attend de nouvelles connexions
 *  * un tableau de client qui souhaitent parler
 *
 *  0 = OK
 * -1 = error
 */

int ipc_application_select (struct ipc_services *services, struct ipc_services *active_services)
{
    assert (services != NULL);
    assert (active_services != NULL);

    // delete previous read active_services array
    ipc_services_free (active_services);

    int i, j;
    /* master file descriptor list */
    fd_set master;
    fd_set readf;

    /* maximum file descriptor number */
    int fdmax;

    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&readf);

    for (i=0; i < services->size; i++) {
        FD_SET(services->services[i]->service_fd, &master);
    }

    /* keep track of the biggest file descriptor */
    fdmax = get_max_fd_from_ipc_services_ (services);

	readf = master;
	if(select(fdmax+1, &readf, NULL, NULL, NULL) == -1) {
		perror("select");
		return -1;
	}

	for (i = 0; i <= fdmax; i++) {
		if (FD_ISSET(i, &readf)) {
			for(j = 0; j < services->size; j++) {
				if(i == services->services[j]->service_fd ) {
					ipc_service_add (active_services, services->services[j]);
				}
			}
		}
	}

	return 0;
}

int handle_new_connection (struct ipc_service *srv
		, struct ipc_clients *clients
		, struct ipc_client **new_client)
{
    *new_client = malloc(sizeof(struct ipc_client));
    memset(*new_client, 0, sizeof(struct ipc_client));

    if (ipc_server_accept (srv, *new_client) < 0) {
        handle_error("server_accept < 0");
		return 1;
    } else {
        printf("new connection\n");
    }

    if (ipc_client_add (clients, *new_client) < 0) {
        handle_error("ipc_client_add < 0");
		return 1;
    }

	return 0;
}

int ipc_service_poll_event (struct ipc_clients *clients, struct ipc_service *srv
        , struct ipc_event *event)
{
    assert (clients != NULL);

	IPC_EVENT_CLEAN(event);

    int i, j;
    /* master file descriptor list */
    fd_set master;
    fd_set readf;

    /* maximum file descriptor number */
    int fdmax;
    /* listening socket descriptor */
    int listener = srv->service_fd;

    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&readf);
    /* add the listener to the master set */
    FD_SET(listener, &master);

    for (i=0; i < clients->size; i++) {
        FD_SET(clients->clients[i]->proc_fd, &master);
    }

    /* keep track of the biggest file descriptor */
    fdmax = get_max_fd_from_ipc_clients_ (clients) > srv->service_fd ? get_max_fd_from_ipc_clients_ (clients) : srv->service_fd; 

	readf = master;
	if(select(fdmax+1, &readf, NULL, NULL, NULL) == -1) {
		perror("select");
		return -1;
	}

	for (i = 0; i <= fdmax; i++) {
		if (FD_ISSET(i, &readf)) {
			if (i == listener) {
				// connection
				struct ipc_client *new_client = NULL;
				handle_new_connection (srv, clients, &new_client);
				IPC_EVENT_SET (event, IPC_EVENT_TYPE_CONNECTION, NULL, new_client);
				return 0;
			} else {
				for(j = 0; j < clients->size; j++) {
					if(i == clients->clients[j]->proc_fd ) {
						// listen to what they have to say (disconnection or message)
						// then add a client to `event`, the ipc_event structure
						int ret = 0;
						struct ipc_message *m = NULL;
						m = malloc (sizeof(struct ipc_message));
						if (m == NULL) {
							return IPC_ERROR_NOT_ENOUGH_MEMORY;
						}
						memset (m, 0, sizeof (struct ipc_message));

						// current talking client
						struct ipc_client *pc = clients->clients[j];
						ret = ipc_server_read (pc, m);
						if (ret < 0) {
							handle_err ("ipc_service_poll_event", "ipc_server_read < 0");
							ipc_message_empty (m);
							free (m);

							IPC_EVENT_SET(event, IPC_EVENT_TYPE_ERROR, NULL, pc);
							return IPC_ERROR_READ;
						}

						// disconnection: close the client then delete it from clients
						if (ret == 1) {
							if (ipc_server_close_client (pc) < 0) {
								handle_err( "ipc_service_poll_event", "ipc_server_close_client < 0");
							}
							if (ipc_client_del (clients, pc) < 0) {
								handle_err( "ipc_service_poll_event", "ipc_client_del < 0");
							}
							ipc_message_empty (m);
							free (m);

							IPC_EVENT_SET(event, IPC_EVENT_TYPE_DISCONNECTION, NULL, pc);

							// warning: do not forget to free the ipc_client structure
							return 0;
						}

						// we received a new message from a client
						IPC_EVENT_SET (event, IPC_EVENT_TYPE_MESSAGE, m, pc);
						return 0;
					}
				}
			}
		}
	}

	return 0;
}

int ipc_application_poll_event_ (struct ipc_services *services, struct ipc_event *event, int interactive)
{
    assert (services != NULL);

	IPC_EVENT_CLEAN(event);

    int i, j;
    /* master file descriptor list */
    fd_set master;
    fd_set readf;

    /* maximum file descriptor number */
    int fdmax;

    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&readf);

	if (interactive) {
		FD_SET(0, &master);
	}

    for (i=0; i < services->size; i++) {
        FD_SET(services->services[i]->service_fd, &master);
    }

    /* keep track of the biggest file descriptor */
    fdmax = get_max_fd_from_ipc_services_ (services);

	readf = master;
	if(select(fdmax+1, &readf, NULL, NULL, NULL) == -1) {
		perror("select");
		return -1;
	}

	for (i = 0; i <= fdmax; i++) {
		if (FD_ISSET(i, &readf)) {

			// interactive: input on stdin
			if (i == 0) {
				// XXX: by default, message type is 0
				struct ipc_message *m = malloc (sizeof (struct ipc_message));
				if (m == NULL) {
					return IPC_ERROR_NOT_ENOUGH_MEMORY;
				}
				memset (m, 0, sizeof (struct ipc_message));
				m->payload = malloc (IPC_MAX_MESSAGE_SIZE);

				m->length = read(0, m->payload , IPC_MAX_MESSAGE_SIZE);
				IPC_EVENT_SET(event, IPC_EVENT_TYPE_STDIN, m, NULL);
				return 0;
			}

			for(j = 0; j < services->size; j++) {
				if(i == services->services[j]->service_fd ) {
					// listen to what they have to say (disconnection or message)
					// then add a client to `event`, the ipc_event structure
					int ret = 0;
					struct ipc_message *m = NULL;
					m = malloc (sizeof(struct ipc_message));
					if (m == NULL) {
						return IPC_ERROR_NOT_ENOUGH_MEMORY;
					}
					memset (m, 0, sizeof (struct ipc_message));

					// current talking client
					struct ipc_service *ps = services->services[j];
					ret = ipc_application_read (ps, m);
					if (ret < 0) {
						handle_err ("ipc_application_poll_event", "ipc_application_read < 0");
						ipc_message_empty (m);
						free (m);

						IPC_EVENT_SET(event, IPC_EVENT_TYPE_ERROR, NULL, ps);
						return IPC_ERROR_READ;
					}

					// disconnection: close the service
					if (ret == 1) {
						if (ipc_application_close (ps) < 0) {
							handle_err( "ipc_application_poll_event", "ipc_application_close < 0");
						}
						if (ipc_service_del (services, ps) < 0) {
							handle_err( "ipc_application_poll_event", "ipc_service_del < 0");
						}
						ipc_message_empty (m);
						free (m);

						IPC_EVENT_SET(event, IPC_EVENT_TYPE_DISCONNECTION, NULL, ps);

						// warning: do not forget to free the ipc_client structure
						return 0;
					}

					// we received a new message from a client
					IPC_EVENT_SET (event, IPC_EVENT_TYPE_MESSAGE, m, ps);
					return 0;

				}
			}
		}
	}

	return 0;
}

int ipc_application_poll_event (struct ipc_services *services, struct ipc_event *event) {
	return ipc_application_poll_event_ (services, event, 0);
}

int ipc_application_peek_event (struct ipc_services *services, struct ipc_event *event) {
	return ipc_application_poll_event_ (services, event, 1);
}
