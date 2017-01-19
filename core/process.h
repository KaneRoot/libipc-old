#ifndef __PROCESS_H__
#define __PROCESS_H__

struct process {
    unsigned int version;
    unsigned int index;
    int proc_fd;
};

struct array_proc {
	struct process **tab_proc;
	int size;
};

int add_proc (struct array_proc *, struct process *);
int del_proc (struct array_proc *aproc, struct process *p);

void array_proc_print (struct array_proc *);
void array_proc_free (struct array_proc *);

struct process * srv_process_copy (const struct process *p);
int srv_process_eq (const struct process *p1, const struct process *p2);
// create the service process structure
void srv_process_gen (struct process *p
        , unsigned int index, unsigned int version);

void process_print (struct process *);
#endif
