#include "../lib/pubsubd.h"
#include <stdlib.h>
#include <string.h>

#define MESSAGE "coucou"
#define CHAN    "chan1"

void
ohshit(int rvalue, const char* str) {
    fprintf (stderr, "\033[31merr: %s\033[00m\n", str);
    exit (rvalue);
}

void usage (char **argv)
{
    printf ( "usage: %s\n", argv[0]);
}

int
main(int argc, char **argv)
{
    if (argc != 1) {
        usage (argv);
        exit (1);
    }

    struct pubsub_msg msg;
    memset (&msg, 0, sizeof (struct pubsub_msg));
    msg.type = PUBSUB_TYPE_MESSAGE;
    msg.chan = malloc (strlen (CHAN) + 1);
    strncpy ((char *)msg.chan, CHAN, strlen (CHAN) + 1);
    msg.chanlen = strlen (CHAN) + 1;

    msg.data = malloc (strlen (MESSAGE) + 1);
    strncpy ((char *)msg.data, MESSAGE, strlen (CHAN) + 1);
    msg.datalen = strlen (MESSAGE) + 1;

    char *data = NULL;
    size_t len = 0;
    pubsubd_msg_serialize (&msg, &data, &len);
    pubsubd_msg_free (&msg);

    if ((int) len != write (1, data, len)) {
        ohshit (1, "unable to write the data");
    }

    return EXIT_SUCCESS;
}
