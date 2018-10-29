#ifndef __IPC_USOCKET_H__
#define __IPC_USOCKET_H__

#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

#define LISTEN_BACKLOG 128

#define RUNDIR "/run/ipc/"
#define PATH_MAX 4096

#define IPC_HEADER_SIZE  5
#define IPC_MAX_MESSAGE_SIZE  8000000-IPC_HEADER_SIZE

struct ipc_service {
    unsigned int version;
    unsigned int index;
    char spath[PATH_MAX];
    int service_fd;
};

struct ipc_services {
	struct ipc_service ** services;
	int size;
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


// store and remove only pointers on allocated structures
int ipc_service_add (struct ipc_services *, struct ipc_service *);
int ipc_service_del (struct ipc_services *, struct ipc_service *);

void ipc_services_print (struct ipc_services *);
void ipc_services_free  (struct ipc_services *);

struct ipc_service * ipc_client_server_copy (const struct ipc_service *p);
int ipc_service_eq (const struct ipc_service *p1, const struct ipc_service *p2);
// create the client service structure
void ipc_client_server_gen (struct ipc_service *p
        , unsigned int index, unsigned int version);

void service_print (struct ipc_service *);

#endif
