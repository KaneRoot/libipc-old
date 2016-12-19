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

    // init service
    if (app_connection (argc, argv, env, &srv, SERVICE_NAME, NULL, 0) < 0) {
        handle_err("main", "srv_init < 0");
        return EXIT_FAILURE;
    }

    if (app_write (&srv, MSG, strlen(MSG)) < 0) {
        handle_err("main", "app_write < 0");
        return EXIT_FAILURE;
    }

    if (app_read (&srv, &buf, &msize) < 0) {
        handle_err("main", "app_read < 0");
        return EXIT_FAILURE;
    }

    printf ("msg recv: %s\n", buf);

    if (app_close (&srv) < 0) {
        handle_err("main", "app_close < 0");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
