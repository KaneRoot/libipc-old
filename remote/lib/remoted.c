#include "../../core/communication.h"
#include "../../core/msg.h"
#include "../../core/process.h"
#include "../../core/utils.h"
#include "../../core/error.h"

#include "remoted.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

void remoted_main_loop (struct service *srv)
{
    (void) srv;
    /* TODO */
}
