#include "pubsubd.h"
#include <stdlib.h>

// CHANNELS

void pubsubd_channels_init (struct channels *chans) { LIST_INIT(chans); }

struct channel *
pubsubd_channels_add (struct channels *chans, struct channel *c)
{
    if(!chans || !c)
        return NULL;

    struct channel *n = pubsubd_channel_copy (c);
    LIST_INSERT_HEAD(chans, n, entries);

    return n;
}

void
pubsubd_channels_del (struct channels *chans, struct channel *c)
{
    struct channel *todel = pubsubd_channel_get (chans, c);
    if(todel != NULL) {
        pubsubd_channel_free (todel);
        LIST_REMOVE(todel, entries);
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
    if (c == NULL)
        return NULL;

    struct channel *copy;
    copy = malloc (sizeof(struct channel));
    bzero (copy, sizeof (struct channel));

    memcpy (copy, c, sizeof(struct channel));

    if (c->chan != NULL) {
        copy->chan = strndup (c->chan, c->chanlen);
        copy->chanlen = c->chanlen;
    }

    return copy;
}

void pubsubd_channel_free (struct channel * c)
{
    // TODO
    if (c == NULL)
        return;

    if (c->chan != NULL) {
        free (c->chan);
        c->chan = NULL;
    }

    if (c->alh != NULL) {
        pubsubd_subscriber_del_all (c->alh);
        free (c->alh);
    }
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

void pubsubd_subscriber_init (struct app_list_head **chans) {
    if (chans == NULL)
        return;

    if (*chans == NULL)
        *chans = malloc (sizeof(struct channels));
    LIST_INIT(*chans);
} 

void pubsubd_channels_print (const struct channels *chans)
{
    printf ("\033[36mmchannels\033[00m\n\n");

    if (chans == NULL)
        return ;

    struct channel *chan = NULL;
    LIST_FOREACH(chan, chans, entries) {
        pubsubd_channel_print (chan);
    }
}

void pubsubd_channel_print (const struct channel *c)
{
    if (c == NULL || c->chan == NULL)
        return;

    printf ( "\033[32mchan %s\033[00m\n\t", c->chan);

    if (c->alh == NULL)
        return;

    struct app_list_elm *ale = NULL;
    LIST_FOREACH(ale, c->alh, entries) {
        srv_process_print (ale->p);
    }
}

struct app_list_elm * pubsubd_app_list_elm_copy (const struct app_list_elm *ale)
{
    if (ale == NULL)
        return NULL;

    struct app_list_elm * n;
    n = malloc (sizeof (struct app_list_elm));

    if (ale->p != NULL)
        n->p = srv_process_copy (ale->p);

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
        pubsubd_app_list_elm_free (todel);
        LIST_REMOVE(todel, entries);
        free (todel);
        todel = NULL;
    }
}

void pubsubd_subscriber_del_all (struct app_list_head *alh)
{
    if (!alh)
        return;

    struct app_list_elm *ale;

    while (!LIST_EMPTY(alh)) {
        ale = LIST_FIRST(alh);
        LIST_REMOVE(ale, entries);
        pubsubd_app_list_elm_free (ale);
        free (ale);
        ale = NULL;
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
    if (todel == NULL || todel->p == NULL)
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

int pubsubd_get_new_process (struct service *srv, struct app_list_elm *ale
        , struct channels *chans)
{
    if (srv == NULL || ale == NULL || chans == NULL)
        return -1;

    if (ale->p != NULL) {
        free (ale->p);
        ale->p = NULL;
    }
    ale->p = malloc (sizeof (struct process));

    char *buf;
    size_t msize;
    srv_get_listen_raw (srv, &buf, &msize);

    // parse pubsubd init msg (sent in TMPDIR/<service>)
    //
    // line fmt : pid index version chan action
    // action : quit | pub | sub

    size_t i;
    char *str, *token, *saveptr;

    pid_t pid;
    int index;
    int version;

    char chan[BUFSIZ];
    bzero (chan, BUFSIZ);

    for (str = buf, i = 1; ; str = NULL, i++) {
        token = strtok_r(str, " ", &saveptr);
        if (token == NULL)
            break;

        switch (i) {
            case 1 : pid = strtoul(token, NULL, 10); break;
            case 2 : index = strtoul(token, NULL, 10); break;
            case 3 : version = strtoul(token, NULL, 10); break;
            case 4 : {
                         memcpy (chan, token, (strlen (token) < BUFSIZ) ?
                                 strlen (token) : BUFSIZ);
                         break;
                     }
            case 5 : {
                         if (strncmp("both", token, 4) == 0) {
                             ale->action = PUBSUB_BOTH;
                         }
                         else if (strncmp("pub", token, 3) == 0) {
                             ale->action = PUBSUB_PUB;
                         } 
                         else if (strncmp("sub", token, 3) == 0) {
                             ale->action = PUBSUB_SUB;
                         }
                         else { // everything else is about killing the service
                             ale->action = PUBSUB_QUIT;
                         }
                     }
        }
    }

    if (buf != NULL) {
        free (buf);
        buf = NULL;
    }

    chan[BUFSIZ -1] = '\0';

    struct channel c;
    bzero (&c, sizeof (struct channel));
    c.chan = strndup (chan, BUFSIZ);
    c.chanlen = strlen (chan);

    struct channel *new_chan;
    new_chan = pubsubd_channel_get (chans, &c);
    if (new_chan == NULL) {
        new_chan = pubsubd_channels_add (chans, &c);
        pubsubd_subscriber_init (&new_chan->alh);
    }
    pubsubd_channel_free (&c);

    srv_process_gen (ale->p, pid, index, version);

    // add the subscriber
    if (ale->action == PUBSUB_SUB || ale->action == PUBSUB_BOTH)
        pubsubd_subscriber_add (new_chan->alh, ale);

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

    free (chan);
    free (data);

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
