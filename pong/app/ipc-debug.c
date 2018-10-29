#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "../../core/communication.h"
#include "../../core/error.h"

#define SERVICE_NAME "pongd"

#define MAX_MESSAGE_SIZE  IPC_MAX_MESSAGE_SIZE
#define MESSAGE "salut ça va ?"

void interactive (char * service_name, char *env[])
{
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
    struct ipc_service srv;
    memset (&srv, 0, sizeof (struct ipc_service));

    // index and version should be filled
    srv.index = 0;
    srv.version = 0;

    // init service
    if (ipc_application_connection (env, &srv, service_name) < 0) {
        handle_err ("main", "ipc_application_connection < 0");
        exit (EXIT_FAILURE);
    }

	struct ipc_event event;
	memset (&event, 0, sizeof (struct ipc_event));

	struct ipc_services services;
	memset (&services, 0, sizeof (struct ipc_services));
	ipc_service_add (&services, &srv);
	int ret = 0;

    while (1) {
        printf ("msg to send: ");
        fflush (stdout);
		ret = ipc_application_peek_event (&services, &event);

		if (ret != 0) {
			handle_error("ipc_application_peek_event != 0");
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

					char mtype_str[5];
					memset(mtype_str, 0, 5);
					printf ("message type: ");
					fflush(stdout);
					read(0, mtype_str, 5);
					m->type = atoi(mtype_str);
					memset(mtype_str, 0, 5);

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
				{
					printf ("server disconnected: quitting...\n");

					// just remove srv from services, it's already closed
					ipc_services_free (&services);

					exit (EXIT_SUCCESS);
				};
			case IPC_EVENT_TYPE_NOT_SET:
			case IPC_EVENT_TYPE_CONNECTION:
			case IPC_EVENT_TYPE_ERROR:
			default :
				fprintf (stderr, "should not happen, event type %d\n", event.type);
		}
    }
}


void non_interactive (char msg_type, char *msg, char * service_name, char *env[])
{
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
    struct ipc_service srv;
    memset (&srv, 0, sizeof (struct ipc_service));

    // index and version should be filled
    srv.index = 0;
    srv.version = 0;

    // init service
    if (ipc_application_connection (env, &srv, service_name) < 0) {
        handle_err ("main", "ipc_application_connection < 0");
        exit (EXIT_FAILURE);
    }

	ipc_message_format (&m, msg_type, msg, strlen(msg) + 1);
	// print_msg (&m);

	if (ipc_application_write (&srv, &m) < 0) {
		handle_err("main", "application_write < 0");
		exit (EXIT_FAILURE);
	}
	ipc_message_empty (&m);

	if (ipc_application_read (&srv, &m) < 0) {
		handle_err("main", "application_read < 0");
		exit (EXIT_FAILURE);
	}

	if (m.length > 0) {
		printf ("msg recv: %.*s\n", m.length, m.payload);
	}
	ipc_message_empty (&m);

	if (ipc_application_close (&srv) < 0) {
        handle_err("main", "application_close < 0");
        exit (EXIT_FAILURE);
    }
	ipc_message_empty (&m);
}

// usage: ipc-debug [service-name]
int main (int argc, char *argv[], char *env[])
{
	if (argc == 1) {
		printf ("usage: %s [-n] [service_name [message-type [message]]]\n", argv[0]);
		exit (EXIT_SUCCESS);
	}

	char service_name[100];
	memset (service_name, 0, 100);

	int asked_non_interactive = 0;
	int current_param = 1;

	if (argc >= 2) {
		if (memcmp (argv[current_param], "-n", 2) == 0) {
			// non interactive
			asked_non_interactive = 1;
			current_param++;
			argc--;
		}
	}

	if (argc != 1) {
		ssize_t t = strlen(argv[current_param]) > 100 ? 100 : strlen(argv[current_param]);
		memcpy(service_name, argv[current_param], t);
		current_param++;
	}
	else { memcpy(service_name, SERVICE_NAME, strlen(SERVICE_NAME)); }

	char mtype = 2;
	if (argc > 2) {
		mtype = atoi(argv[current_param]);
		current_param++;
   	}

	char *msg = malloc (MAX_MESSAGE_SIZE);
	if (msg == NULL) {
		handle_err("main", "not enough memory");
		exit (EXIT_FAILURE);
	}
	memset(msg, 0, MAX_MESSAGE_SIZE);

	if (argc > 3) { memcpy(msg, argv[current_param], strlen(argv[current_param])); }
	else          { memcpy(msg, MESSAGE, strlen(MESSAGE)); }

	if (asked_non_interactive) {
		non_interactive (mtype, msg, service_name, env);
		free (msg);
	}
	else {

		free (msg);
		interactive (service_name, env);
	}

    return EXIT_SUCCESS;
}
