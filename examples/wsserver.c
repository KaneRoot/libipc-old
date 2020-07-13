#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "../src/ipc.h"
#include "../src/utils.h"

#define WEBSOCKETD_BULLSHIT

void chomp (char *str, ssize_t len)
{
	if (str[len - 1] == '\n') {
		str[len - 1] = '\0';
	}
	if (str[len - 2] == '\n') {
		str[len - 2] = '\0';
	}
}

struct ipc_ctx *ctx;

void interactive ()
{
	int timer = 10000; // 10 seconds

	SECURE_DECLARATION (struct ipc_event, event);
	SECURE_BUFFER_DECLARATION (char, service_name, 100);

	char *sn = getenv ("PATH_TRANSLATED");

	if (sn != NULL) {
		memcpy (service_name, sn, strlen (sn));
	} else {
		fprintf (stderr, "cannot see PATH_TRANSLATED variable\n");
		exit (EXIT_FAILURE);
	}

	// init service
	TEST_IPC_Q (ipc_connection (ctx, service_name, NULL), EXIT_FAILURE);

	ipc_add_fd (ctx, 0); // add STDIN

	while (1) {
		TEST_IPC_WAIT_EVENT_Q (ipc_wait_event (ctx, &event, &timer), EXIT_FAILURE);

		switch (event.type) {
		case IPC_EVENT_TYPE_TIMER:{
				fprintf (stderr, "time up!\n");
				timer = 10000;
			};
			break;
		case IPC_EVENT_TYPE_EXTRA_SOCKET:
			{
				// structure not read, should read the message here
				SECURE_BUFFER_DECLARATION (char, buf, 4096);
				ssize_t len;

				len = read (event.origin, buf, 4096);

				// in case we want to quit the program
				if (len == 0 || strncmp (buf, "quit", 4) == 0 || strncmp (buf, "exit", 4) == 0) {

					TEST_IPC_Q (ipc_close_all (ctx), EXIT_FAILURE);

					ipc_ctx_free (ctx);

					exit (EXIT_SUCCESS);
				}
				// send the message read on STDIN
				ssize_t len_sent = write (ctx->pollfd[0].fd, buf, len);
				if (len_sent != len) {
					fprintf (stderr, "cannot send the message %lu-byte message, sent %lu bytes",
						 len, len_sent);
					exit (EXIT_FAILURE);
				}
			}
			break;
		case IPC_EVENT_TYPE_MESSAGE:
			{
				struct ipc_message *m = event.m;
				SECURE_BUFFER_DECLARATION (char, buf, 4096);
				uint32_t size;
				size = ipc_message_raw_serialize (buf, m->type, m->user_type, m->payload, m->length);

				write (1, buf, size);
#ifdef WEBSOCKETD_BULLSHIT
				printf ("\n");
#endif
				fflush (stdout);
			};
			break;
		case IPC_EVENT_TYPE_TX: {
				printf ("Message sent\n");
			}
			break;
		ERROR_CASE (IPC_EVENT_TYPE_DISCONNECTION, "main loop", "disconnection: should not happen");
		ERROR_CASE (IPC_EVENT_TYPE_NOT_SET      , "main loop", "not set: should not happen");
		ERROR_CASE (IPC_EVENT_TYPE_CONNECTION   , "main loop", "connection: should not happen");
		// ERROR_CASE (IPC_EVENT_TYPE_ERROR        , "main loop", "error");
		default:
			fprintf (stderr, "event type error: should not happen, event type %d\n", event.type);
		}
	}
}

int main (void)
{
	ctx = malloc (sizeof (struct ipc_ctx));
	memset (ctx, 0, sizeof (struct ipc_ctx));

	interactive ();

	return EXIT_SUCCESS;
}
