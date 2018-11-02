#ifndef __IPC_USOCKET_H__
#define __IPC_USOCKET_H__

#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stdint.h>

#define LISTEN_BACKLOG 128

#define RUNDIR "/run/ipc/"
#define PATH_MAX 4096

#define IPC_HEADER_SIZE  5
#define IPC_MAX_MESSAGE_SIZE  8000000-IPC_HEADER_SIZE

struct ipc_service {
    uint32_t version;
    uint32_t index;
    char spath[PATH_MAX];
    int32_t service_fd;
};

struct ipc_services {
	struct ipc_service ** services;
	int32_t size;
};

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

static inline int32_t ipc_service_empty (struct ipc_service *srv) { srv = srv; return 0 ;};


// store and remove only pointers on allocated structures
int32_t ipc_service_add (struct ipc_services *, struct ipc_service *);
int32_t ipc_service_del (struct ipc_services *, struct ipc_service *);

void ipc_services_print (struct ipc_services *);
void ipc_services_free  (struct ipc_services *);

struct ipc_service * ipc_client_server_copy (const struct ipc_service *p);
int32_t ipc_service_eq (const struct ipc_service *p1, const struct ipc_service *p2);
// create the client service structure
void ipc_client_server_gen (struct ipc_service *p
        , uint32_t index, uint32_t version);

void service_print (struct ipc_service *);

#endif
