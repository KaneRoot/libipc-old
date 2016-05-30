#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h> // unlink

#include <sys/types.h> // mkfifo
#include <sys/stat.h> // mkfifo
#include <fcntl.h> // open

#include <errno.h> // error numbers

#define TMPDIR "/tmp/ipc/"

#define COMMUNICATION_VERSION 1

// TODO to check the right length for a path
#define PATH_MAX BUFSIZ

struct process {
    pid_t pid;
    unsigned int version;
    unsigned int index;
    FILE *in, *out;
};

struct service {
    unsigned int version;
    unsigned int index;
};

// TODO create the service process structure

int service_path (char *buf, const char *sname);

void gen_process_structure (struct process *p
        , pid_t pid, unsigned int index, unsigned int version);

/*
 * returns
 *  0 : ok
 *  1 : no service name
 *  2 : service name too long
 *  3 : unable to create fifo
 */
int service_create (const char *sname);
int service_close (const char *sname);

int service_get_new_process (struct process *proc, const char * spath);
void service_get_new_processes (struct process ***, int *nproc, char *spath);
void service_free_processes (struct process **, int nproc);

void process_print (struct process *);
int process_create (struct process *, int index); // called by the application
int process_destroy (struct process *); // called by the application

int process_read (struct process *, void * buf, size_t *);
int process_write (struct process *, void * buf, size_t);

int service_read (struct process *, void * buf, size_t *);
int service_write (struct process *, void * buf, size_t);

#endif
