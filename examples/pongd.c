#include "../src/ipc.h"
#include <signal.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define PONGD_SERVICE_NAME "pong"
#define PONGD_VERBOSE

#define PRINTERR(ret,msg) {\
	const char * err = ipc_errors_get (ret);\
	fprintf(stderr, "error while %s: %s\n", msg, err);\
}

int cpt = 0;

struct ipc_connection_info *srv = NULL;
struct ipc_connection_infos *clients = NULL;


void main_loop ()
{
    enum ipc_errors ret = 0; 

	clients = malloc (sizeof (struct ipc_connection_infos));
	memset(clients, 0, sizeof(struct ipc_connection_infos));

	struct ipc_event event;
	memset(&event, 0, sizeof (struct ipc_event));
	event.type = IPC_EVENT_TYPE_NOT_SET;

    while(1) {
		// ipc_service_poll_event provides one event at a time
		// warning: event->m is free'ed if not NULL
		// printf ("before wait event\n"); // TODO remove
		ret = ipc_wait_event (clients, srv, &event);
		// printf ("after wait event\n"); // TODO remove
		if (ret != IPC_ERROR_NONE && ret != IPC_ERROR_CLOSED_RECIPIENT) {
			PRINTERR(ret,"service poll event");

			// the application will shut down, and close the service
			ret = ipc_server_close (srv);
			if (ret != IPC_ERROR_NONE) {
				PRINTERR(ret,"server close");
			}
			exit (EXIT_FAILURE);
		}

		switch (event.type) {
			case IPC_EVENT_TYPE_CONNECTION:
				{
					cpt++;
#ifdef PONGD_VERBOSE
					printf ("connection: %d clients connected, new client is %d\n"
							, cpt, (event.origin)->fd);
#endif
				};
				break;
			case IPC_EVENT_TYPE_DISCONNECTION:
				{
					cpt--;
#ifdef PONGD_VERBOSE
					printf ("disconnection: %d clients remaining\n", cpt);
#endif

					// free the ipc_client structure
					free (event.origin);
				};
				break;
			case IPC_EVENT_TYPE_MESSAGE:
			   	{
					struct ipc_message *m = event.m;
					if (m->length > 0) {
#ifdef PONGD_VERBOSE
						printf ("message received (type %d): %.*s\n", m->type, m->length, m->payload);
#endif
					}

					ret = ipc_write (event.origin, m);
					if (ret != IPC_ERROR_NONE) {
						PRINTERR(ret,"server write");
					}
				};
				break;
			case IPC_EVENT_TYPE_ERROR:
			   	{
					cpt--;
					fprintf (stderr, "a problem happened with client %d (now disconnected)", (event.origin)->fd);
					fprintf (stderr, ", %d clients remaining\n", cpt);

					// free the ipc_client structure
					free (event.origin);
				};
				break;
			default :
				{
					fprintf (stderr, "there must be a problem, event not set\n");
				};
		}
    }

	// should never go there
	exit (EXIT_FAILURE);
}


void exit_program(int signal)
{
	printf("Quitting, signal: %d\n", signal);

	// free remaining clients
	for (int i = 0; i < clients->size ; i++) {
		struct ipc_connection_info *cli = clients->cinfos[i];
		if (cli != NULL) {
			free (cli);
		}
		clients->cinfos[i] = NULL;
	}

	ipc_connections_free (clients);
	free (clients);


    // the application will shut down, and close the service
	enum ipc_errors ret = ipc_server_close (srv);
    if (ret != IPC_ERROR_NONE) {
		PRINTERR(ret,"server close");
    }
	free (srv);

	exit(EXIT_SUCCESS);
}

/*
 * service ping-pong: send back everything sent by the clients
 * stop the program on SIG{TERM,INT,ALRM,USR{1,2},HUP} signals
 */

int main(int argc, char * argv[], char **env)
{
	argc = argc; // warnings
	argv = argv; // warnings

	printf ("pid = %d\n", getpid ());

	srv = malloc (sizeof (struct ipc_connection_info));
	if (srv == NULL) {
		exit (EXIT_FAILURE);
	}
    memset (srv, 0, sizeof (struct ipc_connection_info));
    srv->type = '\0';
    srv->index = 0;
    srv->version = 0;
    srv->fd = 0;
    srv->spath = NULL;

	enum ipc_errors ret = ipc_server_init (env, srv, PONGD_SERVICE_NAME);
    if (ret != IPC_ERROR_NONE) {
		PRINTERR(ret,"server init");
        return EXIT_FAILURE;
    }
    printf ("Listening on %s.\n", srv->spath);

    printf("MAIN: server created\n" );

	signal (SIGHUP, exit_program);
	signal (SIGALRM, exit_program);
	signal (SIGUSR1, exit_program);
	signal (SIGUSR2, exit_program);
	signal (SIGTERM, exit_program);
	signal (SIGINT, exit_program);

    // the service will loop until the end of time, or a signal
    main_loop ();

	// main_loop should not return
    return EXIT_FAILURE;
}
