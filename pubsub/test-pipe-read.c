#include "../lib/pubsubd.h"
#include <stdlib.h>

#define TEST_NAME "test-chan-lists"

void
ohshit(int rvalue, const char* str) {
    fprintf(stderr, "%s\n", str);
    exit(rvalue);
}

void usage (char **argv) {
    fprintf (stderr, "usage: %s path times\n", argv[0]);
    fprintf (stderr, "ex: %s /tmp/pipe 5  => you will read 5 times\n", argv[0]);
    exit (1);
}

int
main(int argc, char **argv)
{

    if (argc != 3)
        usage (argv);

    char *path = argv[1];
    int nb = atoi (argv[2]);

    printf ("Listening on %s %d times.\n", path, nb);

    char *buf = NULL;
    size_t msize = 0;

    int ret = 0;

    while (nb--) {
        ret = file_read (path, &buf, &msize);
        if (ret == 0) {
            printf ("no msg\n");
            nb++;
            continue;
        }
        struct pubsub_msg m;
        pubsubd_msg_unserialize (&m, buf, msize);
        pubsubd_msg_print (&m);
        sleep (1);
    }

    return EXIT_SUCCESS;
}
