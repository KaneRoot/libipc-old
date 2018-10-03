#ifndef __PUBSUBD_H__
#define __PUBSUBD_H__

// #include "../../core/pubsub.h"
#include "../../core/process.h"
#include "../../core/msg.h"
#include "msg.h"
#include "channels.h"

#define PUBSUBD_SERVICE_NAME "pubsubd"

void pubsubd_main_loop (struct service *srv, struct channels * chans);
void pubsubd_message_send (const struct array_proc *ap, const struct pubsub_msg * m);

#endif
