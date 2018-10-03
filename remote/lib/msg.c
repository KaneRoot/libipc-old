#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "msg.h"
#include "../../core/error.h"

void remote_message_serialize (const struct remoted_msg *msg, char **data, size_t *len)
{
    if (msg == NULL) {
        handle_err ("remote remote_message_serialize", "msg == NULL");
        return;
    }

    if (data == NULL) {
        handle_err ("remote remote_message_serialize", "data == NULL");
        return;
    }

    if (*data != NULL) {
        handle_err ("remote remote_message_serialize", "*data != NULL");
        return;
    }

    if (len == NULL) {
        handle_err ("remote remote_message_serialize", "len == NULL");
        return;
    }

    // buflen = remote msg type (1) + size_t (16) + data
    size_t buflen = 1 + sizeof (size_t) + msg->datalen;

    if (buflen > BUFSIZ) {
        handle_err ("remote remote_message_serialize", "datalen too high");
        return;
    }

    char *buf = malloc (buflen);
    memset (buf, 0, buflen);

    size_t offset = 0;

    // msg type
    buf[offset++] = msg->type;

    // data
    memcpy (buf + offset, &msg->datalen, sizeof (size_t));
    offset += sizeof (size_t);
    memcpy (buf + offset, msg->data, msg->datalen);
    offset += msg->datalen;

    *data = buf;
    *len = buflen;
}

void remote_message_unserialize (struct remoted_msg *msg, const char *buf, size_t mlen)
{
    if (msg == NULL) {
        handle_err ("remote remote_message_unserialize", "msg == NULL");
        return;
    }

    remote_message_free (msg);

    if (mlen > BUFSIZ) {
        handle_err ("remote remote_message_unserialize", "mlen > BUFSIZ");
        return;
    }

    size_t offset = 0;

    // msg type
    msg->type = buf[offset++];

    // data
    memcpy (&msg->datalen, buf + offset, sizeof (size_t));
    if (msg->datalen > BUFSIZ) {
        handle_err ("remote remote_message_unserialize", "datalen > BUFSIZ");
        return;
    }
    msg->data = malloc (msg->datalen);
    memset (msg->data, 0, msg->datalen);
    offset += sizeof (size_t);
    memcpy (msg->data, buf + offset, msg->datalen);
    offset += msg->datalen;
}

void remote_message_free (struct remoted_msg *msg)
{
    if (msg == NULL) {
        handle_err ("remote remote_message_free", "msg == NULL");
        return;
    }

    if (msg->data) {
        free (msg->data);
        msg->data = NULL;
    }
}

void remote_message_print (const struct remoted_msg *msg)
{
    if (msg == NULL) {
        handle_err ("remote remote_message_print", "msg == NULL");
        return;
    }

    printf ("msg: type=%d data=%s\n", msg->type, msg->data);
}
