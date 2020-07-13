#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/ipc.h"

#define MSG "coucou"
#define SERVICE_NAME "pong"

#define PRINTERR(ret,msg) {\
	const char * err = ipc_errors_get (ret);\
	fprintf(stderr, "error while %s: %s\n", msg, err);\
}

void chomp (char *str, ssize_t len)
{
	if (str[len - 1] == '\n') {
		str[len - 1] = '\0';
	}
	if (str[len - 2] == '\n') {
		str[len - 2] = '\0';
	}
}

struct ipc_ctx *ctx = NULL;

void non_interactive ()
{
	SECURE_DECLARATION (struct ipc_message, m);

	// init service
	TEST_IPC_Q (ipc_connection (ctx, SERVICE_NAME), EXIT_FAILURE);
	TEST_IPC_Q (ipc_message_format_data (&m, 42, MSG, (ssize_t) strlen (MSG) + 1), EXIT_FAILURE);

	printf ("msg to send (%ld): %.*s\n", (ssize_t) strlen (MSG) + 1, (int)strlen (MSG), MSG);
	// ipc_write fd: write a message without fd availability check.
	TEST_IPC_Q (ipc_write_fd (ctx->pollfd[0].fd, &m), EXIT_FAILURE);
	ipc_message_empty (&m);
	TEST_IPC_Q (ipc_read (ctx, 0 /* only valid index */, &m), EXIT_FAILURE);

	printf ("msg recv (type: %u): %s\n", m.user_type, m.payload);
	ipc_message_empty (&m);

	TEST_IPC_Q (ipc_close_all (ctx), EXIT_FAILURE);
}

void interactive ()
{
	// init service
	TEST_IPC_Q (ipc_connection (ctx, SERVICE_NAME), EXIT_FAILURE);

	SECURE_DECLARATION (struct ipc_event, event);

	ipc_add_fd (ctx, 0);	// add STDIN

	ipc_ctx_print (ctx);

	int timer = 10000;

	while (1) {
		printf ("msg to send: ");
		fflush (stdout);

		TEST_IPC_WAIT_EVENT_Q (ipc_wait_event (ctx, &event, &timer), EXIT_FAILURE);

		switch (event.type) {
		case IPC_EVENT_TYPE_TIMER:{
				printf ("time up!\n");
				timer = 10000;
			};
			break;
		case IPC_EVENT_TYPE_EXTRA_SOCKET:
			{
				// structure not read, should read the message here
				ssize_t len;
				char buf[4096];
				memset (buf, 0, 4096);

				len = read (event.origin, buf, 4096);

				buf[len - 1] = '\0';
				chomp (buf, len);

#if 0
				printf ("\n");
				printf ("message to send: %.*s\n", (int)len, buf);
#endif

				// in case we want to quit the program
				if (len == 0 || strncmp (buf, "quit", 4) == 0 || strncmp (buf, "exit", 4) == 0) {

					TEST_IPC_Q (ipc_close_all (ctx), EXIT_FAILURE);

					ipc_ctx_free (ctx);

					exit (EXIT_SUCCESS);
				}
				// send the message read on STDIN
				struct ipc_message *m = NULL;
				SECURE_BUFFER_HEAP_ALLOCATION (m, sizeof (struct ipc_message),,);

				for (size_t i = 0; i < 5; i++) {
					memset (buf, 0, 4096);
					snprintf (buf, 4096, "%lu", i);
					len = strlen (buf);
					printf ("message %lu, buffer %.*s\n", i, (int)len, buf);
					TEST_IPC_Q (ipc_message_format_data (m, 42, buf, len), EXIT_FAILURE);
					m->fd = ctx->pollfd[0].fd;

					printf ("message from structure: %.*s\n", m->length, m->payload);

#if 0
					printf ("\n");
					printf ("right before sending a message\n");
#endif
					TEST_IPC_Q (ipc_write (ctx, m), EXIT_FAILURE);
#if 0
					printf ("right after sending a message\n");
#endif

					ipc_message_empty (m);
					// sleep (1);
				}
				free (m);
			}
			break;
		case IPC_EVENT_TYPE_TX:
			{
				printf ("a message was sent\n");
			}
			break;
		case IPC_EVENT_TYPE_MESSAGE:
			{
				struct ipc_message *m = event.m;
				printf ("\rmsg recv: %.*s\n", m->length, m->payload);
			};
			break;
		case IPC_EVENT_TYPE_DISCONNECTION:
		case IPC_EVENT_TYPE_NOT_SET:
		case IPC_EVENT_TYPE_CONNECTION:
		case IPC_EVENT_TYPE_ERROR:
		default:
			fprintf (stderr, "should not happen, event type %d\n", event.type);
		}
	}
}

int main (int argc, char **argv)
{
	// Compilers fuckery.
	argv = argv;

	ctx = malloc (sizeof (struct ipc_ctx));
	memset (ctx, 0, sizeof (struct ipc_ctx));

	if (argc == 1)
		non_interactive ();
	else
		interactive ();

	return EXIT_SUCCESS;
}
