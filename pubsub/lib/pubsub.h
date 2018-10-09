#ifndef __PUBSUB_H__
#define __PUBSUB_H__

#include "../../core/communication.h"
#include "../../core/client.h"
#include "../../core/queue.h"

#include "message.h"

enum subscriber_action {PUBSUB_QUIT = 1, PUBSUB_PUB, PUBSUB_SUB, PUBSUB_BOTH};

#define PUBSUB_TYPE_DISCONNECT                                      0
#define PUBSUB_TYPE_MESSAGE                                         1
#define PUBSUB_TYPE_ERROR                                           2
#define PUBSUB_TYPE_DEBUG                                           4
#define PUBSUB_TYPE_INFO                                            5

int pubsub_connection (int argc, char **argv, char **env, struct ipc_service *srv);
int pubsub_disconnect (struct ipc_service *srv);
int pubsub_message_send (struct ipc_service *srv, const struct pubsub_msg * m);
int pubsub_message_recv (struct ipc_service *srv, struct pubsub_msg *m);

// TODO
void pubsub_quit (struct ipc_service *srv);

#endif
