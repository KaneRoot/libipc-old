#include "pubsubd.h"
#include <stdlib.h>

#include <string.h> // strndup

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

    struct channel *c = NULL;

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

    struct channel *copy = NULL;
    copy = malloc (sizeof(struct channel));
    memset (copy, 0, sizeof (struct channel));

    memcpy (copy, c, sizeof(struct channel));

    if (c->chan != NULL) {
        copy->chan = malloc (c->chanlen);
        memset (copy->chan, 0, c->chanlen);
        memcpy (copy->chan, c->chan, c->chanlen);
        copy->chanlen = c->chanlen;
    }

    return copy;
}

int pubsubd_channel_new (struct channel *c, const char * name)
{
    if (c == NULL) {
        return 1;
    }

    size_t nlen = (strlen (name) > BUFSIZ) ? BUFSIZ : strlen (name);

    printf ("NAME : %s, SIZE : %ld\n", name, nlen);

    if (c->chan == NULL)
        c->chan = malloc (nlen +1);

    memset (c->chan, 0, nlen +1);
    memcpy (c->chan, name, nlen);
    c->chanlen = nlen;
    return 0;
}

void pubsubd_channel_free (struct channel * c)
{
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
    printf ("\033[36mmchannels\033[00m\n");

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

    if (c->chan == NULL) {
        printf ( "\033[32mchan name not available\033[00m\n");
    }
    else {
        printf ( "\033[32mchan %s\033[00m\n", c->chan);
    }

    if (c->alh == NULL)
        return;

    struct app_list_elm *ale = NULL;
    LIST_FOREACH(ale, c->alh, entries) {
        printf ("\t");
        srv_process_print (ale->p);
    }
}

struct app_list_elm * pubsubd_app_list_elm_copy (const struct app_list_elm *ale)
{
    if (ale == NULL)
        return NULL;

    struct app_list_elm * n = NULL;
    n = malloc (sizeof (struct app_list_elm));

    if (ale->p != NULL)
        n->p = srv_process_copy (ale->p);

