#ifndef __PUBSUBD_H__
#define __PUBSUBD_H__

#include "queue.h"

struct message {
    unsigned char *chan;
    size_t chanlen;
    unsigned char *data;
    size_t datalen;
    unsigned char type; // message type : alert, notification, …
};

struct channel {
    unsigned char *chan;
    size_t chanlen;
};

struct channels {
    struct channel *chan;
    LIST_ENTRY(channels) entries;
};

int pubsubd_channels_eq (const struct channels *c1, const struct channels *c2);

struct app_list {
    struct process *p;
    LIST_ENTRY(app_list) entries;
};

void pubsubd_msg_send (struct service *, struct message *msg, struct process *p);
void pubsubd_msg_recv (struct service *, struct message *msg, struct process *p);

struct process * pubsubd_subscriber_get (const struct app_list *
        , const struct process *);
void pubsubd_subscriber_del (struct app_list *al, struct process *p);

void pubsub_msg_send (struct service *, struct message *msg);
void pubsub_msg_recv (struct service *, struct message *msg);

#endif
