#ifndef __IPC_USOCKET_H__
#define __IPC_USOCKET_H__

#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "ipc.h"
#include <stdint.h>

#define LISTEN_BACKLOG 128

/**
 * for all functions: 0 ok, < 0 not ok
 */

// input:  len   = max buf size
// output: *sent = nb received bytes
int32_t usock_send (const int32_t fd, const char *buf, ssize_t len, ssize_t *sent);

// -1 on msize == NULL or buf == NULL
// -1 on unsupported errors from read(2)
// exit on most errors from read(2)
//
// allocation of *len bytes on *buf == NULL
//
// output: *len = nb sent bytes
int32_t usock_recv (int32_t fd, char **buf, ssize_t *len);

// -1 on close(2) < 0
int32_t usock_close (int32_t fd);

// same as connect(2)
// -1 on fd == NULL
int32_t usock_connect (int32_t *fd, const char *path);

int32_t usock_init (int32_t *fd, const char *path);

int32_t usock_accept (int32_t fd, int32_t *pfd);

// same as unlink(2)
int32_t usock_remove (const char *path);

void ipc_services_print (struct ipc_services *);
void service_print (struct ipc_service *);

#endif
