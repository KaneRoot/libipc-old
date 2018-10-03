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

struct msg {
    char type;
    unsigned short valsize;
    char *val;
};

// used to create msg structure from buffer
int ipc_message_format_read (struct msg *m, const char *buf, size_t msize);
// used to create buffer from msg structure
int ipc_message_format_write (const struct msg *m, char **buf, size_t *msize);

// read a structure msg from fd
int ipc_message_read (int fd, struct msg *m);
// write a structure msg to fd
int ipc_message_write (int fd, const struct msg *m);

int ipc_message_format_con (struct msg *m, const char *val, size_t valsize);
int ipc_message_format_data (struct msg *m, const char *val, size_t valsize);
int ipc_message_format_ack (struct msg *m, const char *val, size_t valsize);

int ipc_message_free (struct msg *m);
void ipc_message_print (const struct msg *m);

#endif
