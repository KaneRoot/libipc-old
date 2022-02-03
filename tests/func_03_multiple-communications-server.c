#include "../src/ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define SERVICE_NAME "pong"
struct ipc_ctx *pctx = NULL;

void exit_program(int signal)
{
	printf("Quitting, signal: %d\n", signal);

    // the application will shut down, and close the service
	TEST_IPC_Q(ipc_close_all (pctx), EXIT_FAILURE);

	// free remaining ctx
	ipc_ctx_free (pctx);

	exit(EXIT_SUCCESS);
}

int main_loop(int argc, char * argv[])
{
	argc = argc;
	argv = argv;

	SECURE_DECLARATION (struct ipc_ctx, ctx);
	pctx = &ctx;
	int timer = 10000; // in ms

	printf ("func 03 - server init...\n");
	TEST_IPC_Q (ipc_server_init (&ctx, SERVICE_NAME), EXIT_FAILURE);
	printf ("func 03 - server init ok\n");

	SECURE_DECLARATION (struct ipc_event, event);

	printf ("func 01 - service polling...\n");
	// listen only for a single client
	while (1) {
		TEST_IPC_WAIT_EVENT_Q (ipc_wait_event (&ctx, &event, &timer), EXIT_FAILURE);

		switch (event.type) {
			case IPC_EVENT_TYPE_TIMER : {
					fprintf (stderr, "time up!\n");
					timer = 10000;
				}
				break;
			case IPC_EVENT_TYPE_CONNECTION : {
					printf ("connection establishment: %d \n", event.origin);
				}
				break;
			case IPC_EVENT_TYPE_DISCONNECTION : {
					printf ("client %d disconnecting\n", event.origin);
				};
				break;
			case IPC_EVENT_TYPE_MESSAGE : {
					struct ipc_message *m = (struct ipc_message*) event.m;
					printf ("received message (%d bytes): %.*s\n"
						, m->length
						, m->length
						, m->payload);
					ipc_write (&ctx, m);
				}
				break;
			case IPC_EVENT_TYPE_TX : {
					printf ("message sent to fd %d\n", event.origin);
				}
				break;
			case IPC_EVENT_TYPE_NOT_SET :
			case IPC_EVENT_TYPE_ERROR :
			case IPC_EVENT_TYPE_EXTRA_SOCKET :
			default :
				printf ("not ok - should not happen\n");
				exit (EXIT_FAILURE);
				break;
		}
	}

	printf ("func 03 - closing server...\n");
	TEST_IPC_Q (ipc_close_all(&ctx), EXIT_FAILURE);

	printf ("func 03 - freeing the context\n");
	ipc_ctx_free (&ctx);

	return 0;
}


int main(int argc, char * argv[])
{
	// In case we want to quit the program, do it cleanly.
	signal (SIGHUP, exit_program);
	signal (SIGALRM, exit_program);
	signal (SIGUSR1, exit_program);
	signal (SIGUSR2, exit_program);
	signal (SIGTERM, exit_program);
	signal (SIGINT, exit_program);

	main_loop (argc, argv);
    return EXIT_SUCCESS;
}
