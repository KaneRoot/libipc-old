#ifndef __PUBSUBD_H__
#define __PUBSUBD_H__

#include "communication.h"
#include "process.h"

#include "queue.h"

#define PUBSUB_SERVICE_NAME "pubsub"

struct pubsub_msg {
    unsigned char *chan;
    size_t chanlen;
    unsigned char *data;
    size_t datalen;
    unsigned char type; // message type : alert, notification, …
};

void pubsubd_msg_serialize (const struct pubsub_msg *msg, char **data, size_t *len);
void pubsubd_msg_unserialize (struct pubsub_msg *msg, const char *data, size_t len);
void pubsubd_msg_free (struct pubsub_msg *msg);

void pubsubd_msg_send (struct service *, struct pubsub_msg *msg, struct process *p);
void pubsubd_msg_recv (struct service *, struct pubsub_msg *msg, struct process *p);

void pubsub_msg_send (struct service *, struct pubsub_msg *msg);
void pubsub_msg_recv (struct service *, struct pubsub_msg *msg);

// CHANNEL

// head of the list
LIST_HEAD(channels, channel);

// element of the list
struct channel {
    char *chan;
    size_t chanlen;
    LIST_ENTRY(channel) entries;
};

void pubsubd_channels_init (struct channels *chans);
struct channel * pubsubd_channel_copy (struct channel *c);
struct channel * pubsubd_channel_get (struct channels *chans, struct channel *c);

void pubsubd_channel_free (struct channel *c);
int pubsubd_channel_eq (const struct channel *c1, const struct channel *c2);

// APPLICATION

// head of the list
LIST_HEAD(app_list_head, app_list_elm);

// element of the list
struct app_list_elm {
    struct process *p;
    LIST_ENTRY(app_list_elm) entries;
};

int
pubsubd_subscriber_eq (const struct app_list_elm *, const struct app_list_elm *);

void pubsubd_subscriber_add (struct app_list_head *
        , const struct app_list_elm *);
struct app_list_elm * pubsubd_subscriber_get (const struct app_list_head *
        , const struct app_list_elm *);
void pubsubd_subscriber_del (struct app_list_head *al, struct app_list_elm *p);

struct app_list_elm * pubsubd_app_list_elm_copy (const struct app_list_elm *ale);
void pubsubd_app_list_elm_create (struct app_list_elm *ale, struct process *p);
void pubsubd_app_list_elm_free (struct app_list_elm *todel);

#endif
