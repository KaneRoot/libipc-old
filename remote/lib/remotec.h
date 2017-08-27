#ifndef __REMOTEC_H__
#define __REMOTEC_H__

#include "../../core/process.h"
#include "../../core/msg.h"
#include "remoted.h"

/* TODO */

int remotec_connection (int argc, char **argv, char **env, struct service *srv);
int remotec_disconnection (struct service *srv);

int remotec_msg_send (struct service *srv, const struct remoted_msg *msg);
int remotec_msg_recv (struct service *srv, struct remoted_msg *msg);

#endif
