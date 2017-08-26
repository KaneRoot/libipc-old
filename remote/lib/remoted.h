#ifndef __REMOTED_H__
#define __REMOTED_H__

#include "../../core/process.h"
#include "../../core/msg.h"

#define REMOTED_SERVICE_NAME "remoted"

struct remoted_ctx {
    /* TODO */
};

void remoted_main_loop (struct service *srv);

#endif
