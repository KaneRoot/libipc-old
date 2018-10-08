#ifndef __IPC_USOCKET_H__
#define __IPC_USOCKET_H__

#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

#define LISTEN_BACKLOG 128

#define TMPDIR "/run/ipc/"
#define PATH_MAX 4096

struct ipc_service {
    unsigned int version;
    unsigned int index;
    char spath[PATH_MAX];
    int service_fd;
};

/**
 * for all functions: 0 ok, < 0 not ok
 */

// input:  len   = max buf size
// output: *sent = nb received bytes
int usock_send (const int fd, const char *buf, ssize_t len, ssize_t *sent);

// -1 on msize == NULL or buf == NULL
// -1 on unsupported errors from read(2)
// exit on most errors from read(2)
//
// allocation of *len bytes on *buf == NULL
//
// output: *len = nb sent bytes
int usock_recv (int fd, char **buf, ssize_t *len);

// -1 on close(2) < 0
int usock_close (int fd);

// same as connect(2)
// -1 on fd == NULL
int usock_connect (int *fd, const char *path);

int usock_init (int *fd, const char *path);

int usock_accept (int fd, int *pfd);

// same as unlink(2)
int usock_remove (const char *path);

static inline int ipc_service_empty (struct ipc_service *srv) { srv = srv; return 0 ;};

#endif
