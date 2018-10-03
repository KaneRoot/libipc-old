#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../core/error.h"
#include "../../core/communication.h"

#define MSG "coucou"
#define SERVICE_NAME "test"

int main (int argc, char *argv[], char *env[])
{

    struct msg m;
    memset (&m, 0, sizeof (struct msg));
    struct service srv;
    memset(&srv, 0, sizeof (struct service));

    // index and version should be filled
    srv.index = 0;
    srv.version = 0;

    struct process p;
    memset (&p, 0, sizeof (struct process));

    // init service
    if (server_init (argc, argv, env, &srv, SERVICE_NAME) < 0) {
        handle_err("main", "server_init < 0");
        return EXIT_FAILURE;
    }

    if (server_accept (&srv, &p) < 0) {
        handle_err("main", "server_accept < 0");
        return EXIT_FAILURE;
    }

    if (server_read (&p, &m) < 0) {
        handle_err("main", "server_read < 0");
        return EXIT_FAILURE;
    }

    printf ("msg recv: %s\n", m.val);

    if (server_write (&p, &m) < 0) {
        handle_err("main", "server_write < 0");
        return EXIT_FAILURE;
    }
    ipc_message_free (&m);

    // client quits
    if (server_read (&p, &m) < 0) {
        handle_err("main", "server_read < 0");
        return EXIT_FAILURE;
    }
    if (m.type == MSG_TYPE_CLOSE) {
        printf ("the client quits\n");
    }
    else {
        fprintf (stderr, "err: should have received the client dis msg\n");
    }
    ipc_message_free (&m);

    if (server_close_proc (&p) < 0) {
        handle_err("main", "server_close_proc < 0");
        return EXIT_FAILURE;
    }

    if (server_close (&srv) < 0) {
        handle_err("main", "server_close < 0");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
