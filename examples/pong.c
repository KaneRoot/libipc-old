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

void chomp (char *str, ssize_t len)
{
	if (str[len - 1] == '\n') {
		str[len - 1] = '\0';
	}
	if (str[len - 2] == '\n') {
		str[len - 2] = '\0';
	}
}

struct ipc_connection_info *srv;

void non_interactive (char *env[])
{
	SECURE_DECLARATION (struct ipc_message, m);

	// init service
	TEST_IPC_QUIT_ON_ERROR (ipc_connection (env, srv, SERVICE_NAME), EXIT_FAILURE);
	TEST_IPC_QUIT_ON_ERROR (ipc_message_format_data (&m, 42, MSG, (ssize_t) strlen (MSG) + 1), EXIT_FAILURE);

	printf ("msg to send (%ld): %.*s\n", (ssize_t) strlen (MSG) + 1, (int)strlen (MSG), MSG);
	TEST_IPC_QUIT_ON_ERROR (ipc_write (srv, &m), EXIT_FAILURE);
	ipc_message_empty (&m);
	TEST_IPC_QUIT_ON_ERROR (ipc_read (srv, &m), EXIT_FAILURE);

	printf ("msg recv (type: %u): %s\n", m.user_type, m.payload);
	ipc_message_empty (&m);

	TEST_IPC_QUIT_ON_ERROR (ipc_close (srv), EXIT_FAILURE);
}

void interactive (char *env[])
{
	// init service
	TEST_IPC_QUIT_ON_ERROR (ipc_connection (env, srv, SERVICE_NAME), EXIT_FAILURE);

	SECURE_DECLARATION (struct ipc_error, ret);
	SECURE_DECLARATION (struct ipc_event, event);
	SECURE_DECLARATION (struct ipc_connection_infos, services);

	ipc_add (&services, srv);
	ipc_add_fd (&services, 0);	// add STDIN

	ipc_connections_print (&services);

	long timer = 10;

	while (1) {
		printf ("msg to send: ");
		fflush (stdout);

		TEST_IPC_WAIT_EVENT_Q (ipc_wait_event (&services, NULL, &event, &timer), EXIT_FAILURE);
		switch (event.type) {
		case IPC_EVENT_TYPE_EXTRA_SOCKET:
			{
				// structure not read, should read the message here
				ssize_t len;
				char buf[4096];
				memset (buf, 0, 4096);

				len = read (event.origin->fd, buf, 4096);

				buf[len - 1] = '\0';
				chomp (buf, len);

#if 0
				printf ("\n");
				printf ("message to send: %.*s\n", (int)len, buf);
#endif

				// in case we want to quit the program
				if (len == 0 || strncmp (buf, "quit", 4) == 0 || strncmp (buf, "exit", 4) == 0) {

					struct ipc_error ret = ipc_close (srv);
					if (ret.error_code != IPC_ERROR_NONE) {
						fprintf (stderr, "%s", ret.error_message);
						exit (EXIT_FAILURE);
					}

					ipc_connections_free (&services);

					exit (EXIT_SUCCESS);
				}
				// send the message read on STDIN
				struct ipc_message *m = NULL;
				SECURE_BUFFER_HEAP_ALLOCATION (m, sizeof (struct ipc_message),, return; );

				struct ipc_error ret = ipc_message_format_data (m, 42, buf, len);
				if (ret.error_code != IPC_ERROR_NONE) {
					fprintf (stderr, "%s", ret.error_message);
					exit (EXIT_FAILURE);
				}
#if 0
				printf ("\n");
				printf ("right before sending a message\n");
#endif
				ret = ipc_write (srv, m);
				if (ret.error_code != IPC_ERROR_NONE) {
					fprintf (stderr, "%s", ret.error_message);
					exit (EXIT_FAILURE);
				}
#if 0
				printf ("right after sending a message\n");
#endif

				ipc_message_empty (m);
				free (m);
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

int main (int argc, char *argv[], char *env[])
{
	argc = argc;		// warnings
	argv = argv;		// warnings

	srv = malloc (sizeof (struct ipc_connection_info));
	memset (srv, 0, sizeof (struct ipc_connection_info));

	// index and version should be filled
	srv->index = 0;
	srv->version = 0;

	if (argc == 1)
		non_interactive (env);
	else
		interactive (env);

	return EXIT_SUCCESS;
}
