#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h> // unlink

#include <sys/types.h> // mkfifo
#include <sys/stat.h> // mkfifo
#include <fcntl.h> // open

#define TMPDIR "/tmp/"

#define COMMUNICATION_VERSION 1

// TODO to check the right length for a path
#define PATH_MAX BUFSIZ

struct process {
    pid_t pid;
    unsigned int version;
    unsigned int index;
    int in, out;
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

void service_get_new_processes (struct process **, int *nproc, int sfifo);

int process_create (struct process *, int index); // called by the application
int process_destroy (struct process *); // called by the application

int process_open (struct process *); // called by the service & application
int process_close (struct process *); // called by the service & application

int process_read (struct process *, void * buf, size_t *);
int process_write (struct process *, void * buf, size_t);

#endif
