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

struct ipc_connection_info *srv;

void non_interactive (char *env[])
{
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));

	enum ipc_errors ret;

    // init service
	ret = ipc_connection (env, srv, SERVICE_NAME);
    if (ret != IPC_ERROR_NONE) {
        handle_err("main", "ipc_application_connection < 0");
		PRINTERR(ret, "application connection");
        exit (EXIT_FAILURE);
    }

    printf ("msg to send (%ld): %.*s\n", (ssize_t) strlen(MSG) +1, (int) strlen(MSG), MSG);
    ret = ipc_message_format_data (&m, 42, MSG, (ssize_t) strlen(MSG) +1);
	if (ret != IPC_ERROR_NONE) {
		PRINTERR(ret, "message format data");
		exit (EXIT_FAILURE);
	}

    // printf ("msg to send in the client: ");
    // ipc_message_print (&m);
	ret = ipc_write (srv, &m);
    if (ret != IPC_ERROR_NONE) {
        handle_err("main", "application_write");
		PRINTERR(ret, "application write");
        exit (EXIT_FAILURE);
    }
    ipc_message_empty (&m);

	ret = ipc_read (srv, &m);
    if (ret != IPC_ERROR_NONE) {
        handle_err("main", "application_read");
		PRINTERR(ret, "application read");
        exit (EXIT_FAILURE);
    }

    printf ("msg recv (type: %u): %s\n", m.user_type, m.payload);
    ipc_message_empty (&m);

	ret = ipc_close (srv);
    if (ret != IPC_ERROR_NONE) {
        handle_err("main", "application_close");
		PRINTERR(ret, "application close");
        exit (EXIT_FAILURE);
    }
}

void interactive (char *env[])
{
	enum ipc_errors ret;

    // init service
	ret = ipc_connection (env, srv, SERVICE_NAME);
    if (ret != IPC_ERROR_NONE) {
        handle_err ("main", "ipc_application_connection < 0");
		PRINTERR(ret, "application connection");
        exit (EXIT_FAILURE);
    }

	struct ipc_event event;
	memset (&event, 0, sizeof (struct ipc_event));

	struct ipc_connection_infos services;
	memset (&services, 0, sizeof (struct ipc_connection_infos));
	ipc_add (&services, srv);

	ipc_add_fd (&services, 0); // add STDIN

	ipc_connections_print(&services);

    while (1) {
        printf ("msg to send: ");
        fflush (stdout);
		ret = ipc_wait_event (&services, NULL, &event);

		if (ret != IPC_ERROR_NONE) {
			PRINTERR(ret, "wait event");
			exit (EXIT_FAILURE);
		}

		switch (event.type) {
			case IPC_EVENT_TYPE_EXTRA_SOCKET:
				{
					// structure not read, should read the message here

					ssize_t len;
					char buf[4096];
					memset(buf, 0, 4096);

					len = read (event.origin->fd, buf, 4096);

					buf[100] = '\0';

					// in case we want to quit the program
					if ( len == 0
						|| strncmp (buf, "quit", 4) == 0
						|| strncmp (buf, "exit", 4) == 0) {

						ret = ipc_close (srv);
						if (ret != IPC_ERROR_NONE) {
							handle_err("main", "ipc_close");
							PRINTERR(ret, "application close");
							exit (EXIT_FAILURE);
						}

						ipc_connections_free (&services);

						exit (EXIT_SUCCESS);
					}

					// send the message read on STDIN
					struct ipc_message *m = NULL;
					m = malloc (sizeof (struct ipc_message));
					memset (m, 0, sizeof (struct ipc_message));

					ret = ipc_write (srv, m);
					if (ret != IPC_ERROR_NONE) {
						handle_err("main", "ipc_write");
						PRINTERR(ret, "ipc_write");
						exit (EXIT_FAILURE);
					}

					ipc_message_empty (m);
					free (m);
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

int main (int argc, char *argv[], char *env[])
{
	argc = argc; // warnings
	argv = argv; // warnings

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
