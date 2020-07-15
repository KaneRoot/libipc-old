#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/ipc.h"

#define MSG "coucou"
#define SERVICE_NAME "pong"

#define PRINTERR(ret,msg) {\
	const char * err = ipc_errors_get (ret.error_code);\
	fprintf(stderr, "error while %s: %s\n", msg, err);\
}

void non_interactive ()
{
	SECURE_DECLARATION(struct ipc_message, m);
	SECURE_DECLARATION(struct ipc_ctx, ctx);

    // init service
	TEST_IPC_Q(ipc_connection (&ctx, SERVICE_NAME, NULL), EXIT_FAILURE);

	int server_fd = ctx.pollfd[0].fd;

    printf ("msg for fd %d to send (%ld): %.*s\n"
    	, server_fd
    	, (ssize_t) strlen(MSG) +1, (int) strlen(MSG), MSG);
    TEST_IPC_Q(ipc_message_format_data (&m, /* type */ 'a', MSG, (ssize_t) strlen(MSG) +1), EXIT_FAILURE);

    m.fd = server_fd;
	TEST_IPC_Q(ipc_write_fd (server_fd, &m), EXIT_FAILURE);
	TEST_IPC_Q(ipc_read (&ctx, 0 /* only one option */, &m), EXIT_FAILURE);

    printf ("msg recv: %s\n", m.payload);
    ipc_message_empty (&m);

	TEST_IPC_Q(ipc_close_all (&ctx), EXIT_FAILURE);
	ipc_ctx_free (&ctx);
}

#if 0
void interactive ()
{
	SECURE_DECLARATION(struct ipc_ctx, ctx);

    // init service
	TEST_IPC_Q(ipc_connection (&ctx, SERVICE_NAME), EXIT_FAILURE);

	int server_fd = ctx.pollfd[0].fd;

	SECURE_DECLARATION(struct ipc_event, event);

	long timer = 10;

    while (1) {
        printf ("msg to send: ");
        fflush (stdout);
		TEST_IPC_Q(ipc_wait_event (&ctx, &event, &timer), EXIT_FAILURE);

		switch (event.type) {
			case IPC_EVENT_TYPE_TIMER: {
					printf("time up!\n");

					timer = 10;
				};
				break;
			case IPC_EVENT_TYPE_EXTRA_SOCKET:
				{
					struct ipc_message *m = event.m;
					if ( m->length == 0 || strncmp (m->payload, "exit", 4) == 0) {

						ipc_message_empty (m);
						free (m);

						TEST_IPC_Q(ipc_close_all (&ctx), EXIT_FAILURE);

						ipc_ctx_free (&ctx);

						exit (EXIT_SUCCESS);
					}

					m->fd = server_fd;

					TEST_IPC_Q(ipc_write (&ctx, m), EXIT_FAILURE);
				}
				break;
			case IPC_EVENT_TYPE_MESSAGE:
				{
					struct ipc_message *m = event.m;
					printf ("msg recv: %.*s", m->length, m->payload);
				};
				break;
			case IPC_EVENT_TYPE_DISCONNECTION:
			case IPC_EVENT_TYPE_NOT_SET:
			case IPC_EVENT_TYPE_CONNECTION:
			case IPC_EVENT_TYPE_ERROR:
			default :
				fprintf (stderr, "should not happen, event type %d\n", event.type);
		}
    }
}
#endif

//int main (int argc, char *argv[])
int main (void)
{
	non_interactive ();

#if 0
    if (argc == 1)
        non_interactive ();
    else
        interactive ();
#endif

    return EXIT_SUCCESS;
}
