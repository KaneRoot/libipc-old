#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define TMPDIR "/tmp/ipc/"

// TODO to check the right length for a path
#define PATH_MAX BUFSIZ

#include <string.h>

struct process {
    pid_t pid;
    unsigned int version;
    unsigned int index;
    char path_in [PATH_MAX];
    char path_out [PATH_MAX];
    FILE *in, *out;
};

struct process * srv_process_copy (const struct process *p);

int srv_process_eq (const struct process *p1, const struct process *p2);

// create the service process structure
void srv_process_gen (struct process *p
        , pid_t pid, unsigned int index, unsigned int version);

void process_print (struct process *);

#endif
