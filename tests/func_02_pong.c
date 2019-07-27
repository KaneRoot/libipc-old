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

void non_interactive (char *env[])
{
	enum ipc_errors ret;
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
	SECURE_DECLARATION(struct ipc_connection_info, srv);

    // init service
	ret = ipc_connection (env, &srv, SERVICE_NAME);
    if (ret != IPC_ERROR_NONE) {
		PRINTERR(ret, "application connection");
        exit (EXIT_FAILURE);
    }

    printf ("msg to send (%ld): %.*s\n", (ssize_t) strlen(MSG) +1, (int) strlen(MSG), MSG);
    ret = ipc_message_format_data (&m, /* type */ 'a', MSG, (ssize_t) strlen(MSG) +1);
	if (ret != IPC_ERROR_NONE) {
		PRINTERR(ret, "message format data");
		exit (EXIT_FAILURE);
	}

    // printf ("msg to send in the client: ");
    // ipc_message_print (&m);
	ret = ipc_write (&srv, &m);
    if (ret != IPC_ERROR_NONE) {
		PRINTERR(ret, "application write");
        exit (EXIT_FAILURE);
    }
    ipc_message_empty (&m);

	ret = ipc_read (&srv, &m);
    if (ret != IPC_ERROR_NONE) {
		PRINTERR(ret, "application read");
        exit (EXIT_FAILURE);
    }

    printf ("msg recv: %s\n", m.payload);
    ipc_message_empty (&m);

	ret = ipc_close (&srv);
    if (ret != IPC_ERROR_NONE) {
		PRINTERR(ret, "application close");
        exit (EXIT_FAILURE);
    }
}

void interactive (char *env[])
{
	enum ipc_errors ret;
	SECURE_DECLARATION(struct ipc_connection_info, srv);

    // index and version should be filled
    srv.index = 0;
    srv.version = 0;

    // init service
	ret = ipc_connection (env, &srv, SERVICE_NAME);
    if (ret != IPC_ERROR_NONE) {
		PRINTERR(ret, "application connection");
        exit (EXIT_FAILURE);
    }

	SECURE_DECLARATION(struct ipc_event, event);
	SECURE_DECLARATION(struct ipc_connection_infos, services);

	ipc_add (&services, &srv);

    while (1) {
        printf ("msg to send: ");
        fflush (stdout);
		ret = ipc_wait_event (&services, NULL, &event);

		if (ret != IPC_ERROR_NONE) {
			PRINTERR(ret, "application peek event");
			exit (EXIT_FAILURE);
		}

		switch (event.type) {
			case IPC_EVENT_TYPE_EXTRA_SOCKET:
				{
					struct ipc_message *m = event.m;
					if ( m->length == 0 || strncmp (m->payload, "exit", 4) == 0) {

						ipc_message_empty (m);
						free (m);

						ipc_connections_free (&services);

						ret = ipc_close (&srv);
						if (ret != IPC_ERROR_NONE) {
							PRINTERR(ret, "application close");
							exit (EXIT_FAILURE);
						}

						exit (EXIT_SUCCESS);
					}

					ret = ipc_write (&srv, m);
					if (ret != IPC_ERROR_NONE) {
						PRINTERR(ret, "application write");
						exit (EXIT_FAILURE);
					}
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

    if (argc == 1)
        non_interactive (env);
    else
        interactive (env);

    return EXIT_SUCCESS;
}
