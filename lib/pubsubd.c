#include "pubsubd.h"
#include <stdlib.h>

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
        pubsubd_channel_free (todel);
        free (todel);
        todel = NULL;
    }
}

void pubsubd_channels_del_all (struct channels *chans)
{
    if (!chans)
        return;

    struct channel *c;

    while (!LIST_EMPTY(chans)) {
        c = LIST_FIRST(chans);
        LIST_REMOVE(c, entries);
        pubsubd_channel_free (c);
        free (c);
        c = NULL;
    }
}

struct channel * pubsubd_channel_copy (struct channel *c)
{
    struct channel *copy;
    copy = malloc (sizeof(struct channel));
    memcpy (copy, c, sizeof(struct channel));
    return copy;
}

void pubsubd_channel_free (struct channel * c)
{
}

struct channel * pubsubd_channel_get (struct channels *chans, struct channel *c)
{
    struct channel * np = NULL;
    LIST_FOREACH(np, chans, entries) {
        if (pubsubd_channel_eq (np, c))
            return np;
    }
    return NULL;
}

int
pubsubd_channel_eq (const struct channel *c1, const struct channel *c2)
{
    return (strncmp (c1->chan, c2->chan, c1->chanlen) == 0);
}

// SUBSCRIBER

void pubsubd_subscriber_init (struct app_list_head *chans) { LIST_INIT(chans); } 

void pubsubd_app_list_elm_print (const struct app_list_elm *ale)
{
    printf ( "app_list_elm\n\t");
    srv_process_print (ale->p);

    printf ( "\tchan : %s\n", ale->chan);
    printf ( "\taction : %d\n", (int) ale->action);
}

struct app_list_elm * pubsubd_app_list_elm_copy (const struct app_list_elm *ale)
{
    if (ale == NULL)
        return NULL;

    struct app_list_elm * n;
    n = malloc (sizeof (struct app_list_elm));

    n->p = srv_process_copy(ale->p);

    return n;
}

int
pubsubd_subscriber_eq (const struct app_list_elm *ale1, const struct app_list_elm *ale2)
{
    return srv_process_eq (ale1->p, ale2->p);
}


void
pubsubd_subscriber_add (struct app_list_head *alh, const struct app_list_elm *ale)
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
        if(pubsubd_subscriber_eq (np, p)) {
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
        return;
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
    memcpy (&data[0][i], &msg->chanlen, sizeof(size_t)); i += sizeof(size_t);
    memcpy (&data[0][i], msg->chan, msg->chanlen);       i += msg->chanlen;
    memcpy (&data[0][i], &msg->datalen, sizeof(size_t)); i += sizeof(size_t);
    memcpy (&data[0][i], msg->data, msg->datalen);       i += msg->datalen;
}

void pubsubd_msg_unserialize (struct pubsub_msg *msg, const char *data, size_t len)
{
    if (msg == NULL || data == NULL)
        return;

    size_t i = 0;
    msg->type = data[i];                             i++;
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

int pubsubd_get_new_process (struct service *srv, struct app_list_elm *ale)
{
    if (ale->p != NULL) {
        free (ale->p);
    }

    ale->p = malloc (sizeof (struct process));

    char *buf;
    size_t msize;
    srv_get_listen_raw (srv, &buf, &msize);

    // parse pubsubd init msg (sent in TMPDIR/<service>)
    //
    // line fmt : pid index version chan action
    // action : pub | sub

    size_t i;
    char *str, *token, *saveptr;

    pid_t pid;
    int index;
    int version;

    for (str = buf, i = 1; ; str = NULL, i++) {
        token = strtok_r(str, " ", &saveptr);
        if (token == NULL)
            break;

        switch (i) {
            case 1 : pid = strtoul(token, NULL, 10); break;
            case 2 : index = strtoul(token, NULL, 10); break;
            case 3 : version = strtoul(token, NULL, 10); break;
            case 4 : memcpy (ale->chan, token, strlen (token)); break;
            case 5 : {
                         if (strncmp("pub", token, 3) == 0) {
                             ale->action = 0;
                         }
                         else if (strncmp("sub", token, 3) == 0) {
                             ale->action = 1;
                         }
                         else if (strncmp("both", token, 4) == 0) {
                             ale->action = 2;
                         }
                         else { // everything else is about killing the service
                             ale->action = 3;
                         }
                     }
        }
    }

    srv_process_gen (ale->p, pid, index, version);
    ale->chanlen = strlen (ale->chan);

    return 0;
}

// TODO CBOR
int pubsubd_msg_read_cb (FILE *f, char ** buf, size_t * msize)
{
    // msg: "type(1) chanlen(8) chan datalen(8) data

    // read 
    char type;
    fread (&type, 1, 1, f);

    size_t chanlen;
    fread (&chanlen, sizeof (size_t), 1, f);

    char *chan = malloc (chanlen);
    fread (chan, chanlen, 1, f);

    size_t datalen;
    fread (&datalen, sizeof (size_t), 1, f);

    char *data = malloc (datalen);
    fread (data, datalen, 1, f);

    *msize = 1 + chanlen;
    *buf = malloc(*msize);

    // TODO CHECK THIS
    size_t i = 0;

    char *cbuf = *buf;

    cbuf[i] = type;                                  i++;
    memcpy (&cbuf[i], &chanlen, sizeof(size_t));     i += sizeof(size_t);
    memcpy (&cbuf[i], chan, chanlen);                i += chanlen;
    memcpy (&cbuf[i], &datalen, sizeof(size_t));     i += sizeof(size_t);
    memcpy (&cbuf[i], data, datalen);                i += datalen;
    return 0;
}

void pubsubd_msg_send (const struct app_list_head *alh, const struct pubsub_msg * m)
{
}
void pubsubd_msg_recv (struct process *p, struct pubsub_msg *m)
{
    // read the message from the process
    size_t mlen;
    char *buf;
    srv_read_cb (p, &buf, &mlen, pubsubd_msg_read_cb);

    pubsubd_msg_unserialize (m, buf, mlen);
    free (buf);
}

void pubsub_msg_send (const struct service *s, const struct pubsub_msg * m)
{
}
void pubsub_msg_recv (const struct service *s, struct pubsub_msg * m)
{
}

// SERVICE

void pubsubd_srv_init ();
