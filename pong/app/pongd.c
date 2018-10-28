#include "../../core/ipc.h"
#include <signal.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define PONGD_SERVICE_NAME "pongd"

int cpt = 0;

struct ipc_service *srv = 0;
struct ipc_clients *clients;


void main_loop ()
{
    int ret = 0; 

	clients = malloc (sizeof (struct ipc_clients));
	memset(clients, 0, sizeof(struct ipc_clients));

	struct ipc_event event;
	memset(&event, 0, sizeof (struct ipc_event));
	event.type = IPC_EVENT_TYPE_NOT_SET;

    while(1) {
		// ipc_service_loop provides one event at a time
		// warning: event->m is free'ed if not NULL
		ret = ipc_service_loop (clients, srv, &event);
		if (ret != 0) {
			handle_error("ipc_service_loop != 0");
			// the application will shut down, and close the service
			if (ipc_server_close (srv) < 0) {
				handle_error("ipc_server_close < 0");
			}
			exit (EXIT_FAILURE);
		}

		switch (event.type) {
			case IPC_EVENT_TYPE_CONNECTION:
				{
					cpt++;
					printf ("connection: %d clients connected\n", cpt);
					printf ("new client has the fd %d\n", ((struct ipc_client*) event.origin)->proc_fd);
				};
				break;
			case IPC_EVENT_TYPE_DISCONNECTION:
				{
					cpt--;
					printf ("disconnection: %d clients remaining\n", cpt);

					// free the ipc_client structure
					free (event.origin);
				};
				break;
			case IPC_EVENT_TYPE_MESSAGE:
			   	{
					struct ipc_message *m = event.m;
					if (m->length > 0) {
						printf ("message received (type %d): %.*s\n", m->type, m->length, m->payload);
					}
					if (ipc_server_write (event.origin, m) < 0) {
						handle_err( "handle_new_msg", "server_write < 0");
					}
				};
				break;
			case IPC_EVENT_TYPE_ERROR:
			   	{
					fprintf (stderr, "a problem happened with client %d\n"
							, ((struct ipc_client*) event.origin)->proc_fd);
				};
				break;
			default :
				{
					fprintf (stderr, "there must be a problem, event not set\n");
				};
		}
    }

	// should never go there
	exit (1);
}


void exit_program(int signal)
{
	printf("Quitting, signal: %d\n", signal);

	// free remaining clients
	for (int i = 0; i < clients->size ; i++) {
		struct ipc_client *cli = clients->clients[i];
		// TODO: replace with specific ipc_client_empty function
		if (cli != NULL) {
			// ipc_client_empty (cli);
			free (cli);
		}
		clients->clients[i] = NULL;
	}

	ipc_clients_free (clients);
	free (clients);


    // the application will shut down, and close the service
    if (ipc_server_close (srv) < 0) {
        handle_error("ipc_server_close < 0");
    }
	free (srv);

	exit(EXIT_SUCCESS);
}

/*
 * service ping-pong: send back everything sent by the clients
 * stop the program on SIGTERM, SIGALRM, SIGUSR{1,2}, SIGHUP signals
 */

int main(int argc, char * argv[], char **env)
{
	argc = argc; // warnings
	argv = argv; // warnings

	printf ("pid = %d\n", getpid ());

	srv = malloc (sizeof (struct ipc_service));
	if (srv == NULL) {
		exit (1);
	}
    memset (srv, 0, sizeof (struct ipc_service));
    srv->index = 0;
    srv->version = 0;

    // unlink("/tmp/ipc/pongd-0-0");

    if (ipc_server_init (env, srv, PONGD_SERVICE_NAME) < 0) {
        handle_error("ipc_server_init < 0");
        return EXIT_FAILURE;
    }
    printf ("Listening on %s.\n", srv->spath);

    printf("MAIN: server created\n" );

	signal (SIGHUP, exit_program);
	signal (SIGALRM, exit_program);
	signal (SIGUSR1, exit_program);
	signal (SIGUSR2, exit_program);
	signal (SIGTERM, exit_program);

    // the service will loop until the end of time, or a signal
    main_loop ();

	// main_loop should not return
    return EXIT_FAILURE;
}
