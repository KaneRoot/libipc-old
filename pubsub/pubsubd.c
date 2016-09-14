#include "../lib/pubsubd.h"
#include <stdlib.h>
#include <pthread.h>

#define NB_CLIENTS                                      3

void
ohshit(int rvalue, const char* str) {
    fprintf(stderr, "%s\n", str);
    exit(rvalue);
}

// head of the list
LIST_HEAD(workers, worker);

struct workers *my_workers;

// element of the list
// worker : process to handle (threaded)
struct worker {
    pthread_t *thr;
    struct channels *chans;
    struct channel *chan;
    struct app_list_elm *ale;
    LIST_ENTRY(worker) entries;
};

void pubsubd_worker_free (struct worker * w);
struct worker * pubsubd_worker_get (struct workers *wrkrs, struct worker *w);
int pubsubd_worker_eq (const struct worker *w1, const struct worker *w2);

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
            printf ("process %d disconnecting...\n", ale->p->pid);
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

    pubsubd_worker_del (my_workers, w);

    pthread_exit (NULL);
}

int
main(int argc, char **argv, char **env)
{
    struct service srv;
    memset (&srv, 0, sizeof (struct service));
    srv_init (argc, argv, env, &srv, PUBSUB_SERVICE_NAME, NULL);
    printf ("Listening on %s.\n", srv.spath);

    // creates the service named pipe, that listens to client applications
    if (srv_create (&srv))
        ohshit(1, "service_create error");

    // init chans list
    struct channels chans;
    memset (&chans, 0, sizeof (struct channels));
    pubsubd_channels_init (&chans);

    my_workers = malloc (sizeof (struct workers));
    memset (my_workers, 0, sizeof (struct workers));
    pubsubd_workers_init (my_workers);

    int i = 0;
    // for (i = 0; i < NB_CLIENTS; i++)
    for (i = 0; ; i++) {
        // for each new process
        struct app_list_elm ale;
        memset (&ale, 0, sizeof (struct app_list_elm));
        struct channel *chan = NULL;
        pubsubd_get_new_process (srv.spath, &ale, &chans, &chan);
        pubsubd_channels_print (&chans);

        // end the application
        if (ale.action == PUBSUB_QUIT) {
            pubsubd_app_list_elm_free (&ale);
            break;
        }

        // thread to handle multiple clients at a time
        struct worker *w = NULL;
        w = malloc (sizeof (struct worker));
        w->thr = malloc (sizeof (pthread_t));
        memset (w->thr, 0, sizeof (pthread_t));
        w->ale = pubsubd_app_list_elm_copy (&ale);
        w->chans = &chans;
        w->chan = chan;
        struct worker *wtmp = pubsubd_workers_add (my_workers, w);
        pubsubd_worker_free (w);
        free (w);
        w = wtmp;

        pthread_create (w->thr, NULL, pubsubd_worker_thread, w);
        pthread_detach (*w->thr);

        pubsubd_app_list_elm_free (&ale);
    }

    printf ("Quitting ...\n");

    pubsubd_workers_stop (my_workers);
    pubsubd_channels_del_all (&chans);
    pubsubd_workers_del_all (my_workers);

    free (my_workers);

    // the application will shut down, and remove the service named pipe
    if (srv_close (&srv))
        ohshit (1, "service_close error");

    return EXIT_SUCCESS;
}
