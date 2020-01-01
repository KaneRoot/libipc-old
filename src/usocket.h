#ifndef __IPC_USOCKET_H__
#define __IPC_USOCKET_H__

#include "ipc.h"

#define LISTEN_BACKLOG 128

// input:  len   = max buf size
// output: *sent = nb received bytes
struct ipc_error usock_send (const int32_t fd, const char *buf, size_t len, size_t * sent);

// allocation of *len bytes on *buf == NULL
//
// output: *len = nb sent bytes
struct ipc_error usock_recv (int32_t fd, char **buf, size_t * len);

struct ipc_error usock_close (int32_t fd);

// same as connect(2)
struct ipc_error usock_connect (int32_t * fd, const char *path);

struct ipc_error usock_init (int32_t * fd, const char *path);

struct ipc_error usock_accept (int32_t fd, int32_t * pfd);

// same as unlink(2)
struct ipc_error usock_remove (const char *path);

#endif
