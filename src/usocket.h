#ifndef __IPC_USOCKET_H__
#define __IPC_USOCKET_H__

#include "ipc.h"

#define LISTEN_BACKLOG 128

// input:  len   = max buf size
// output: *sent = nb received bytes
enum ipc_errors usock_send (const int32_t fd, const char *buf, ssize_t len, ssize_t *sent);

// -1 on msize == NULL or buf == NULL
// -1 on unsupported errors from read(2)
// exit on most errors from read(2)
//
// allocation of *len bytes on *buf == NULL
//
// output: *len = nb sent bytes
enum ipc_errors usock_recv (int32_t fd, char **buf, ssize_t *len);

// -1 on close(2) < 0
enum ipc_errors usock_close (int32_t fd);

// same as connect(2)
// -1 on fd == NULL
enum ipc_errors usock_connect (int32_t *fd, const char *path);

enum ipc_errors usock_init (int32_t *fd, const char *path);

enum ipc_errors usock_accept (int32_t fd, int32_t *pfd);

// same as unlink(2)
enum ipc_errors usock_remove (const char *path);

#endif
