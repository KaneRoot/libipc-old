#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "msg.h"
#include "../../core/error.h"

void pubsub_msg_serialize (const struct pubsub_msg *msg, char **data, size_t *len)
{
    if (msg == NULL) {
        handle_err ("pubsub_msg_serialize", "msg == NULL");
        return;
    }

    if (data == NULL) {
        handle_err ("pubsub_msg_serialize", "data == NULL");
        return;
    }

    if (*data != NULL) {
        handle_err ("pubsub_msg_serialize", "*data != NULL");
        return;
    }

    if (len == NULL) {
        handle_err ("pubsub_msg_serialize", "len == NULL");
        return;
    }

    // buflen = pubsub msg type (1) + 2* size_t (16) + chan+data
    size_t buflen = 1 + 2 * sizeof (size_t) + msg->chanlen + msg->datalen;

    if (buflen > BUFSIZ) {
        handle_err ("pubsub_msg_serialize", "chanlen + datalen too high");
        return;
    }

    char *buf = malloc (buflen);
    memset (buf, 0, buflen);

    size_t offset = 0;

    // msg type
    buf[offset++] = msg->type;

    // chan
    memcpy (buf + offset, &msg->chanlen, sizeof (size_t));
    offset += sizeof (size_t);
    memcpy (buf + offset, msg->chan, msg->chanlen);
    offset += msg->chanlen;

    // data
    memcpy (buf + offset, &msg->datalen, sizeof (size_t));
    offset += sizeof (size_t);
    memcpy (buf + offset, msg->data, msg->datalen);
    offset += msg->datalen;

    *data = buf;
    *len = buflen;
}

void pubsub_msg_unserialize (struct pubsub_msg *msg, const char *buf, size_t mlen)
{
    if (msg == NULL) {
        handle_err ("pubsub_msg_unserialize", "msg == NULL");
        return;
    }

    pubsub_msg_free (msg);

    if (mlen > BUFSIZ) {
        handle_err ("pubsub_msg_unserialize", "mlen > BUFSIZ");
        return;
    }

    size_t offset = 0;

    // msg type
    msg->type = buf[offset++];

    // chan
    memcpy (&msg->chanlen, buf + offset, sizeof (size_t));
    if (msg->chanlen > BUFSIZ) {
        handle_err ("pubsub_msg_unserialize", "chanlen > BUFSIZ");
        return;
    }
    msg->chan = malloc (msg->chanlen);
    memset (msg->chan, 0, msg->chanlen);
    offset += sizeof (size_t);
    memcpy (msg->chan, buf + offset, msg->chanlen);
    offset += msg->chanlen;

    // data
    memcpy (&msg->datalen, buf + offset, sizeof (size_t));
    if (msg->datalen > BUFSIZ) {
        handle_err ("pubsub_msg_unserialize", "datalen > BUFSIZ");
        return;
    }
    msg->data = malloc (msg->datalen);
    memset (msg->data, 0, msg->datalen);
    offset += sizeof (size_t);
    memcpy (msg->data, buf + offset, msg->datalen);
    offset += msg->datalen;
}

void pubsub_msg_free (struct pubsub_msg *msg)
{
    if (msg == NULL) {
        handle_err ("pubsub_msg_free", "msg == NULL");
        return;
    }

    if (msg->chan) {
        free (msg->chan);
        msg->chan = NULL;
    }
    if (msg->data) {
        free (msg->data);
        msg->data = NULL;
    }
}

void pubsub_msg_print (const struct pubsub_msg *msg)
{
    if (msg == NULL) {
        handle_err ("pubsub_msg_print", "msg == NULL");
        return;
    }

    printf ("msg: type=%d chan=%s, data=%s\n"
            , msg->type, msg->chan, msg->data);
}
