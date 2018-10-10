#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "../../core/communication.h"
#include "../../core/error.h"

#define SERVICE_NAME "pongd"

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

	char msg_type = 0;

    while (1) {
        char * mtype_str = readline("msg type: ");
		sscanf(mtype_str, "%c", &msg_type);
		free(mtype_str);

        char * buf = readline ("msg:      ");
        if (strlen(buf) == 0 || strncmp (buf, "exit", 4) == 0)
            break;

        ipc_message_format (&m, msg_type, buf, strlen(buf));
        memset (buf, 0, BUFSIZ);

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

        printf ("msg recv: %s", m.payload);
        ipc_message_empty (&m);
    }

	if (ipc_application_close (&srv) < 0) {
        handle_err("main", "application_close < 0");
        exit (EXIT_FAILURE);
    }
}

int main (int argc, char *argv[], char *env[])
{
	argc = argc; // warnings
	argv = argv; // warnings

	char service_name[100];

	if (argc != 1) {
		ssize_t t = strlen(argv[1]) > 100 ? 100 : strlen(argv[1]);
		memcpy(service_name, argv[1], t);
	}

	interactive (service_name, env);

    return EXIT_SUCCESS;
}
