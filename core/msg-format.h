#ifndef __MSG_FORMAT_H__
#define __MSG_FORMAT_H__

#include <assert.h>

#define MSG_TYPE_CON 1
#define MSG_TYPE_ERR 2
#define MSG_TYPE_ACK 4

int msg_format_con (char *buf, const char *constr, size_t *msgsize);
int msg_format_ack (char *buf, const char *constr, size_t *msgsize);

#endif
