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
	int ret = 0;
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
    int i = 0;
    for (i = 0; i < clients_talking->size; i++) {
        // printf ("loop handle_new_msg\n");

		// current talking client
		struct ipc_client *pc = clients_talking->clients[i];

		ret = ipc_server_read (pc, &m);
        if (ret < 0) {
            handle_error("server_read < 0");
        }

        // close the client then delete it from clients
		if (ret == 1) {
            cpt--;
            printf ("disconnection => %d client(s) remaining\n", cpt);

            if (ipc_server_close_client (pc) < 0)
                handle_err( "handle_new_msg", "server_close_client < 0");
            if (ipc_client_del (clients, pc) < 0)
                handle_err( "handle_new_msg", "ipc_client_del < 0");
            if (ipc_client_del (clients_talking, pc) < 0)
                handle_err( "handle_new_msg", "ipc_client_del < 0");
            i--;

			// free the ipc_client structure
			free (pc);
            continue;
		}

		if (m.type == MSG_TYPE_SERVER_CLOSE) {
			// free remaining clients
			for (int y = 0; y < clients->size ; y++) {
				struct ipc_client *cli = clients->clients[y];
				// TODO: replace with specific ipc_client_empty function
				if (cli != NULL)
					free (cli);
				clients->clients[y] = NULL;
			}

			ipc_clients_free (clients);
			ipc_clients_free (clients_talking);

			if (ipc_server_close (srv) < 0) {
				handle_error("server_close < 0");
			}

			ipc_message_empty (&m);
			free (srv);
			exit (0);
		}

        printf ("new message : %s", m.payload);
        if (ipc_server_write (pc, &m) < 0) {
            handle_err( "handle_new_msg", "server_write < 0");
        }

		// empty the message structure
		ipc_message_empty (&m);
		memset (&m, 0, sizeof m);
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
    int ret = 0; 

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
        ipc_clients_free (&clients_talking);
    }
	// should never go there
	exit (1);
}


/*
 * service ping-pong
 *
 * 1. creates the unix socket /run/ipc/<service>.sock, then listens
 * 2. listen for new clients
 * 3. then accept a new client, and send back everything it sends
 * 4. close any client that closes its socket
 *
 * and finally, stop the program once a client sends a SERVER CLOSE command
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
