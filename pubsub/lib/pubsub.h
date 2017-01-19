#ifndef __PUBSUB_H__
#define __PUBSUB_H__

#include "../../core/communication.h"
#include "../../core/process.h"
#include "../../core/queue.h"

#include "msg.h"

enum subscriber_action {PUBSUB_QUIT = 1, PUBSUB_PUB, PUBSUB_SUB, PUBSUB_BOTH};

#define PUBSUB_TYPE_DISCONNECT                                      0
#define PUBSUB_TYPE_MESSAGE                                         1
#define PUBSUB_TYPE_ERROR                                           2
#define PUBSUB_TYPE_DEBUG                                           4
#define PUBSUB_TYPE_INFO                                            5

int pubsub_connection (int argc, char **argv, char **env, struct service *srv);
int pubsub_disconnect (struct service *srv);
int pubsub_msg_send (struct service *srv, const struct pubsub_msg * m);
int pubsub_msg_recv (struct service *srv, struct pubsub_msg *m);

// TODO
void pubsub_quit (struct service *srv);

#endif
