#ifndef __PUBSUBD_H__
#define __PUBSUBD_H__

#include "pubsub.h"
#include <pthread.h>

struct channel;
struct channels;
struct app_list_head;
struct app_list_elm;

// parse pubsubd init msg (sent in TMPDIR/<service>)
//
// line fmt : pid index version action chan
// action : quit | pub | sub
int pubsubd_get_new_process (const char *spath, struct app_list_elm *ale
        , struct channels *chans, struct channel **c);
void pubsubd_msg_send (const struct app_list_head *alh, const struct pubsub_msg *m);
void pubsubd_msg_recv (struct process *p, struct pubsub_msg *m);

// CHANNEL

// head of the list
LIST_HEAD(channels, channel);

// element of the list
// channel : chan name + chan name length + a list of applications
struct channel {
    char *chan;
    size_t chanlen;
    struct app_list_head *alh;
    LIST_ENTRY(channel) entries;
};

//  simple channel
int pubsubd_channel_new (struct channel *c, const char *name);
struct channel * pubsubd_channel_copy (struct channel *c);
struct channel * pubsubd_channel_get (struct channels *chans, struct channel *c);
void pubsubd_channel_free (struct channel *c);
int pubsubd_channel_eq (const struct channel *c1, const struct channel *c2);
void pubsubd_channel_print (const struct channel *c);

// list of channels
void pubsubd_channels_init (struct channels *chans);
void pubsubd_channels_print (const struct channels *chans);
struct channel * pubsubd_channels_add (struct channels *chans, const char *chan);
void pubsubd_channels_del (struct channels *chans, struct channel *c);
void pubsubd_channels_del_all (struct channels *chans);
struct channel * pubsubd_channel_search (struct channels *chans, char *chan);

// remove an app_list_elm from the list (msg type DISCONNECT received)
int pubsubd_channels_del_subscriber (struct channels *chans
        , struct channel *c);

struct channel *
pubsubd_channels_search_from_app_list_elm (struct channels *chans
        , struct app_list_elm *ale);

// APPLICATION

// head of the list
LIST_HEAD(app_list_head, app_list_elm);

// element of the list
struct app_list_elm {
    struct process *p;
    enum app_list_elm_action action;
    LIST_ENTRY(app_list_elm) entries;
};

int
pubsubd_subscriber_eq (const struct app_list_elm *, const struct app_list_elm *);

void pubsubd_subscriber_init (struct app_list_head **chans);
void pubsubd_subscriber_print (struct app_list_head *alh);
void pubsubd_subscriber_add (struct app_list_head *
        , const struct app_list_elm *);
struct app_list_elm * pubsubd_subscriber_get (const struct app_list_head *
        , const struct app_list_elm *);
int pubsubd_subscriber_del (struct app_list_head *al, struct app_list_elm *p);
void pubsubd_subscriber_del_all (struct app_list_head *alh);

struct app_list_elm * pubsubd_app_list_elm_copy (const struct app_list_elm *ale);
void pubsubd_app_list_elm_create (struct app_list_elm *ale, struct process *p);
void pubsubd_app_list_elm_free (struct app_list_elm *todel);

void pubsubd_quit (struct service *srv);

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
    struct app_list_elm *ale;
    LIST_ENTRY(worker) entries;
};

void pubsubd_worker_free (struct worker * w);
struct worker * pubsubd_worker_get (struct workers *wrkrs, struct worker *w);
int pubsubd_worker_eq (const struct worker *w1, const struct worker *w2);
void pubsubd_workers_init (struct workers *wrkrs);
void * pubsubd_worker_thread (void *params);
struct worker *
pubsubd_workers_add (struct workers *wrkrs, const struct worker *w);
void pubsubd_workers_del_all (struct workers *wrkrs);
void pubsubd_workers_stop (struct workers *wrkrs);
void pubsubd_worker_del (struct workers *wrkrs, struct worker *w);

#endif
