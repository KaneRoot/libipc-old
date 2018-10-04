#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../core/msg.h"
#include "../../core/error.h"
#include "../../core/communication.h"

#define MSG "coucou"
#define SERVICE_NAME "pongd"

void non_interactive (int argc, char *argv[], char *env[])
{
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
    struct ipc_service srv;
    memset (&srv, 0, sizeof (struct ipc_service));

    // index and version should be filled
    srv.index = 0;
    srv.version = 0;

    // init service
    if (ipc_application_connection (argc, argv, env, &srv, SERVICE_NAME, NULL, 0) < 0) {
        handle_err("main", "server_init < 0");
        exit (EXIT_FAILURE);
    }

    printf ("msg to send: %s\n", MSG);
    ipc_message_format_data (&m, MSG, strlen(MSG) +1);
    printf ("msg to send in the client: ");
    ipc_message_print (&m);
    if (ipc_application_write (&srv, &m) < 0) {
        handle_err("main", "application_write < 0");
        exit (EXIT_FAILURE);
    }
    ipc_message_free (&m);

    if (ipc_application_read (&srv, &m) < 0) {
        handle_err("main", "application_read < 0");
        exit (EXIT_FAILURE);
    }

    printf ("msg recv: %s\n", m.payload);
    ipc_message_free (&m);

    if (ipc_application_close (&srv) < 0) {
        handle_err("main", "application_close < 0");
        exit (EXIT_FAILURE);
    }
}

void interactive (int argc, char *argv[], char *env[])
{
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
    struct ipc_service srv;
    memset (&srv, 0, sizeof (struct ipc_service));

    char buf[BUFSIZ];
    memset (buf, 0, BUFSIZ);
    int n;

	int ask_server_to_quit = 0;

    // index and version should be filled
    srv.index = 0;
    srv.version = 0;

    // init service
    if (ipc_application_connection (argc, argv, env, &srv, SERVICE_NAME, NULL, 0) < 0) {
        handle_err ("main", "server_init < 0");
        exit (EXIT_FAILURE);
    }

    while (1) {
        printf ("msg to send: ");
        fflush (stdout);
        n = read (0, buf, BUFSIZ);

        if (n == 0 || strncmp (buf, "exit", 4) == 0)
            break;

		if (strncmp(buf, "close server", 12) == 0) {
			ask_server_to_quit = 1;
			break;
		}

        ipc_message_format_data (&m, buf, strlen(buf) +1);
        memset (buf, 0, BUFSIZ);

        // print_msg (&m);

        if (ipc_application_write (&srv, &m) < 0) {
            handle_err("main", "application_write < 0");
            exit (EXIT_FAILURE);
        }
        ipc_message_free (&m);

        if (ipc_application_read (&srv, &m) < 0) {
            handle_err("main", "application_read < 0");
            exit (EXIT_FAILURE);
        }

        printf ("msg recv: %s", m.payload);
        ipc_message_free (&m);
    }

	if (ask_server_to_quit) {
        ipc_message_format_server_close (&m);

        if (ipc_application_write (&srv, &m) < 0) {
            handle_err("main", "application_write < 0");
            exit (EXIT_FAILURE);
        }
        ipc_message_free (&m);
	} else if (ipc_application_close (&srv) < 0) {
        handle_err("main", "application_close < 0");
        exit (EXIT_FAILURE);
    }
}

int main (int argc, char *argv[], char *env[])
{
    if (argc == 1)
        non_interactive (argc, argv, env);
    else
        interactive (argc, argv, env);

    return EXIT_SUCCESS;
}
