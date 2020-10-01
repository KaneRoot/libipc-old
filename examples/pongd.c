#include "../src/ipc.h"
#include <signal.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define PONGD_SERVICE_NAME "pong"
#define PONGD_VERBOSE

#define PRINTERR(ret,msg) {\
	fprintf(stderr, "error while %s: %s\n", msg, ret.error_message);\
}

/**
 * ipc_ctx:
 *   cinfos: array of ipc_connection_info
 *   pollfd: array of `pollfd` structure
 * Both arrays share the same indices.
 */

/**
 ******************************************************************************
 * Overview of the main loop:
 * 1. "ctx" pointer declaration (struct ipc_ctx).
 * 1. ipc_server_init (ctx, SERVICE_NAME)
 ******************************************************************************
 */

int cpt = 0;

int verbosity = 1;

struct ipc_ctx *ctx = NULL;

void main_loop ()
{
	int base_timer = 10000;
	int timer = base_timer;
	SECURE_DECLARATION (struct ipc_error, ret);

	SECURE_DECLARATION (struct ipc_event, event);
	event.type = IPC_EVENT_TYPE_NOT_SET;

	while (1) {
		// ipc_wait_event provides one event at a time
		// warning: event->m is free'ed if not NULL
		TEST_IPC_WAIT_EVENT_Q (ipc_wait_event (ctx, &event, &timer), EXIT_FAILURE);

		switch (event.type) {
		case IPC_EVENT_TYPE_CONNECTION:
			{
				cpt++;
				if (verbosity > 1) {
					printf ("connection: %d ctx connected, new client is %d\n", cpt, event.origin);
				}
			};
			break;
		case IPC_EVENT_TYPE_DISCONNECTION:
			{
				cpt--;
				if (verbosity > 1) {
					printf ("disconnection: %d ctx remaining\n", cpt);
				}
			};
			break;
		case IPC_EVENT_TYPE_MESSAGE:
			{
				struct ipc_message *m = event.m;
				if (verbosity > 1) {
					if (m->length > 0) {
						printf ("message received (type %d, user type %d, size %u bytes): %.*s\n",
							m->type, m->user_type, m->length, m->length, m->payload);
					} else {
						printf ("message with a 0-byte size :(\n");
					}
				}

				ret = ipc_write (ctx, m);
				if (ret.error_code != IPC_ERROR_NONE) {
					PRINTERR (ret, "server write");
				}
			};
			break;
		case IPC_EVENT_TYPE_TIMER:{
				printf ("timer\n");

				timer = base_timer;
			};
			break;
		case IPC_EVENT_TYPE_ERROR:
			{
				cpt--;
				fprintf (stderr, "a problem happened with client %d (now disconnected)", event.origin);
				fprintf (stderr, ", %d ctx remaining\n", cpt);
			};
			break;
		case IPC_EVENT_TYPE_TX:
			{
				if (verbosity > 1) {
					printf ("a message was sent\n");
				}
			}
			break;
		default:
			{
				fprintf (stderr, "there must be a problem, event not set\n");
			};
		}
	}

	// should never go there
	exit (EXIT_FAILURE);
}

void exit_program (int signal)
{
	printf ("Quitting, signal: %d\n", signal);

	// the application will shut down, and close the service
	struct ipc_error ret = ipc_close_all (ctx);
	if (ret.error_code != IPC_ERROR_NONE) {
		PRINTERR (ret, "server close");
	}

	ipc_ctx_free (ctx);
	free (ctx);

	exit (EXIT_SUCCESS);
}

/*
 * service ping-pong: send back everything sent by the ctx
 * stop the program on SIG{TERM,INT,ALRM,USR{1,2},HUP} signals
 */

int main (int argc, char *argv[])
{
	printf ("Usage: %s [verbosity]\n", argv[0]);
	if (argc > 1) {
		verbosity = atoi(argv[1]);
	}

	printf ("pid = %d\n", getpid ());

	ctx = malloc (sizeof (struct ipc_ctx));
	memset (ctx, 0, sizeof (struct ipc_ctx));

	struct ipc_error ret = ipc_server_init (ctx, PONGD_SERVICE_NAME);
	if (ret.error_code != IPC_ERROR_NONE) {
		PRINTERR (ret, "server init");
		return EXIT_FAILURE;
	}
	printf ("Listening on %s.\n", ctx->cinfos[0].spath);

	printf ("MAIN: server created\n");

	signal (SIGHUP,  exit_program);
	signal (SIGALRM, exit_program);
	signal (SIGUSR1, exit_program);
	signal (SIGUSR2, exit_program);
	signal (SIGTERM, exit_program);
	signal (SIGINT,  exit_program);

	// the service will loop until the end of time, or a signal
	main_loop ();

	// main_loop should not return
	return EXIT_FAILURE;
}
