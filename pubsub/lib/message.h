#ifndef __PUBSUB_MSG_H__
#define __PUBSUB_MSG_H__

#include "../../core/ipc.h"

#define PUBSUB_SUBSCRIBER_ACTION_STR_PUB    "pub"
#define PUBSUB_SUBSCRIBER_ACTION_STR_SUB    "sub"

enum subscriber_action {PUBSUB_PUB, PUBSUB_SUB};

#define PUBSUB_TYPE_MESSAGE                                         1
#define PUBSUB_TYPE_ERROR                                           2
#define PUBSUB_TYPE_DEBUG                                           4
#define PUBSUB_TYPE_INFO                                            5

enum pubsub_message_types {
	PUBSUB_MSG_TYPE_SUB
		, PUBSUB_MSG_TYPE_UNSUB
		, PUBSUB_MSG_TYPE_PUB
};

struct pubsub_msg {
    unsigned char type; // message type : alert, notification, …
    char *chan;
    size_t chanlen;
    char *data;
    size_t datalen;
};

void pubsub_message_from_message (struct pubsub_msg *msg, struct ipc_message *m);
void pubsub_message_to_message (const struct pubsub_msg *msg, struct ipc_message *m);

void pubsub_message_set_chan (struct pubsub_msg *pm, char *chan, size_t len);
void pubsub_message_set_data (struct pubsub_msg *pm, char *data, size_t len);

void pubsub_message_serialize (const struct pubsub_msg *msg, char **data, size_t *len);
void pubsub_message_unserialize (struct pubsub_msg *msg, const char *data, size_t len);
void pubsub_message_empty (struct pubsub_msg *msg);
void pubsub_message_print (const struct pubsub_msg *msg);

int pubsub_message_send (struct ipc_service *srv, const struct pubsub_msg * m);

#endif
