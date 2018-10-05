#include "communication.h"
#include "usocket.h"
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

int ipc_server_init (int argc, char **argv, char **env
        , struct ipc_service *srv, const char *sname)
{
    if (srv == NULL)
        return ER_PARAMS;

    // TODO
    //      use the argc, argv and env parameters
    //      it will be useful to change some parameters transparently
    //      ex: to get resources from other machines, choosing the
    //          remote with environment variables

    argc = argc;
    argv = argv;
    env = env;

    // gets the service path
    service_path (srv->spath, sname, srv->index, srv->version);

    return usock_init (&srv->service_fd, srv->spath);
}

int ipc_server_accept (struct ipc_service *srv, struct ipc_client *p)
{
    assert (srv != NULL);
    assert (p != NULL);

    usock_accept (srv->service_fd, &p->proc_fd);

    struct ipc_message m_con;
    memset (&m_con, 0, sizeof (struct ipc_message));
    ipc_server_read (p, &m_con);
    // TODO: handle the parameters in the first message
    ipc_message_free (&m_con);

    struct ipc_message m_ack;
    memset (&m_ack, 0, sizeof (struct ipc_message));
    ipc_message_format_ack (&m_ack, NULL, 0);
    ipc_server_write (p, &m_ack);
    ipc_message_free (&m_ack);

    return 0;
}

int ipc_server_close (struct ipc_service *srv)
{
    usock_close (srv->service_fd);
    return usock_remove (srv->spath);
}

int ipc_server_close_client (struct ipc_client *p)
{
    // struct ipc_message m_ack_dis;
    // memset (&m_ack_dis, 0, sizeof (struct ipc_message));
    // m_ack_dis.type = MSG_TYPE_CLOSE;

    // if (ipc_message_write (p->proc_fd, &m_ack_dis) < 0) {
    //     handle_err ("server_close_client", "ipc_message_write < 0");
    // }

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

int ipc_application_connection (int argc, char **argv, char **env
        , struct ipc_service *srv, const char *sname
        , const char *connectionstr, size_t msize)
{
    argc = argc;
    argv = argv;
    env = env;

    assert (srv != NULL);
    assert (sname != NULL);

    if (srv == NULL) {
        return -1;
    }

    // gets the service path
    service_path (srv->spath, sname, srv->index, srv->version);

    usock_connect (&srv->service_fd, srv->spath);

    // send connection string and receive acknowledgement
    struct ipc_message m_con;
    memset (&m_con, 0, sizeof (struct ipc_message));

    // format the connection msg
    if (ipc_message_format_con (&m_con, connectionstr, msize) < 0) {
        handle_err ("application_connection", "ipc_message_format_con");
        return -1;
    }

    // send connection msg
    ipc_application_write (srv, &m_con);
    ipc_message_free (&m_con);

    // receive ack msg
    struct ipc_message m_ack;
    memset (&m_ack, 0, sizeof (struct ipc_message));
    ipc_application_read (srv, &m_ack);

    assert (m_ack.type == MSG_TYPE_ACK);
    assert (m_ack.length == 0);
    ipc_message_free (&m_ack);

    return 0;
}

// send a CLOSE message then close the socket
int ipc_application_close (struct ipc_service *srv)
{
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
    m.type = MSG_TYPE_CLOSE;
    if (ipc_message_write (srv->service_fd, &m) < 0) {
        handle_err ("application_close", "ipc_message_write < 0");
    }

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
    ipc_client_array_free (active_clients);

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
