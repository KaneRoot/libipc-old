#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <stdint.h>

#include "../../core/communication.h"
#include "../../core/error.h"

#define SERVICE_NAME "pongd"

#define NUMBER_OF_MESSAGES 1000
#define MAX_MESSAGE_SIZE  IPC_MAX_MESSAGE_SIZE
#define MESSAGE "salut ça va ?"

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

	for (int i = 0 ; i < NUMBER_OF_MESSAGES ; i++) {
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

#ifdef WITH_PRINT_MESSAGES
		if (m.length > 0) {
			printf ("msg recv: %.*s\n", m.length, m.payload);
		}
#endif
		ipc_message_empty (&m);
	}

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
		printf ("usage: %s service_name [message-type [message]]\n", argv[0]);
		exit (EXIT_SUCCESS);
	}

	char service_name[100];
	memset (service_name, 0, 100);

	int current_param = 1;

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

	non_interactive (mtype, msg, service_name, env);
	free (msg);

    return EXIT_SUCCESS;
}
