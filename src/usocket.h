#ifndef __IPC_USOCKET_H__
#define __IPC_USOCKET_H__

#include "ipc.h"

#define LISTEN_BACKLOG 128

// input:  len   = max buf size
// output: *sent = nb received bytes
enum ipc_errors usock_send (const int32_t fd, const char *buf, size_t len, size_t *sent);

// allocation of *len bytes on *buf == NULL
//
// output: *len = nb sent bytes
enum ipc_errors usock_recv (int32_t fd, char **buf, size_t *len);

enum ipc_errors usock_close (int32_t fd);

// same as connect(2)
enum ipc_errors usock_connect (int32_t *fd, const char *path);

enum ipc_errors usock_init (int32_t *fd, const char *path);

enum ipc_errors usock_accept (int32_t fd, int32_t *pfd);

// same as unlink(2)
enum ipc_errors usock_remove (const char *path);

#endif
