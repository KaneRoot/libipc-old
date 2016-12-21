#ifndef __MSG_H__
#define __MSG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSG_TYPE_CON    1
#define MSG_TYPE_DIS    2
#define MSG_TYPE_ERR    3
#define MSG_TYPE_ACK    4
#define MSG_TYPE_DATA   5

struct msg {
    char type;
    short valsize;
    char *val;
};

// used to create msg structure from buffer
int msg_format_read (struct msg *m, const char *buf, size_t msize);
// used to create buffer from msg structure
int msg_format_write (struct msg *m, char **buf, size_t *msize);

// read a structure msg from fd
int msg_read (int fd, struct msg *m);
// write a structure msg to fd
int msg_write (int fd, const struct msg *m);

int msg_format_con (struct msg *m, const char *val, size_t valsize);
int msg_format_data (struct msg *m, const char *val, size_t valsize);
int msg_format_ack (struct msg *m, const char *val, size_t valsize);

int msg_free (struct msg *m);

#endif
