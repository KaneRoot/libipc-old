#ifndef __IPC_MSG_H__
#define __IPC_MSG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// the underlying communication must always correctly handled by the system
// (currently: unix sockets)

enum msg_types {
	MSG_TYPE_SERVER_CLOSE = 0
	, MSG_TYPE_ERR
	, MSG_TYPE_DATA
} message_types;

struct ipc_message {
    char type;
    unsigned short length;
    char *payload;
};

// used to create msg structure from buffer
int ipc_message_format_read (struct ipc_message *m, const char *buf, ssize_t msize);
// used to create buffer from msg structure
int ipc_message_format_write (const struct ipc_message *m, char **buf, ssize_t *msize);

// read a structure msg from fd
// 1 on a recipient socket close
int ipc_message_read (int fd, struct ipc_message *m);
// write a structure msg to fd
int ipc_message_write (int fd, const struct ipc_message *m);

int ipc_message_format_data (struct ipc_message *m, const char *payload, ssize_t length);
int ipc_message_format_server_close (struct ipc_message *m);

int ipc_message_empty (struct ipc_message *m);
void ipc_message_print (const struct ipc_message *m);

#endif