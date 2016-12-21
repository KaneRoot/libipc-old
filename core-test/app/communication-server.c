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
    if (srv_init (argc, argv, env, &srv, SERVICE_NAME) < 0) {
        handle_err("main", "srv_init < 0");
        return EXIT_FAILURE;
    }

    if (srv_accept (&srv, &p) < 0) {
        handle_err("main", "srv_accept < 0");
        return EXIT_FAILURE;
    }

    if (srv_read (&p, &m) < 0) {
        handle_err("main", "srv_read < 0");
        return EXIT_FAILURE;
    }

    printf ("msg recv: %s\n", m.val);

    if (srv_write (&p, &m) < 0) {
        handle_err("main", "srv_write < 0");
        return EXIT_FAILURE;
    }
    msg_free (&m);

    // client quits
    if (srv_read (&p, &m) < 0) {
        handle_err("main", "srv_read < 0");
        return EXIT_FAILURE;
    }
    if (m.type == MSG_TYPE_DIS) {
        printf ("the client quits\n");
    }
    else {
        fprintf (stderr, "err: should have received the client dis msg\n");
    }
    msg_free (&m);

    if (srv_close_proc (&p) < 0) {
        handle_err("main", "srv_close_proc < 0");
        return EXIT_FAILURE;
    }

    if (srv_close (&srv) < 0) {
        handle_err("main", "srv_close < 0");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
