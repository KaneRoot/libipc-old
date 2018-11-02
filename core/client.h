#ifndef __IPC_CLIENT_H__
#define __IPC_CLIENT_H__

#include <stdint.h>

struct ipc_client {
    uint32_t version;
    uint32_t index;
    int32_t proc_fd;
};

struct ipc_clients {
	struct ipc_client **clients;
	int32_t size;
};

// store and remove only pointers on allocated structures
int32_t ipc_client_add (struct ipc_clients *, struct ipc_client *);
int32_t ipc_client_del (struct ipc_clients *, struct ipc_client *);

void ipc_clients_print (struct ipc_clients *);
void ipc_clients_free  (struct ipc_clients *);

struct ipc_client * ipc_server_client_copy (const struct ipc_client *p);
int32_t ipc_server_client_eq (const struct ipc_client *p1, const struct ipc_client *p2);
// create the service client structure
void ipc_server_client_gen (struct ipc_client *p
        , uint32_t index, uint32_t version);

void client_print (struct ipc_client *);
#endif
