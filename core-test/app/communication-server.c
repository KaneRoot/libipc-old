#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../core/communication.h"

#define handle_err(fun,msg)\
    fprintf (stderr, "%s: file %s line %d %s\n", fun, __FILE__, __LINE__, msg);

#define MSG "coucou"
#define SERVICE_NAME "test"

int main (int argc, char *argv[], char *env[])
{

    size_t msize = BUFSIZ;
    char *buf = NULL;

    if ( (buf = malloc (BUFSIZ)) == NULL) {
        handle_err ("main", "malloc");
        return EXIT_FAILURE;
    }
    memset (buf, 0, BUFSIZ);

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

    if (srv_read (&p, &buf, &msize) < 0) {
        handle_err("main", "srv_read < 0");
        return EXIT_FAILURE;
    }

    printf ("msg recv: %s\n", buf);

    if (srv_write (&p, buf, msize) < 0) {
        handle_err("main", "srv_write < 0");
        return EXIT_FAILURE;
    }

    if (srv_close_proc (&p) < 0) {
        handle_err("main", "srv_close_proc < 0");
        return EXIT_FAILURE;
    }

    if (srv_close (&srv) < 0) {
        handle_err("main", "srv_close < 0");
        return EXIT_FAILURE;
    }

    if (buf != NULL)
        free (buf);

    return EXIT_SUCCESS;
}