    n->action = ale->action;

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
pubsubd_subscriber_get (const struct app_list_head *alh, const struct app_list_elm *p)
{
    struct app_list_elm *np = NULL, *res = NULL;
    LIST_FOREACH(np, alh, entries) {
        if(pubsubd_subscriber_eq (np, p)) {
            res = np;
        }
    }
    return res;
}

int
pubsubd_subscriber_del (struct app_list_head *alh, struct app_list_elm *p)
{
    struct app_list_elm *todel = pubsubd_subscriber_get (alh, p);
    if(todel != NULL) {
        pubsubd_app_list_elm_free (todel);
        LIST_REMOVE(todel, entries);
        free (todel);
        todel = NULL;
        return 0;
    }

    return 1;
}

void pubsubd_subscriber_del_all (struct app_list_head *alh)
{
    if (!alh)
        return;

    struct app_list_elm *ale = NULL;

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
    free (todel->p);
}

// MESSAGE, TODO CBOR

void pubsubd_msg_serialize (const struct pubsub_msg *msg, char **data, size_t *len)
{
    if (msg == NULL || data == NULL || len == NULL)
        return;

    // msg: "type(1) chanlen(8) chan datalen(8) data
    if (msg->type == PUBSUB_TYPE_DISCONNECT) {
        *len = 1;
        if (*data != NULL) {
            free (*data);
            *data = NULL;
        }
        *data = malloc(*len);
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
    msg->chan = malloc (msg->chanlen);
    memcpy (msg->chan, data + i, msg->chanlen);         i += msg->chanlen;

    memcpy (&msg->datalen, data + i, sizeof(size_t));   i += sizeof(size_t);
    if (msg->datalen > BUFSIZ) {
        fprintf (stderr, "\033[31merr : msg->datalen > BUFSIZ\033[00m\n");
        return;
    }
    msg->data = malloc (msg->datalen);
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

// COMMUNICATION

int pubsubd_get_new_process (const char *spath, struct app_list_elm *ale
        , struct channels *chans, struct channel **c)
{
    if (spath == NULL || ale == NULL || chans == NULL)
        return -1;

    char *buf = NULL;
    size_t msize = 0;
    file_read (spath, &buf, &msize);
    // parse pubsubd init msg (sent in TMPDIR/<service>)
    //
    // line fmt : pid index version action chan
    // action : quit | pub | sub

    size_t i = 0;
    char *str = NULL, *token = NULL, *saveptr = NULL;

    pid_t pid = 0;
    int index = 0;
    int version = 0;

    // chan name
    char chan[BUFSIZ];
    memset (chan, 0, BUFSIZ);

    printf ("INIT: %s\n", buf);

    for (str = buf, i = 1; ; str = NULL, i++) {
        token = strtok_r(str, " ", &saveptr);
        if (token == NULL)
            break;

        switch (i) {
            case 1 : pid = strtoul(token, NULL, 10); break;
            case 2 : index = strtoul(token, NULL, 10); break;
            case 3 : version = strtoul(token, NULL, 10); break;
            case 4 : {
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
                         break;
                     }
            case 5 : {
                         // for the last element of the line
                         // drop the following \n
                         if (ale->action != PUBSUB_QUIT)
                             memcpy (chan, token, (strlen (token) < BUFSIZ) ?
                                     strlen (token) -1 : BUFSIZ);
                         break;
                     }
        }
    }

    if (buf != NULL) {
        free (buf);
        buf = NULL;
    }

    if (ale->action == PUBSUB_QUIT) {
        return 0;
    }

    if (ale->p != NULL) {
        free (ale->p);
        ale->p = NULL;
    }

    ale->p = malloc (sizeof (struct process));
    srv_process_gen (ale->p, pid, index, version);

    if (*c == NULL) {
        *c = malloc (sizeof (struct channel));
    }

    chan[BUFSIZ -1] = '\0';
    pubsubd_channel_new (*c, chan);


    struct channel *new_chan = NULL;
    new_chan = pubsubd_channel_get (chans, *c);
    if (new_chan == NULL) {
        new_chan = pubsubd_channels_add (chans, *c);
        pubsubd_subscriber_init (&new_chan->alh);
    }

    pubsubd_channel_free (*c);
    *c = new_chan;

    // add the subscriber
    if (ale->action == PUBSUB_SUB || ale->action == PUBSUB_BOTH)
        pubsubd_subscriber_add (new_chan->alh, ale);

    return 0;
}

// alh from the channel, message to send
void pubsubd_msg_send (const struct app_list_head *alh, const struct pubsub_msg * m)
{
    struct app_list_elm * ale = NULL;

    char *buf = NULL;
    size_t msize = 0;
    pubsubd_msg_serialize (m, &buf, &msize);

    LIST_FOREACH(ale, alh, entries) {
        srv_write (ale->p, buf, msize);
    }

    if (buf != NULL) {
        free (buf);
    }
}

void pubsubd_msg_print (const struct pubsub_msg *msg)
{
    printf ("msg: type=%d chan=%s, data=%s\n"
            , msg->type, msg->chan, msg->data);
}

void pubsubd_msg_recv (struct process *p, struct pubsub_msg *m)
{
    // read the message from the process
    size_t mlen = 0;
    char *buf = NULL;
#if 0
    srv_read_cb (p, &buf, &mlen, pubsubd_msg_read_cb);
#else
    srv_read (p, &buf, &mlen);
#endif

    pubsubd_msg_unserialize (m, buf, mlen);

    if (buf != NULL) {
        free (buf);
    }
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

void pubsub_msg_recv (struct process *p, struct pubsub_msg * m)
{
    // read the message from the process
    size_t mlen = 0;
    char *buf = NULL;
    app_read (p, &buf, &mlen);

    pubsubd_msg_unserialize (m, buf, mlen);

    if (buf != NULL) {
        free (buf);
    }
}

// SERVICE

void pubsubd_srv_init ();
