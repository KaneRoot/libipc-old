#include "../lib/pubsubd.h"
#include <stdlib.h>
#include <string.h>

void
ohshit(int rvalue, const char* str) {
    fprintf (stderr, "\033[31merr: %s\033[00m\n", str);
    exit (rvalue);
}

void usage (char **argv)
{
    printf ( "usage: cat msg | %s\n", argv[0]);
}

int
main(int argc, char **argv)
{

    if (argc != 1) {
        usage (argv);
        exit (1);
    }

    char data[BUFSIZ];
    memset (data, 0, BUFSIZ);
    size_t len = read (0, data, BUFSIZ);
    printf ("msg len %ld\n", len);

    struct pubsub_msg msg;
    pubsubd_msg_unserialize (&msg, data, len);
    pubsubd_msg_print (&msg);
    pubsubd_msg_free (&msg);

    return EXIT_SUCCESS;
}
