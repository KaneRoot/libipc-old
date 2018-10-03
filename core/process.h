#ifndef __IPC_PROCESS_H__
#define __IPC_PROCESS_H__

struct ipc_process {
    unsigned int version;
    unsigned int index;
    int proc_fd;
};

struct ipc_process_array {
	struct ipc_process **tab_proc;
	int size;
};

int ipc_process_add (struct ipc_process_array *, struct ipc_process *);
int ipc_process_del (struct ipc_process_array *aproc, struct ipc_process *p);

void ipc_process_array_print (struct ipc_process_array *);
void ipc_process_array_free (struct ipc_process_array *);

struct ipc_process * ipc_server_process_copy (const struct ipc_process *p);
int ipc_server_process_eq (const struct ipc_process *p1, const struct ipc_process *p2);
// create the service process structure
void ipc_server_process_gen (struct ipc_process *p
        , unsigned int index, unsigned int version);

void process_print (struct ipc_process *);
#endif
