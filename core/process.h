#ifndef __IPC_PROCESS_H__
#define __IPC_PROCESS_H__

struct ipc_client {
    unsigned int version;
    unsigned int index;
    int proc_fd;
};

struct ipc_process_array {
	struct ipc_client **tab_proc;
	int size;
};

int ipc_process_add (struct ipc_process_array *, struct ipc_client *);
int ipc_process_del (struct ipc_process_array *aproc, struct ipc_client *p);

void ipc_process_array_print (struct ipc_process_array *);
void ipc_process_array_free (struct ipc_process_array *);

struct ipc_client * ipc_server_process_copy (const struct ipc_client *p);
int ipc_server_process_eq (const struct ipc_client *p1, const struct ipc_client *p2);
// create the service process structure
void ipc_server_process_gen (struct ipc_client *p
        , unsigned int index, unsigned int version);

void process_print (struct ipc_client *);
#endif
