#ifndef __REMOTEC_H__
#define __REMOTEC_H__

#include "../../core/client.h"
#include "../../core/msg.h"
#include "remoted.h"

/* TODO */

int remotec_connection (int argc, char **argv, char **env, struct ipc_service *srv);
int remotec_disconnection (struct ipc_service *srv);

int remotec_message_send (struct ipc_service *srv, const struct remoted_msg *msg);
int remotec_message_recv (struct ipc_service *srv, struct remoted_msg *msg);

#endif
