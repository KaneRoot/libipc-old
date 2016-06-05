#include "pubsubd.h"
#include <stdlib.h>

void
ohshit(int rvalue, const char* str) {
    fprintf(stderr, "%s\n", str);
    exit(rvalue);
}

// CHANNELS

void pubsubd_channels_init (struct channels *chans) { LIST_INIT(chans); }

void
pubsubd_channels_add (struct channels *chans, struct channel *c)
{
    if(!chans || !c)
        return;

    struct channel *n = pubsubd_channel_copy (c);
    LIST_INSERT_HEAD(chans, n, entries);
}

void
pubsubd_channels_del (struct channels *chans, struct channel *c)
{
    struct channel *todel = pubsubd_channel_get (chans, c);
    if(todel != NULL) {
        LIST_REMOVE(todel, entries);
        srv_process_free (todel);
        free (todel);
        todel = NULL;
    }
}

struct channel * pubsubd_channel_copy (struct channel *c)
{
    struct channel *copy;
    copy = malloc (sizeof(struct channel));
    memcpy (copy, c, sizeof(struct channel));
    return copy;
}

struct channel * pubsubd_channel_get (struct channels *chans, struct channel *c)
{
    struct channel * np = NULL;
    LIST_FOREACH(np, chans, entries) {
        if (pubsubd_channels_eq (np, c))
            return np;
    }
    return NULL;
}

int
pubsubd_channels_eq (const struct channel *c1, const struct channel *c2)
{
    return (strncmp (c1->chan, c2->chan, c1->chanlen) == 0);
}

// SUBSCRIBER

void pubsubd_subscriber_init (struct app_list_head *chans) { LIST_INIT(chans); } 

struct app_list_elm * pubsubd_app_list_elm_copy (struct app_list_elm *ale)
{
    if (ale == NULL)
        return NULL;

    struct app_list_elm * n;
    n = malloc (sizeof (struct app_list_elm));

    n->p = srv_process_copy(ale->p);

    return n;
}

void
pubsubd_subscriber_add (struct app_list_head *alh, struct app_list_elm *ale)
{
    if(!alh || !ale)
        return;

    struct app_list_elm *n = pubsubd_app_list_elm_copy (ale);
    LIST_INSERT_HEAD(alh, n, entries);
}

struct app_list_elm *
pubsubd_subscriber_get (const struct app_list_head *chans, const struct app_list_elm *p)
{
    struct app_list_elm *np, *res = NULL;
    LIST_FOREACH(np, chans, entries) {
        if(srv_process_eq (np, p)) {
            res = np;
        }
    }
    return res;
}

void
pubsubd_subscriber_del (struct app_list_head *chans, struct app_list_elm *p)
{
    struct app_list_elm *todel = pubsubd_subscriber_get (chans, p);
    if(todel != NULL) {
        LIST_REMOVE(todel, entries);
        pubsubd_app_list_elm_free (todel);
        free (todel);
        todel = NULL;
    }
}

void pubsubd_app_list_elm_create (struct app_list_elm *ale, struct process *p)
{
    if (ale == NULL)
        return;

    ale->p = srv_process_copy (p);
}

void pubsubd_app_list_elm_free (struct app_list_elm *todel)
{
    if (todel == NULL)
        return NULL;
    srv_process_free (todel->p);
}

// MESSAGE, TODO CBOR

void pubsubd_msg_serialize (const struct pubsub_msg *msg, char **data, size_t *len)
{
    if (msg == NULL || data == NULL || len == NULL)
        return;

    // msg: "type(1) chanlen(8) chan datalen(8) data
    *len = 1 + sizeof(size_t) + msg->chanlen + sizeof(size_t) + msg->datalen;
    *data = malloc(*len);

    size_t i = 0;

    data[0][i] = msg->type;                             i++;
    memcpy (data[0][i], msg->chanlen, sizeof(size_t));   i += sizeof(size_t);
    memcpy (data[0][i], msg->chan, msg->chanlen);        i += msg->chanlen;
    memcpy (data[0][i], msg->datalen, sizeof(size_t));   i += sizeof(size_t);
    memcpy (data[0][i], msg->data, msg->datalen);        i += msg->datalen;
}

void pubsubd_msg_unserialize (struct pubsub_msg *msg, const char *data, size_t len)
{
    if (msg == NULL || data == NULL)
        return;

    size_t i = 0;
    msg->type = data[0][i];                             i++;
    memcpy (&msg->chanlen, data + i, sizeof(size_t));   i += sizeof(size_t);
    msg->chan = malloc (msg->chanlen);
    memcpy (msg->chan, data + i, msg->chanlen);         i += msg->chanlen;
    memcpy (&msg->datalen, data + i, sizeof(size_t));   i += sizeof(size_t);
    msg->data = malloc (msg->datalen);
    memcpy (msg->data, data + i, msg->datalen);         i += msg->datalen;
}

void pubsubd_msg_free (struct pubsub_msg *msg)
{
    if (msg->chan) {
        free (msg->chan);
        msg->chan = 0;
    }
    if (msg->data) {
        free (msg->data);
        msg->data = 0;
    }
}

// COMMUNICATION

void pubsubd_msg_send (struct service *s, struct pubsub_msg * m, struct process *p)
{
}
void pubsubd_msg_recv (struct service *s, struct pubsub_msg * m, struct process *p)
{
}
void pubsub_msg_send (struct service *s, struct pubsub_msg * m)
{
}
void pubsub_msg_recv (struct service *s, struct pubsub_msg * m)
{
}

// SERVICE

void pubsubd_srv_init ();
