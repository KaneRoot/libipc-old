#ifndef __PUBSUB_MSG_H__
#define __PUBSUB_MSG_H__

#define PUBSUB_MSG_TYPE_SUB         1
#define PUBSUB_MSG_TYPE_UNSUB       2
#define PUBSUB_MSG_TYPE_PUB         3

struct pubsub_msg {
    unsigned char type; // message type : alert, notification, …
    char *chan;
    size_t chanlen;
    char *data;
    size_t datalen;
};

void pubsub_msg_serialize (const struct pubsub_msg *msg, char **data, size_t *len);
void pubsub_msg_unserialize (struct pubsub_msg *msg, const char *data, size_t len);
void pubsub_msg_free (struct pubsub_msg *msg);
void pubsub_msg_print (const struct pubsub_msg *msg);

#endif
