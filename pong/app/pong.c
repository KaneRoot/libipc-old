#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../core/ipc.h"

#define MSG "coucou"
#define SERVICE_NAME "pongd"

void non_interactive (char *env[])
{
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
    struct ipc_service srv;
    memset (&srv, 0, sizeof (struct ipc_service));

    // index and version should be filled
    srv.index = 0;
    srv.version = 0;

    // init service
    if (ipc_application_connection (env, &srv, SERVICE_NAME) < 0) {
        handle_err("main", "ipc_application_connection < 0");
        exit (EXIT_FAILURE);
    }

    printf ("msg to send: %.*s\n", (int) strlen(MSG), MSG);
    ipc_message_format_data (&m, MSG, strlen(MSG) +1);
    // printf ("msg to send in the client: ");
    // ipc_message_print (&m);
    if (ipc_application_write (&srv, &m) < 0) {
        handle_err("main", "application_write < 0");
        exit (EXIT_FAILURE);
    }
    ipc_message_empty (&m);

    if (ipc_application_read (&srv, &m) < 0) {
        handle_err("main", "application_read < 0");
        exit (EXIT_FAILURE);
    }

    printf ("msg recv: %s\n", m.payload);
    ipc_message_empty (&m);

    if (ipc_application_close (&srv) < 0) {
        handle_err("main", "application_close < 0");
        exit (EXIT_FAILURE);
    }
}

void interactive (char *env[])
{
	int ret = 0;
    struct ipc_service srv;
    memset (&srv, 0, sizeof (struct ipc_service));

    // index and version should be filled
    srv.index = 0;
    srv.version = 0;

    // init service
    if (ipc_application_connection (env, &srv, SERVICE_NAME) < 0) {
        handle_err ("main", "ipc_application_connection < 0");
        exit (EXIT_FAILURE);
    }

	struct ipc_event event;
	memset (&event, 0, sizeof (struct ipc_event));

	struct ipc_services services;
	memset (&services, 0, sizeof (struct ipc_services));
	ipc_service_add (&services, &srv);

    while (1) {
        printf ("msg to send: ");
        fflush (stdout);
		ret = ipc_application_loop_interactive (&services, &event);

		if (ret != 0) {
			handle_error("ipc_application_loop != 0");
			exit (EXIT_FAILURE);
		}

		switch (event.type) {
			case IPC_EVENT_TYPE_STDIN:
				{
					struct ipc_message *m = event.m;
					if ( m->length == 0 || strncmp (m->payload, "exit", 4) == 0) {

						ipc_message_empty (m);
						free (m);

						ipc_services_free (&services);

						if (ipc_application_close (&srv) < 0) {
							handle_err("main", "application_close < 0");
							exit (EXIT_FAILURE);
						}

						exit (EXIT_SUCCESS);
					}

					if (ipc_application_write (&srv, m) < 0) {
						handle_err("main", "application_write < 0");
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
