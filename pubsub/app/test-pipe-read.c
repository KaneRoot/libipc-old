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
        if (ret <= 0) {
            fprintf (stderr, "no msg");
            if (ret == ER_FILE_OPEN) {
                fprintf (stderr, " ER_FILE_OPEN");
            }
            fprintf (stderr, "\n");
            nb++;
            continue;
        }
        if (msize > 0) {
            printf ("msg size %ld\t", msize);
            struct pubsub_msg m;
            memset (&m, 0, sizeof (struct pubsub_msg));
            pubsubd_msg_unserialize (&m, buf, msize);
            pubsubd_msg_print (&m);
            pubsubd_msg_free (&m);
        }
    }

    return EXIT_SUCCESS;
}
