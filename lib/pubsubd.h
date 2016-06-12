#ifndef __PUBSUBD_H__
#define __PUBSUBD_H__

#include "communication.h"
#include "process.h"
#include "queue.h"

#define PUBSUB_TYPE_DISCONNECT                                     1 << 0
#define PUBSUB_TYPE_INFO                                           1 << 1
#define PUBSUB_TYPE_DEBUG                                          1 << 2
#define PUBSUB_TYPE_MESSAGE                                        1 << 3

#define PUBSUB_SERVICE_NAME "pubsub"

struct channel;
struct channels;
struct app_list_head;
struct app_list_elm;
struct pubsub_msg;

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
void pubsubd_msg_print (const struct pubsub_msg *msg);

// parse pubsubd init msg (sent in TMPDIR/<service>)
//
// line fmt : pid index version action chan
// action : quit | pub | sub
int pubsubd_get_new_process (struct service *srv, struct app_list_elm *ale
        , struct channels *chans, struct channel **c);
int pubsubd_msg_read_cb (FILE *f, char ** buf, size_t * msize);
void pubsubd_msg_send (const struct app_list_head *alh, const struct pubsub_msg *m);
void pubsubd_msg_recv (struct process *p, struct pubsub_msg *m);

void pubsub_msg_send (struct process *p, const struct pubsub_msg *msg);
void pubsub_msg_recv (struct process *p, struct pubsub_msg *msg);

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
struct channel * pubsubd_channel_copy (struct channel *c);
struct channel * pubsubd_channel_get (struct channels *chans, struct channel *c);
void pubsubd_channel_free (struct channel *c);
int pubsubd_channel_eq (const struct channel *c1, const struct channel *c2);
void pubsubd_channel_print (const struct channel *c);

// list of channels
void pubsubd_channels_init (struct channels *chans);
void pubsubd_channels_print (const struct channels *chans);
struct channel * pubsubd_channels_add (struct channels *chans, struct channel *c);
void pubsubd_channels_del (struct channels *chans, struct channel *c);
void pubsubd_channels_del_all (struct channels *chans);

// APPLICATION

// head of the list
LIST_HEAD(app_list_head, app_list_elm);

enum app_list_elm_action {PUBSUB_QUIT = 1, PUBSUB_PUB, PUBSUB_SUB, PUBSUB_BOTH};

// element of the list
struct app_list_elm {
    struct process *p;
    enum app_list_elm_action action;
    LIST_ENTRY(app_list_elm) entries;
};

int
pubsubd_subscriber_eq (const struct app_list_elm *, const struct app_list_elm *);

void pubsubd_subscriber_init (struct app_list_head **chans);
void pubsubd_subscriber_add (struct app_list_head *
        , const struct app_list_elm *);
struct app_list_elm * pubsubd_subscriber_get (const struct app_list_head *
        , const struct app_list_elm *);
void pubsubd_subscriber_del (struct app_list_head *al, struct app_list_elm *p);
void pubsubd_subscriber_del_all (struct app_list_head *alh);

struct app_list_elm * pubsubd_app_list_elm_copy (const struct app_list_elm *ale);
void pubsubd_app_list_elm_create (struct app_list_elm *ale, struct process *p);
void pubsubd_app_list_elm_free (struct app_list_elm *todel);

void pubsub_connection (struct service *srv, struct process *p, enum app_list_elm_action action, const char *channame);
void pubsub_disconnect (struct service *srv);

#endif
