#ifndef __PROCESS_H__
#define __PROCESS_H__

struct process {
    unsigned int version;
    unsigned int index;
    int proc_fd;
};

// TODO
// tout revoir

#if 0

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>


struct process * srv_process_copy (const struct process *p);

int srv_process_eq (const struct process *p1, const struct process *p2);

// create the service process structure
void srv_process_gen (struct process *p
        , unsigned int index, unsigned int version);

void srv_process_print (struct process *);

#endif
#endif
