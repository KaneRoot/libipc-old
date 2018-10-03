#ifndef __PUBSUBD_H__
#define __PUBSUBD_H__

// #include "../../core/pubsub.h"
#include "../../core/process.h"
#include "../../core/msg.h"
#include "msg.h"
#include "channels.h"

#define PUBSUBD_SERVICE_NAME "pubsubd"

void pubsubd_main_loop (struct ipc_service *srv, struct channels * chans);
void pubsubd_message_send (const struct ipc_process_array *ap, const struct pubsub_msg * m);

#endif
