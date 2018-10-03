#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/msg.h"
#include "../../core/error.h"
#include "../../core/utils.h"

#define CHAN    "chan1"
#define DATA    "hello chan1"

int main(int argc, char * argv[])
{
    argc = argc;
    argv = argv;

    struct pubsub_msg msg;
    memset (&msg, 0, sizeof (struct pubsub_msg));

    msg.type = 8;

    msg.chanlen = strlen (CHAN) + 1;
    msg.chan = malloc (msg.chanlen);
    memset (msg.chan, 0, msg.chanlen);
    memcpy (msg.chan, CHAN, msg.chanlen);

    msg.datalen = strlen (DATA) + 1;
    msg.data = malloc (msg.datalen);
    memset (msg.data, 0, msg.datalen);
    memcpy (msg.data, DATA, msg.datalen);

    printf ("msg 1: ");
    pubsub_message_print (&msg);

    char *buffer = NULL;
    size_t len = 0;

    pubsub_message_serialize (&msg, &buffer, &len);
    mprint_hexa ("buffer msg 1", (unsigned char *) buffer, len);

    struct pubsub_msg msg2;
    memset (&msg2, 0, sizeof (struct pubsub_msg));

    pubsub_message_unserialize (&msg2, buffer, len);

    printf ("msg 2: ");
    pubsub_message_print (&msg2);

    pubsub_message_free (&msg);
    pubsub_message_free (&msg2);

    if (buffer != NULL)
        free (buffer);

    return EXIT_SUCCESS;
}
