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
    struct msg m;
    memset (&m, 0, sizeof (struct msg));
    struct service srv;
    memset (&srv, 0, sizeof (struct service));

    // index and version should be filled
    srv.index = 0;
    srv.version = 0;

    // init service
    if (app_connection (argc, argv, env, &srv, SERVICE_NAME, NULL, 0) < 0) {
        handle_err("main", "srv_init < 0");
        exit (EXIT_FAILURE);
    }

    printf ("msg to send: %s\n", MSG);
    msg_format_data (&m, MSG, strlen(MSG) +1);
    printf ("msg to send in the client: ");
    print_msg (&m);
    if (app_write (&srv, &m) < 0) {
        handle_err("main", "app_write < 0");
        exit (EXIT_FAILURE);
    }
    msg_free (&m);

    if (app_read (&srv, &m) < 0) {
        handle_err("main", "app_read < 0");
        exit (EXIT_FAILURE);
    }

    printf ("msg recv: %s\n", m.val);
    msg_free (&m);

    if (app_close (&srv) < 0) {
        handle_err("main", "app_close < 0");
        exit (EXIT_FAILURE);
    }
}

void interactive (int argc, char *argv[], char *env[])
{
    struct msg m;
    memset (&m, 0, sizeof (struct msg));
    struct service srv;
    memset (&srv, 0, sizeof (struct service));

    char buf[BUFSIZ];
    memset (buf, 0, BUFSIZ);
    int n;

    // index and version should be filled
    srv.index = 0;
    srv.version = 0;

    // init service
    if (app_connection (argc, argv, env, &srv, SERVICE_NAME, NULL, 0) < 0) {
        handle_err ("main", "srv_init < 0");
        exit (EXIT_FAILURE);
    }

    while (1) {
        printf ("msg to send: ");
        fflush (stdout);
        n = read (0, buf, BUFSIZ);

        if (n == 0 || strncmp (buf, "exit", 4) == 0)
            break;

        msg_format_data (&m, buf, strlen(buf) +1);
        memset (buf, 0, BUFSIZ);

        // print_msg (&m);

        if (app_write (&srv, &m) < 0) {
            handle_err("main", "app_write < 0");
            exit (EXIT_FAILURE);
        }
        msg_free (&m);

        if (app_read (&srv, &m) < 0) {
            handle_err("main", "app_read < 0");
            exit (EXIT_FAILURE);
        }

        printf ("msg recv: %s", m.val);
        msg_free (&m);
    }

    if (app_close (&srv) < 0) {
        handle_err("main", "app_close < 0");
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