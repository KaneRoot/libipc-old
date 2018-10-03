#ifndef __IPC_MSG_H__
#define __IPC_MSG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSG_TYPE_CLOSE              0
#define MSG_TYPE_CON                1
#define MSG_TYPE_ERR                2
#define MSG_TYPE_ACK                3
#define MSG_TYPE_DATA               4

struct ipc_message {
    char type;
    unsigned short length;
    char *payload;
};

// used to create msg structure from buffer
int ipc_message_format_read (struct ipc_message *m, const char *buf, size_t msize);
// used to create buffer from msg structure
int ipc_message_format_write (const struct ipc_message *m, char **buf, size_t *msize);

// read a structure msg from fd
int ipc_message_read (int fd, struct ipc_message *m);
// write a structure msg to fd
int ipc_message_write (int fd, const struct ipc_message *m);

int ipc_message_format_con (struct ipc_message *m, const char *payload, size_t length);
int ipc_message_format_data (struct ipc_message *m, const char *payload, size_t length);
int ipc_message_format_ack (struct ipc_message *m, const char *payload, size_t length);

int ipc_message_free (struct ipc_message *m);
void ipc_message_print (const struct ipc_message *m);

#endif
