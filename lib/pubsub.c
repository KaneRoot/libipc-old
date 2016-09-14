#include "pubsub.h"
#include <stdlib.h>

#include <string.h> // strndup

// MESSAGE, TODO CBOR

void pubsubd_msg_serialize (const struct pubsub_msg *msg, char **data, size_t *len)
{
    if (msg == NULL || data == NULL || len == NULL) {
        fprintf (stderr, "pubsubd_msg_send: msg or data or len == NULL");
        return;
    }

    // msg: "type(1) chanlen(8) chan datalen(8) data
    if (msg->type == PUBSUB_TYPE_DISCONNECT) {
        *len = 1;
        if (*data != NULL) {
            free (*data);
            *data = NULL;
        }
        *data = malloc(*len);
        memset (*data, 0, *len);
        data[0][0] = msg->type;
        return;
    }
    else {
        // type + size chan + chan + size data + data
        *len = 1 + 2 * sizeof(size_t) + msg->chanlen + msg->datalen;
    }

    if (*data != NULL) {
        free (*data);
        *data = NULL;
    }
    *data = malloc(*len);
    memset (*data, 0, *len);

    size_t i = 0;

    data[0][i] = msg->type;                              i++;
    memcpy (&data[0][i], &msg->chanlen, sizeof(size_t)); i += sizeof(size_t);
    memcpy (&data[0][i], msg->chan, msg->chanlen);       i += msg->chanlen;
    memcpy (&data[0][i], &msg->datalen, sizeof(size_t)); i += sizeof(size_t);
    memcpy (&data[0][i], msg->data, msg->datalen);       i += msg->datalen;
}

void pubsubd_msg_unserialize (struct pubsub_msg *msg, const char *data, size_t len)
{
    if (msg == NULL) {
        fprintf (stderr
                , "\033[31merr: pubsubd_msg_unserialize, msg NULL\033[00m\n");
        return;
    }
    
    if (data == NULL) {
        fprintf (stderr
                , "\033[31merr: pubsubd_msg_unserialize, data NULL\033[00m\n");
        return;
    }

    if (len > BUFSIZ) {
        fprintf (stderr
                , "\033[31merr: pubsubd_msg_unserialize, len %ld\033[00m\n"
                , len);
        return;
    }

    size_t i = 0;
    msg->type = data[i];                                i++;

    if (msg->type == PUBSUB_TYPE_DISCONNECT) {
        msg->chanlen = 0;
        msg->chan = NULL;
        msg->datalen = 0;
        msg->data = NULL;
        return ;
    }

    memcpy (&msg->chanlen, data + i, sizeof(size_t));   i += sizeof(size_t);
    if (msg->chanlen > BUFSIZ) {
        fprintf (stderr, "\033[31merr : msg->chanlen > BUFSIZ\033[00m\n");
        return;
    }
    msg->chan = malloc (msg->chanlen +1);
    memset (msg->chan, 0, msg->chanlen +1);
    memcpy (msg->chan, data + i, msg->chanlen);         i += msg->chanlen;

    memcpy (&msg->datalen, data + i, sizeof(size_t));   i += sizeof(size_t);
    if (msg->datalen > BUFSIZ) {
        fprintf (stderr, "\033[31merr : msg->datalen > BUFSIZ\033[00m\n");
        return;
    }
    msg->data = malloc (msg->datalen +1);
    memset (msg->data, 0, msg->datalen +1);
    memcpy (msg->data, data + i, msg->datalen);         i += msg->datalen;
}

void pubsubd_msg_free (struct pubsub_msg *msg)
{
    if (msg == NULL) {
        fprintf (stderr, "\033[31merr: pubsubd_msg_free, msg NULL\033[00m\n");
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

void pubsubd_msg_print (const struct pubsub_msg *msg)
{
    printf ("msg: type=%d chan=%s, data=%s\n"
            , msg->type, msg->chan, msg->data);
}

#define PUBSUB_SUBSCRIBER_ACTION_STR_PUB    "pub"
#define PUBSUB_SUBSCRIBER_ACTION_STR_SUB    "sub"
#define PUBSUB_SUBSCRIBER_ACTION_STR_BOTH   "both"
#define PUBSUB_SUBSCRIBER_ACTION_STR_QUIT   "quit"

// enum app_list_elm_action {PUBSUB_QUIT = 1, PUBSUB_PUB, PUBSUB_SUB, PUBSUB_BOTH};

char * pubsub_action_to_str (enum app_list_elm_action action)
{
    switch (action) {
        case PUBSUB_PUB : return strdup (PUBSUB_SUBSCRIBER_ACTION_STR_PUB);
        case PUBSUB_SUB : return strdup (PUBSUB_SUBSCRIBER_ACTION_STR_SUB);
        case PUBSUB_BOTH : return strdup (PUBSUB_SUBSCRIBER_ACTION_STR_BOTH);
        case PUBSUB_QUIT : return strdup (PUBSUB_SUBSCRIBER_ACTION_STR_QUIT);
    }

    return NULL;
}

void pubsub_connection (struct service *srv, struct process *p, enum app_list_elm_action action, const char *channame)
{
    char * straction = NULL;
    straction = pubsub_action_to_str (action);

    char line[BUFSIZ];
    memset (line, 0, BUFSIZ);

    // line fmt : pid index version action chan
    // "quit" action is also possible (see pubsubd_quit)
    snprintf (line, BUFSIZ, "%d %d %d %s %s\n"
            , p->pid, p->index, p->version
            , straction
            , channame);
    line[BUFSIZ -1] = '\0'; // to be sure

    // send the connection line in the $TMP/<service> pipe
    app_srv_connection (srv, line, strlen (line));

    if (straction != NULL)
        free (straction);
}

void pubsub_disconnect (struct process *p)
{
    struct pubsub_msg m;
    memset (&m, 0, sizeof (struct pubsub_msg));
    m.type = PUBSUB_TYPE_DISCONNECT;

    char *buf = NULL;
    size_t msize = 0;
    pubsubd_msg_serialize (&m, &buf, &msize);

    int ret = app_write (p, buf, msize);
    if (ret != (int) msize) {
        fprintf (stderr, "err: can't disconnect\n");
    }

    pubsubd_msg_free (&m);
    if (buf != NULL) {
        free (buf);
    }
}

// tell the service to stop
void pubsubd_quit (struct service *srv)
{
    // line fmt : 0 0 0 quit
    char line[BUFSIZ];
    snprintf (line, BUFSIZ, "0 0 0 quit\n");
    app_srv_connection (srv, line, strlen (line));
}

void pubsub_msg_send (struct process *p, const struct pubsub_msg * m)
{
    char *buf = NULL;
    size_t msize = 0;
    pubsubd_msg_serialize (m, &buf, &msize);

    app_write (p, buf, msize);

    if (buf != NULL) {
        free (buf);
    }
}

void pubsub_msg_recv (struct process *p, struct pubsub_msg *m)
{
    // read the message from the process
    size_t mlen = 0;
    char *buf = NULL;
    while (buf == NULL || mlen == 0) {
        app_read (p, &buf, &mlen);
    }

    pubsubd_msg_unserialize (m, buf, mlen);

    if (buf != NULL) {
        free (buf);
    }
}
