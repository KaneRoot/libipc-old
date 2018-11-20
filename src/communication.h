#ifndef __IPC_COMMUNICATION_H__
#define __IPC_COMMUNICATION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // error numbers


#include "ipc.h"
#include "client.h"
#include "event.h"
#include "message.h"

#define IPC_WITH_UNIX_SOCKETS
#ifdef  IPC_WITH_UNIX_SOCKETS
#include "usocket.h"
#endif

#endif
