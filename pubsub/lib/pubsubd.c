#include "pubsubd.h"

// WORKERS: one thread per client

void pubsubd_workers_init (struct workers *wrkrs) { LIST_INIT(wrkrs); }

struct worker *
pubsubd_workers_add (struct workers *wrkrs, const struct worker *w)
{
    if (wrkrs == NULL || w == NULL) {
        printf ("pubsubd_workers_add: wrkrs == NULL or w == NULL");
        return NULL;
    }

    struct worker *n = malloc (sizeof (struct worker));
    memset (n, 0, sizeof (struct worker));
    memcpy (n, w, sizeof (struct worker));
    if (w->ale != NULL)
        n->ale = pubsubd_app_list_elm_copy (w->ale);

    LIST_INSERT_HEAD(wrkrs, n, entries);

    return n;
}

void pubsubd_worker_del (struct workers *wrkrs, struct worker *w)
{
    struct worker *todel = pubsubd_worker_get (wrkrs, w);
    if(todel != NULL) {
        LIST_REMOVE(todel, entries);
        pubsubd_worker_free (todel);
        free (todel);
        todel = NULL;
    }
}

// kill the threads
void pubsubd_workers_stop (struct workers *wrkrs)
{
    if (!wrkrs)
        return;

    struct worker *w = NULL;
    struct worker *wtmp = NULL;

    LIST_FOREACH_SAFE(w, wrkrs, entries, wtmp) {
        if (w->thr == NULL)
            continue;

        pthread_cancel (*w->thr);
        void *ret = NULL;
        pthread_join (*w->thr, &ret);
        if (ret != NULL) {
            free (ret);
        }
        free (w->thr);
        w->thr = NULL;
    }
}

void pubsubd_workers_del_all (struct workers *wrkrs)
{
    if (!wrkrs)
        return;

    struct worker *w = NULL;

    while (!LIST_EMPTY(wrkrs)) {
        printf ("KILL THE WORKERS : %p\n", w);
        w = LIST_FIRST(wrkrs);
        LIST_REMOVE(w, entries);
        pubsubd_worker_free (w);
        free (w);
        w = NULL;
    }
}

void pubsubd_worker_free (struct worker * w)
{
    if (w == NULL)
        return;
    pubsubd_app_list_elm_free (w->ale);
    free (w->ale);
    w->ale = NULL;
}

struct worker * pubsubd_worker_get (struct workers *wrkrs, struct worker *w)
{
    struct worker * np = NULL;
    LIST_FOREACH(np, wrkrs, entries) {
        if (pubsubd_worker_eq (np, w))
            return np;
    }
    return NULL;
}

int pubsubd_worker_eq (const struct worker *w1, const struct worker *w2)
{
    return w1 == w2; // if it's the same pointer
}

// a thread for each connected process
void * pubsubd_worker_thread (void *params)
{
    int s = 0;

    s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (s != 0)
        printf ("pthread_setcancelstate: %d\n", s);

    struct worker *w = (struct worker *) params;
    if (w == NULL) {
        fprintf (stderr, "error pubsubd_worker_thread : params NULL\n");
        return NULL;
    }

    struct channels *chans = w->chans;
    struct channel *chan = w->chan;
    struct app_list_elm *ale = w->ale;

    // main loop
    while (1) {
        struct pubsub_msg m;
        memset (&m, 0, sizeof (struct pubsub_msg));

        pubsubd_msg_recv (ale->p, &m);

        if (m.type == PUBSUB_TYPE_DISCONNECT) {
            // printf ("process %d disconnecting...\n", ale->p->pid);
            if ( 0 != pubsubd_subscriber_del (chan->alh, ale)) {
                fprintf (stderr, "err : subscriber not registered\n");
            }
            break;
        }
        else {
            struct channel *ch = pubsubd_channel_search (chans, chan->chan);
            if (ch == NULL) {
                printf ("CHAN NOT FOUND\n");
            }
            else {
                printf ("what should be sent: ");
                pubsubd_msg_print (&m);
                printf ("send the message to:\t");
                pubsubd_channel_print (ch);
                pubsubd_msg_send (ch->alh, &m);
            }
        }
        pubsubd_msg_free (&m);
    }

    pubsubd_app_list_elm_free (ale);
    free (w->ale);
    w->ale = NULL;

    free (w->thr);
    w->thr = NULL;

    pubsubd_worker_del (w->my_workers, w);

    pthread_exit (NULL);
}

