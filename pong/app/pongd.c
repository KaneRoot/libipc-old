#include "../../core/communication.h"
#include "../../core/client.h"
#include "../../core/error.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define PONGD_SERVICE_NAME "pongd"

int cpt = 0;

struct ipc_service *srv = 0;

void handle_new_connection (struct ipc_clients *clients)
{
    struct ipc_client *p = malloc(sizeof(struct ipc_client));
    memset(p, 0, sizeof(struct ipc_client));

    if (ipc_server_accept (srv, p) < 0) {
        handle_error("server_accept < 0");
    } else {
        printf("new connection\n");
    }

    if (ipc_client_add (clients, p) < 0) {
        handle_error("ipc_client_add < 0");
    }

    cpt++;
    printf ("%d client(s)\n", cpt);
}

void handle_new_msg (struct ipc_clients *clients, struct ipc_clients *clients_talking)
{
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
    int i;
    for (i = 0; i < clients_talking->size; i++) {
        // printf ("loop handle_new_msg\n");
        if (ipc_server_read (clients_talking->clients[i], &m) < 0) {
            handle_error("server_read < 0");
        }

        // close the client then delete it from the client array
        if (m.type == MSG_TYPE_CLOSE) {
            cpt--;
            printf ("disconnection => %d client(s) remaining\n", cpt);

            if (ipc_server_close_client (clients_talking->clients[i]) < 0)
                handle_err( "handle_new_msg", "server_close_client < 0");
            if (ipc_client_del (clients, clients_talking->clients[i]) < 0)
                handle_err( "handle_new_msg", "ipc_client_del < 0");
            if (ipc_client_del (clients_talking, clients_talking->clients[i]) < 0)
                handle_err( "handle_new_msg", "ipc_client_del < 0");
            i--;
            continue;
		}

		if (m.type == MSG_TYPE_SERVER_CLOSE) {
			if (ipc_server_close (srv) < 0) {
				handle_error("server_close < 0");
			}
			exit (0);
		}

        printf ("new message : %s", m.payload);
        if (ipc_server_write (clients_talking->clients[i], &m) < 0) {
            handle_err( "handle_new_msg", "server_write < 0");
        }
    }
}

/*
 * main loop
 *
 * accept new application connections
 * read a message and send it back
 * close a connection if MSG_TYPE_CLOSE received
 */

void main_loop ()
{
    int i, ret = 0; 

    struct ipc_clients clients;
    memset(&clients, 0, sizeof(struct ipc_clients));

    struct ipc_clients clients_talking;
    memset(&clients_talking, 0, sizeof(struct ipc_clients));

    while(1) {
		int new_connection = 0;
        ret = ipc_server_select (&clients, srv, &clients_talking, &new_connection);
		if (ret < 0) {
			handle_error("ipc_server_select < 0");
		}

        if (new_connection) {
            handle_new_connection (&clients);
        }

		if (clients_talking.size > 0) {
            handle_new_msg (&clients, &clients_talking);
        }
        ipc_client_array_free (&clients_talking);
    }

    for (i = 0; i < clients.size; i++) {
        if (ipc_server_close_client (clients.clients[i]) < 0) {
            handle_error( "server_close_client < 0");
        }
    }
}


/*
 * service ping-pong
 *
 * 1. creates the named pipe /tmp/<service>, then listens
 * 2. opens the named pipes in & out
 * 3. talks with the (test) program
 * 4. closes the test program named pipes
 * 5. removes the named pipe /tmp/<service>
 */

int main(int argc, char * argv[], char **env)
{
	srv = malloc (sizeof (struct ipc_service));
	if (srv == NULL) {
		exit (1);
	}
    memset (srv, 0, sizeof (struct ipc_service));
    srv->index = 0;
    srv->version = 0;

    // unlink("/tmp/ipc/pongd-0-0");

    if (ipc_server_init (argc, argv, env, srv, PONGD_SERVICE_NAME) < 0) {
        handle_error("server_init < 0");
        return EXIT_FAILURE;
    }
    printf ("Listening on %s.\n", srv->spath);

    printf("MAIN: server created\n" );

    // the service will loop until the end of time, a specific message, a signal
    main_loop ();

    // the application will shut down, and remove the service named pipe
    if (ipc_server_close (srv) < 0) {
        handle_error("server_close < 0");
    }

    return EXIT_SUCCESS;
}
