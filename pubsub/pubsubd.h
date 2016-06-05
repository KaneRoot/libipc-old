#ifndef __PUBSUBD_H__
#define __PUBSUBD_H__

#include "../lib/communication.h"
#include "../lib/process.h"

#include "../lib/queue.h"

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

struct channel * pubsubd_channel_copy (struct channel *c);

int pubsubd_channel_eq (const struct channel *c1, const struct channel *c2);

struct channels {
    struct channel *chan;
    LIST_ENTRY(channels) entries;
};

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
