#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "../src/ipc.h"
#include "../src/utils.h"

#define WEBSOCKETD_BULLSHIT

void chomp (char *str, ssize_t len) {
	if (str[len -1] == '\n') {
		str[len -1] = '\0';
	}
	if (str[len -2] == '\n') {
		str[len -2] = '\0';
	}
}

struct ipc_connection_info *srv;

void interactive (char *env[])
{
	SECURE_BUFFER_DECLARATION (char, service_name, 100);

	char *sn = getenv("PATH_TRANSLATED");

	if (sn != NULL) {
		memcpy (service_name, sn, strlen(sn));
	}
	else {
		fprintf (stderr, "cannot see PATH_TRANSLATED variable\n");
		exit (EXIT_FAILURE);
	}

    // init service
	TIPC_P_Q (ipc_connection (env, srv, service_name), "application connection", EXIT_FAILURE);

	SECURE_DECLARATION (struct ipc_event, event);
	SECURE_DECLARATION (struct ipc_connection_infos, services);

	ipc_add (&services, srv);
	ipc_add_fd (&services, 0); // add STDIN

    while (1) {
		TIPC_P_Q (ipc_wait_event (&services, NULL, &event), "wait event", EXIT_FAILURE);

		switch (event.type) {
			case IPC_EVENT_TYPE_EXTRA_SOCKET:
				{
					// structure not read, should read the message here
					SECURE_BUFFER_DECLARATION (char, buf, 4096);
					ssize_t len;

					len = read (event.origin->fd, buf, 4096);

					// in case we want to quit the program
					if ( len == 0
						|| strncmp (buf, "quit", 4) == 0
						|| strncmp (buf, "exit", 4) == 0) {

						TIPC_P_Q (ipc_close (srv), "application close", EXIT_FAILURE);

						ipc_connections_free (&services);

						exit (EXIT_SUCCESS);
					}

					// send the message read on STDIN
					ssize_t len_sent = write (srv->fd, buf, len);
					if (len_sent != len) {
						fprintf (stderr, "cannot send the message %lu-byte message, sent %lu bytes"
								, len, len_sent);
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
			ERROR_CASE (IPC_EVENT_TYPE_DISCONNECTION, "main loop", "disconnection: should not happen");
			ERROR_CASE (IPC_EVENT_TYPE_NOT_SET      , "main loop", "not set: should not happen");
			ERROR_CASE (IPC_EVENT_TYPE_CONNECTION   , "main loop", "connection: should not happen");
			// ERROR_CASE (IPC_EVENT_TYPE_ERROR        , "main loop", "error");
			default :
				fprintf (stderr, "event type error: should not happen, event type %d\n", event.type);
		}
    }
}

int main (int argc, char *argv[], char *env[])
{
	argc = argc; // warnings
	argv = argv; // warnings

	srv = malloc (sizeof (struct ipc_connection_info));
	memset (srv, 0, sizeof (struct ipc_connection_info));

	// index and version should be filled
	srv->index = 0;
	srv->version = 0;

	interactive (env);

	return EXIT_SUCCESS;
}
