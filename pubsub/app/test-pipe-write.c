#include "../lib/pubsubd.h"
#include <stdlib.h>

#define CHAN        "chan1"
#define MESSAGE     "coucou"

void
ohshit(int rvalue, const char* str) {
    fprintf(stderr, "%s\n", str);
    exit(rvalue);
}

void usage (char **argv) {
    fprintf (stderr, "usage: %s path times\n", argv[0]);
    fprintf (stderr, "ex: %s /tmp/pipe 5  => you will write 5 times\n", argv[0]);
    exit (1);
}

int
main(int argc, char **argv)
{

    if (argc != 3)
        usage (argv);

    char *path = argv[1];
    int nb = atoi (argv[2]);

    printf ("Writing on %s %d times.\n", path, nb);

    char *buf = NULL;
    size_t msize = 0;

    int ret = 0;

    while (nb--) {
        struct pubsub_msg msg;
        memset (&msg, 0, sizeof (struct pubsub_msg));
        msg.type = PUBSUB_TYPE_MESSAGE;
        msg.chan = malloc (strlen (CHAN) + 1);
        strncpy ((char *)msg.chan, CHAN, strlen (CHAN) + 1);
        msg.chanlen = strlen (CHAN) + 1;
        msg.data = malloc (strlen (MESSAGE) + 1);
        strncpy ((char *)msg.data, MESSAGE, strlen (CHAN) + 1);
        msg.datalen = strlen (MESSAGE) + 1;

        pubsubd_msg_serialize (&msg, &buf, &msize);
        pubsubd_msg_print (&msg);
        pubsubd_msg_free (&msg);
        ret = file_write (path, buf, msize);
        if (ret != (int) msize) {
            fprintf (stderr, "msg not written\n");
        }
    }

    return EXIT_SUCCESS;
}
