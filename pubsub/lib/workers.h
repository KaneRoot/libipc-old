#if 0
#ifndef __WORKERS_H__
#define __WORKERS_H__

// WORKERS: one thread per client

// head of the list
LIST_HEAD(workers, worker);

// element of the list
// worker : process to handle (threaded)
struct worker {
    pthread_t *thr;
    struct workers *my_workers;
    struct channels *chans;
    struct channel *chan;
    struct subscriber *ale;
    LIST_ENTRY(worker) entries;
};

void pubsubd_worker_free (struct worker * w);
struct worker * pubsubd_worker_get (struct workers *wrkrs, struct worker *w);
int pubsubd_worker_eq (const struct worker *w1, const struct worker *w2);
void pubsubd_workers_init (struct workers *wrkrs);
void * pubsubd_worker_thread (void *params);
struct worker * pubsubd_workers_add (struct workers *wrkrs, const struct worker *w);
void pubsubd_workers_del_all (struct workers *wrkrs);
void pubsubd_workers_stop (struct workers *wrkrs);
void pubsubd_worker_del (struct workers *wrkrs, struct worker *w);

#endif
#endif
