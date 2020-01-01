#include "../src/ipc.h"
#include <signal.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define PONGD_SERVICE_NAME "pong"

#define PRINTERR(ret,msg) {\
	const char * err = ipc_errors_get (ret.error_code);\
	fprintf(stderr, "error while %s: %s\n", msg, err);\
}

int cpt = 0;

struct ipc_connection_info *srv = 0;
struct ipc_connection_infos *clients;


void main_loop ()
{
	SECURE_DECLARATION(struct ipc_error, ret);

	clients = malloc (sizeof (struct ipc_connection_infos));
	memset(clients, 0, sizeof(struct ipc_connection_infos));

	SECURE_DECLARATION(struct ipc_event,event);

	long timer = 10;

    while(1) {
		// ipc_wait_event provides one event at a time
		// warning: event->m is free'ed if not NULL
		ret = ipc_wait_event (clients, srv, &event, &timer);
		if (ret.error_code != IPC_ERROR_NONE && ret.error_code != IPC_ERROR_CLOSED_RECIPIENT) {
			PRINTERR(ret,"service poll event");

			// the application will shut down, and close the service
			TEST_IPC_Q(ipc_server_close (srv), EXIT_FAILURE);
			exit (EXIT_FAILURE);
		}

		switch (event.type) {
			case IPC_EVENT_TYPE_TIMER: {
					fprintf(stderr, "time up!\n");
					timer = 10;
				};
				break;
			case IPC_EVENT_TYPE_CONNECTION: {
					cpt++;
					printf ("connection: %d clients connected\n", cpt);
					printf ("new client has the fd %d\n", ((struct ipc_connection_info*) event.origin)->fd);
				};
				break;
			case IPC_EVENT_TYPE_DISCONNECTION:
				{
					cpt--;
					printf ("disconnection: %d clients remaining\n", cpt);

					// free the ipc_connection_info structure
					free (event.origin);
				};
				break;
			case IPC_EVENT_TYPE_MESSAGE:
			   	{
					struct ipc_message *m = event.m;
					if (m->length > 0) {
						printf ("message received (type %d): %.*s\n", m->type, m->length, m->payload);
					}

					ret = ipc_write (event.origin, m);
					if (ret.error_code != IPC_ERROR_NONE) {
						PRINTERR(ret,"server write");
					}
				};
				break;
			case IPC_EVENT_TYPE_ERROR:
			   	{
					fprintf (stderr, "a problem happened with client %d\n"
							, ((struct ipc_connection_info*) event.origin)->fd);
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
	for (size_t i = 0; i < clients->size ; i++) {
		struct ipc_connection_info *cli = clients->cinfos[i];
		if (cli != NULL) {
			free (cli);
		}
		clients->cinfos[i] = NULL;
	}

	ipc_connections_free (clients);
	free (clients);


    // the application will shut down, and close the service
	TEST_IPC_Q(ipc_server_close (srv), EXIT_FAILURE);
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
		exit (1);
	}
    memset (srv, 0, sizeof (struct ipc_connection_info));
    srv->index = 0;
    srv->version = 0;

	TEST_IPC_Q(ipc_server_init (env, srv, PONGD_SERVICE_NAME), EXIT_FAILURE);

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
