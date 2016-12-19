#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../core/usocket.h"

#define handle_err(fun,msg)\
    fprintf (stderr, "%s: file %s line %d %s\n", fun, __FILE__, __LINE__, msg);

#define UPATH "/tmp/ipc/usock-path.sock"

int main (int argc, char * argv[])
{

    argc = argc;
    argv = argv;

    int fd = 0;
    int pfd = 0;
    size_t msize = BUFSIZ;
    char *buf = NULL;

    if ( (buf = malloc (BUFSIZ)) == NULL) {
        handle_err ("main", "malloc");
        return EXIT_FAILURE;
    }
    memset (buf, 0, BUFSIZ);

    // socket + bind + listen
    if (usock_init (&fd, UPATH) < 0) {
        handle_err("main", "usock_init < 0");
        return EXIT_FAILURE;
    }

    if (usock_accept (fd, &pfd) < 0) {
        handle_err("main", "usock_accept < 0");
        return EXIT_FAILURE;
    }

    printf ("new connection\n");

    if (usock_recv (pfd, &buf, &msize) < 0) {
        handle_err("main", "usock_recv < 0");
        return EXIT_FAILURE;
    }

    printf ("msg recv: %s\n", buf);

    if (usock_send (pfd, buf, msize) < 0) {
        handle_err("main", "usock_send < 0");
        return EXIT_FAILURE;
    }

    if (usock_close (fd) < 0) {
        handle_err("main", "usock_close fd < 0");
        return EXIT_FAILURE;
    }

    if (usock_close (pfd) < 0) {
        handle_err("main", "usock_close pfd < 0");
        return EXIT_FAILURE;
    }

    if (usock_remove (UPATH) < 0) {
        handle_err("main", "usock_remove < 0");
        return EXIT_FAILURE;
    }

    if (buf != NULL)
        free (buf);

    return EXIT_SUCCESS;
}
