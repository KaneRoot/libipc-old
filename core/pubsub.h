#ifndef __PUBSUB_H__
#define __PUBSUB_H__

#include "communication.h"
#include "process.h"
#include "queue.h"

#if 0

#define PUBSUB_TYPE_DISCONNECT                                      0
#define PUBSUB_TYPE_MESSAGE                                         1
#define PUBSUB_TYPE_ERROR                                           2
#define PUBSUB_TYPE_DEBUG                                           4
#define PUBSUB_TYPE_INFO                                            128

#define PUBSUB_SERVICE_NAME "pubsub"

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

void pubsub_disconnect (struct process *p);
void pubsub_msg_send (struct process *p, const struct pubsub_msg *msg);
void pubsub_msg_recv (struct process *p, struct pubsub_msg *msg);

enum app_list_elm_action {PUBSUB_QUIT = 1, PUBSUB_PUB, PUBSUB_SUB, PUBSUB_BOTH};

void pubsub_connection (struct service *srv, struct process *p, enum app_list_elm_action action, const char *channame);
void pubsubd_quit (struct service *srv);

#endif

#endif
