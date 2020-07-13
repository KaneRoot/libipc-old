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

struct ipc_ctx *ctx;


void main_loop ()
{
	SECURE_DECLARATION(struct ipc_error, ret);
	SECURE_DECLARATION(struct ipc_event, event);

	int timer = 10000;

    while(1) {
		// ipc_wait_event provides one event at a time
		// warning: event->m is free'ed if not NULL
		ret = ipc_wait_event (ctx, &event, &timer);
		if (ret.error_code != IPC_ERROR_NONE && ret.error_code != IPC_ERROR_CLOSED_RECIPIENT) {
			PRINTERR(ret,"service poll event");

			// the application will shut down, and close the service
			TEST_IPC_Q(ipc_close_all (ctx), EXIT_FAILURE);
			exit (EXIT_FAILURE);
		}

		switch (event.type) {
			case IPC_EVENT_TYPE_TIMER: {
					fprintf(stderr, "time up!\n");
					timer = 10000;
				};
				break;
			case IPC_EVENT_TYPE_CONNECTION: {
					cpt++;
					// printf ("new connection (fd %d): %d ctx connected\n", event.origin, cpt);
				};
				break;
			case IPC_EVENT_TYPE_DISCONNECTION:
				{
					cpt--;
					// printf ("disconnection (fd %d): %d clients remaining\n", event.origin, cpt);
				};
				break;
			case IPC_EVENT_TYPE_MESSAGE:
			   	{
					struct ipc_message *m = event.m;
					if (m->length > 0) {
						// printf ("message received (type %d): %.*s\n", m->type, m->length, m->payload);
					}

					m->fd = event.origin;
					ret = ipc_write (ctx, m);
					if (ret.error_code != IPC_ERROR_NONE) {
						PRINTERR(ret,"server write");
					}
				};
				break;
			case IPC_EVENT_TYPE_TX:
				{
					// printf ("a message was sent\n");
				}
				break;
			case IPC_EVENT_TYPE_ERROR:
			   	{
					fprintf (stderr, "a problem happened with client %d\n", event.origin);
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

    // the application will shut down, and close the service
	TEST_IPC_Q(ipc_close_all (ctx), EXIT_FAILURE);

	// free remaining ctx
	ipc_ctx_free (ctx);
	free (ctx);

	exit(EXIT_SUCCESS);
}

/*
 * service ping-pong: send back everything sent by the ctx
 * stop the program on SIG{TERM,INT,ALRM,USR{1,2},HUP} signals
 */

int main(void)
{
	printf ("pid = %d\n", getpid ());

	ctx = malloc (sizeof (struct ipc_ctx));
	if (ctx == NULL) {
		exit (1);
	}
    memset (ctx, 0, sizeof (struct ipc_ctx));

	TEST_IPC_Q(ipc_server_init (ctx, PONGD_SERVICE_NAME), EXIT_FAILURE);

	struct ipc_connection_info * srv = &ctx->cinfos[0];
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
