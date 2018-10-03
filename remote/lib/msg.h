#ifndef __REMOTE_MSG_H__
#define __REMOTE_MSG_H__

#define REMOTE_MSG_TYPE_CONNECT     1
#define REMOTE_MSG_TYPE_LISTEN      2
#define REMOTE_MSG_TYPE_PUB         3

struct remoted_msg {
    unsigned char type; // message types = commands (connect, listen, ...)
    char *data;
    size_t datalen;
};

void remote_message_serialize (const struct remoted_msg *msg, char **data, size_t *len);
void remote_message_unserialize (struct remoted_msg *msg, const char *data, size_t len);

void remote_message_free (struct remoted_msg *msg);
void remote_message_print (const struct remoted_msg *msg);

#endif
