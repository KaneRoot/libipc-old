#ifndef __IPC_PROCESS_H__
#define __IPC_PROCESS_H__

struct ipc_client {
    unsigned int version;
    unsigned int index;
    int proc_fd;
};

struct ipc_client_array {
	struct ipc_client **tab_proc;
	int size;
};

int ipc_client_add (struct ipc_client_array *, struct ipc_client *);
int ipc_client_del (struct ipc_client_array *aproc, struct ipc_client *p);

void ipc_client_array_print (struct ipc_client_array *);
void ipc_client_array_free (struct ipc_client_array *);

struct ipc_client * ipc_server_client_copy (const struct ipc_client *p);
int ipc_server_client_eq (const struct ipc_client *p1, const struct ipc_client *p2);
// create the service client structure
void ipc_server_client_gen (struct ipc_client *p
        , unsigned int index, unsigned int version);

void client_print (struct ipc_client *);
#endif
