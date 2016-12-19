#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "../../core/usocket.h"

#define handle_err(fun,msg)\
    fprintf (stderr, "%s: file %s line %d %s\n", fun, __FILE__, __LINE__, msg);

#define UPATH "/tmp/ipc/usock-path.sock"
#define MSG "coucou"

int main (int argc, char * argv[])
{
    argc = argc;
    argv = argv;

    int fd = 0;
    size_t msize = BUFSIZ;
    char *buf = NULL;

    struct timeval t1;
    struct timeval t2;

    if ( (buf = malloc (BUFSIZ)) == NULL) {
        handle_err ("main", "malloc");
        return EXIT_FAILURE;
    }
    memset (buf, 0, BUFSIZ);

    if (usock_connect (&fd, UPATH) < 0) {
        handle_err("main", "usock_listen < 0");
        return EXIT_FAILURE;
    }

    gettimeofday (&t1, NULL);
    if (usock_send (fd, MSG, strlen(MSG)) < 0) {
        handle_err("main", "usock_send < 0");
        return EXIT_FAILURE;
    }

    if (usock_recv (fd, &buf, &msize) < 0) {
        handle_err("main", "usock_recv < 0");
        return EXIT_FAILURE;
    }
    gettimeofday (&t2, NULL);

    printf ("it took %ld µs to send then recv a message\n"
            , t2.tv_usec - t1.tv_usec);

    if (usock_close (fd) < 0) {
        handle_err("main", "usock_close < 0");
        return EXIT_FAILURE;
    }

    if (buf != NULL)
        free (buf);

    return EXIT_SUCCESS;
}