// CHANNELS

void pubsubd_channels_init (struct channels *chans) { LIST_INIT(chans); }

struct channel *
pubsubd_channels_add (struct channels *chans, const char *chan)
{
    if(chans == NULL || chan == NULL) {
        printf ("pubsubd_channels_add: chans == NULL or chan == NULL");
        return NULL;
    }

    struct channel *n = malloc (sizeof (struct channel));
    memset (n, 0, sizeof (struct channel));
    pubsubd_channel_new (n, chan);

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
        copy->chan = malloc (c->chanlen +1);
        memset (copy->chan, 0, c->chanlen +1);
        memcpy (copy->chan, c->chan, c->chanlen);
        copy->chanlen = c->chanlen;
    }
    else {
        printf ("pubsubd_channel_copy: c->chan == NULL\n");
    }

    return copy;
}

int pubsubd_channel_new (struct channel *c, const char * name)
{
    if (c == NULL) {
        return 1;
    }

    size_t nlen = (strlen (name) > BUFSIZ) ? BUFSIZ : strlen (name);

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

struct channel * pubsubd_channel_search (struct channels *chans, char *chan)
{
    struct channel * np = NULL;
    LIST_FOREACH(np, chans, entries) {
        // TODO debug
        // printf ("pubsubd_channel_search: %s (%ld) vs %s (%ld)\n"
        //         , np->chan, np->chanlen, chan, strlen(chan));
        if (np->chanlen == strlen (chan)
                && strncmp (np->chan, chan, np->chanlen) == 0) {
        //    printf ("pubsubd_channel_search: FOUND\n");
            return np;
        }
    }
    return NULL;
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
    return c1->chanlen == c2->chanlen &&
        strncmp (c1->chan, c2->chan, c1->chanlen) == 0;
}

// SUBSCRIBER

void pubsubd_subscriber_init (struct app_list_head **chans) {
    if (chans == NULL)
        return;

    if (*chans == NULL) {
        *chans = malloc (sizeof(struct channels));
        memset (*chans, 0, sizeof(struct channels));
    }
    LIST_INIT(*chans);
} 

void pubsubd_channel_print (const struct channel *chan)
{
    if (chan->chan == NULL) {
        printf ("pubsubd_channels_print: chan->chan == NULL\n");
    }

    printf ( "\033[32mchan %s\033[00m\n", chan->chan);

    if (chan->alh == NULL)
        printf ("pubsubd_channels_print: chan->alh == NULL\n");
    else
        pubsubd_subscriber_print (chan->alh);
}

void pubsubd_channels_print (const struct channels *chans)
{
    printf ("\033[36mmchannels\033[00m\n");

    if (chans == NULL) {
        // TODO debug
        printf ("pubsubd_channels_print: chans == NULL\n");
        return ;
    }

    struct channel *chan = NULL;
    LIST_FOREACH(chan, chans, entries) {
        pubsubd_channel_print (chan);
    }
}

struct app_list_elm * pubsubd_app_list_elm_copy (const struct app_list_elm *ale)
{
    if (ale == NULL)
        return NULL;

    struct app_list_elm * n = NULL;
    n = malloc (sizeof (struct app_list_elm));
    memset (n, 0, sizeof (struct app_list_elm));

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
    if(alh == NULL || ale == NULL) {
        fprintf (stderr, "err alh or ale is NULL\n");
        return;
    }

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

void pubsubd_subscriber_print (struct app_list_head *alh)
{
    struct app_list_elm *np = NULL;
    LIST_FOREACH(np, alh, entries) {
        printf ("\t");
        srv_process_print (np->p);
    }
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

    if (ale->p != NULL)
        free (ale->p);

    ale->p = srv_process_copy (p);
}

void pubsubd_app_list_elm_free (struct app_list_elm *todel)
{
    if (todel == NULL || todel->p == NULL)
        return;
    free (todel->p);
}

int pubsubd_get_new_process (const char *spath, struct app_list_elm *ale
        , struct channels *chans, struct channel **c)
{
    if (spath == NULL || ale == NULL || chans == NULL) {
        fprintf (stderr, "pubsubd_get_new_process: spath or ale or chans == NULL\n");
        return -1;
    }

    char *buf = NULL;
    size_t msize = 0;
    file_read (spath, &buf, &msize);
    // parse pubsubd init msg (sent in TMPDIR/<service>)
    //
    // line fmt : index version action chan
    // action : quit | pub | sub

    size_t i = 0;
    char *str = NULL, *token = NULL, *saveptr = NULL;

    int index = 0;
    int version = 0;

    // chan name
    char chan[BUFSIZ];
    memset (chan, 0, BUFSIZ);

    if (buf == NULL) {
        return -2;
    }

    printf ("INIT: %s\n", buf);

    for (str = buf, i = 1; ; str = NULL, i++) {
        token = strtok_r(str, " ", &saveptr);
        if (token == NULL)
            break;

        switch (i) {
            case 1 : index = strtoul(token, NULL, 10); break;
            case 2 : version = strtoul(token, NULL, 10); break;
            case 3 : {
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
            case 4 : {
                         // for the last element of the line
                         // drop the following \n
                         if (ale->action != PUBSUB_QUIT) {
                             memcpy (chan, token, (strlen (token) < BUFSIZ) ?
                                     strlen (token) -1 : BUFSIZ);
                         }
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
    memset (ale->p, 0, sizeof (struct process)); 
    srv_process_gen (ale->p, index, version);

    chan[BUFSIZ -1] = '\0';

    // not found = new
    struct channel *new_chan = NULL;
    new_chan = pubsubd_channel_search (chans, chan);
    if (new_chan == NULL) {
        new_chan = pubsubd_channels_add (chans, chan);
        pubsubd_subscriber_init (&new_chan->alh);
    }

    *c = new_chan;

    // add the subscriber
    if (ale->action == PUBSUB_SUB || ale->action == PUBSUB_BOTH) {
        printf ("new process in chan %s\n", chan);
        pubsubd_subscriber_add ((*c)->alh, ale);
    }

    return 0;
}

// alh from the channel, message to send
void pubsubd_msg_send (const struct app_list_head *alh, const struct pubsub_msg * m)
{
    if (alh == NULL) {
        fprintf (stderr, "pubsubd_msg_send: alh == NULL");
        return;
    }

    if (m == NULL) {
        fprintf (stderr, "pubsubd_msg_send: m == NULL");
        return;
    }

    struct app_list_elm * ale = NULL;

    char *buf = NULL;
    size_t msize = 0;
    pubsubd_msg_serialize (m, &buf, &msize);

    LIST_FOREACH(ale, alh, entries) {
        srv_write (ale->p->proc_fd, buf, msize);
    }

    if (buf != NULL) {
        free (buf);
    }
}

void pubsubd_msg_recv (struct process *p, struct pubsub_msg *m)
{
    // read the message from the process
    size_t mlen = 0;
    char *buf = NULL;
    while (buf == NULL || mlen == 0) {
        srv_read (p->proc_fd, &buf, &mlen);
    }

    pubsubd_msg_unserialize (m, buf, mlen);

    if (buf != NULL) {
        free (buf);
    }
}
