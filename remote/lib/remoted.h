#ifndef __REMOTED_H__
#define __REMOTED_H__

#include "../../core/process.h"
#include "../../core/msg.h"
#include "msg.h"

#define REMOTED_SERVICE_NAME "remoted"

struct remoted_ctx {
    char * unix_socket_dir;
    /* TODO: authorizations */
};

void remoted_main_loop (struct service *srv, struct remoted_ctx *ctx);
void remoted_free_ctx (struct remoted_ctx *ctx);

#endif
