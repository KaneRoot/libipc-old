#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "../../core/communication.h"
#include "../../core/error.h"

#define SERVICE_NAME "pongd"

#define MAX_MESSAGE_SIZE  20000
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

	char msg_type = 0;

    while (1) {
#if 0 // version with readline, slow, valgrind-incompatible
		
        char * mtype_str = readline("msg type: ");
        if (strlen(mtype_str) == 0 || strncmp (mtype_str, "exit", 4) == 0) {
			free (mtype_str);
            break;
		}
		msg_type = atoi(mtype_str);
		free(mtype_str);

        char * buf = readline ("msg:      ");
        if (strlen(buf) == 0 || strncmp (buf, "exit", 4) == 0) {
			free (buf);
            break;
		}

        ipc_message_format (&m, msg_type, buf, strlen(buf));
		free (buf);

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

        printf ("\nmsg recv: %s", m.payload);
        ipc_message_empty (&m);
#else

		char mtype_str[50];
		memset(mtype_str, 0, 50);
		printf ("message type: ");
		fflush(stdout);
		read(0, mtype_str, 50);
		msg_type = atoi(mtype_str);
		memset(mtype_str, 0, 50);

		char mpayload[MAX_MESSAGE_SIZE];
		memset(mpayload, 0, MAX_MESSAGE_SIZE);
		printf ("message payload: ");
		fflush(stdout);
		read(0, mpayload, MAX_MESSAGE_SIZE);
        if (strlen(mpayload) == 0 || strncmp (mpayload, "exit", 4) == 0) {
            break;
		}

        ipc_message_format (&m, msg_type, mpayload, strlen(mpayload));
        // print_msg (&m);

        if (ipc_application_write (&srv, &m) < 0) {
            handle_err("main", "application_write < 0");
            exit (EXIT_FAILURE);
        }
        ipc_message_empty (&m);

		if (msg_type == 0) { // message type 0 => close the server, no read to do
			break;
		}

        if (ipc_application_read (&srv, &m) < 0) {
            handle_err("main", "application_read < 0");
            exit (EXIT_FAILURE);
        }

		if (m.length > 0) {
			printf ("msg recv: %.*s", m.length, m.payload);
		}
        ipc_message_empty (&m);
#endif
    }

	if (ipc_application_close (&srv) < 0) {
        handle_err("main", "application_close < 0");
        exit (EXIT_FAILURE);
    }
	ipc_message_empty (&m);
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

int main (int argc, char *argv[], char *env[])
{
	char service_name[100];
	memset (service_name, 0, 100);

	if (argc != 1) {
		ssize_t t = strlen(argv[1]) > 100 ? 100 : strlen(argv[1]);
		memcpy(service_name, argv[1], t);
	}
	else { memcpy(service_name, SERVICE_NAME, strlen(SERVICE_NAME)); }

	char mtype = 2;
	if (argc > 2) { mtype = atoi(argv[2]); }

	char msg[IPC_MAX_MESSAGE_SIZE];
	memset(msg, 0, IPC_MAX_MESSAGE_SIZE);

	if (argc > 3) { memcpy(msg, argv[3], strlen(argv[3])); }
	else          { memcpy(msg, MESSAGE, strlen(MESSAGE)); }

	non_interactive (mtype, msg, service_name, env);

    return EXIT_SUCCESS;
}
