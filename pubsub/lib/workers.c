#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../core/error.h"

#include <pthread.h>

// WORKERS: one thread per client

void pubsubd_workers_init (struct workers *wrkrs) { LIST_INIT(wrkrs); }

struct worker * pubsubd_workers_add (struct workers *wrkrs, const struct worker *w)
{
    if (wrkrs == NULL || w == NULL) {
        printf ("pubsubd_workers_add: wrkrs == NULL or w == NULL");
        return NULL;
    }

    struct worker *n = malloc (sizeof (struct worker));
    memset (n, 0, sizeof (struct worker));
    memcpy (n, w, sizeof (struct worker));
    if (w->ale != NULL)
        n->ale = pubsubd_subscriber_copy (w->ale);

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
    pubsubd_subscriber_free (w->ale);
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

// a thread for each connected client
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
    struct subscriber *ale = w->ale;

    // main loop
    while (1) {
        struct pubsub_msg m;
        memset (&m, 0, sizeof (struct pubsub_msg));

        pubsub_message_recv (ale->p, &m);

        if (m.type == PUBSUB_TYPE_DISCONNECT) {
            // printf ("client %d disconnecting...\n", ale->p->pid);
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
                pubsub_message_print (&m);
                printf ("send the message to:\t");
                pubsubd_channel_print (ch);
                pubsub_message_send (ch->alh, &m);
            }
        }
        pubsub_message_free (&m);
    }

    pubsubd_subscriber_free (ale);
    free (w->ale);
    w->ale = NULL;

    free (w->thr);
    w->thr = NULL;

    pubsubd_worker_del (w->my_workers, w);

    pthread_exit (NULL);
}
#endif
