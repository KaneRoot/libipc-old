#include "communication.h"
#include "utils.h"
#include "error.h"

#include <assert.h>
#include <stdio.h>
#include <errno.h>

void service_path (char *path, const char *sname, int index, int version)
{
    assert (path != NULL);
    assert (sname != NULL);
    memset (path, 0, PATH_MAX);
    snprintf (path, PATH_MAX, "%s/%s-%d-%d", TMPDIR, sname, index, version);
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

// send a CLOSE message then close the socket
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
static int getMaxFd(struct ipc_clients *clients)
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
    fdmax = getMaxFd(clients) > srv->service_fd ? getMaxFd(clients) : srv->service_fd; 

	// printf ("loop ipc_server_select main_loop\n");
	readf = master;
	if(select(fdmax+1, &readf, NULL, NULL, NULL) == -1) {
		perror("select");
		return -1;
	}

	/*run through the existing connections looking for data to be read*/
	for (i = 0; i <= fdmax; i++) {
		// printf ("loop ipc_server_select inner loop\n");
		if (FD_ISSET(i, &readf)) {
			if (i == listener) {
				*new_connection = 1;
			} else {
				for(j = 0; j < clients->size; j++) {
					// printf ("loop ipc_server_select inner inner loop\n");
					if(i == clients->clients[j]->proc_fd ) {
						ipc_client_add (active_clients, clients->clients[j]);
					}
				}
			}
		}
	}

	return 0;
}

/*calculer le max filedescriptor*/
static int getMaxFdServices(struct ipc_services *services)
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
    fdmax = getMaxFdServices(services);

	// printf ("loop ipc_server_select main_loop\n");
	readf = master;
	if(select(fdmax+1, &readf, NULL, NULL, NULL) == -1) {
		perror("select");
		return -1;
	}

	/*run through the existing connections looking for data to be read*/
	for (i = 0; i <= fdmax; i++) {
		// printf ("loop ipc_server_select inner loop\n");
		if (FD_ISSET(i, &readf)) {
			for(j = 0; j < services->size; j++) {
				// printf ("loop ipc_server_select inner inner loop\n");
				if(i == services->services[j]->service_fd ) {
					ipc_service_add (active_services, services->services[j]);
				}
			}
		}
	}

	return 0;
}
