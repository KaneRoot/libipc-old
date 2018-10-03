#ifndef __IPC_USOCKET_H__
#define __IPC_USOCKET_H__

#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

#define LISTEN_BACKLOG 128

// same as recv(2)
int usock_send (int fd, const char *buf, const int m_size);

// same as send(2)
// if msize == NULL => -1
// if buf == NULL => -1
// if *buf == NULL => allocation of *msize bytes
int usock_recv (int fd, char **buf, size_t *msize);

// same as close(2)
int usock_close (int fd);

// same as connect(2)
// if fd == NULL => -1
int usock_connect (int *fd, const char *path);

// if not ok => -1
// if ok => 0
int usock_init (int *fd, const char *path);

// if not ok => -1
// if ok => 0
int usock_accept (int fd, int *pfd);

// same as unlink(2)
int usock_remove (const char *path);

#endif
