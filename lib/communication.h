#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "process.h"
#include <unistd.h> // unlink

#include <sys/types.h> // mkfifo
#include <sys/stat.h> // mkfifo
#include <fcntl.h> // open

#include <errno.h> // error numbers

#define COMMUNICATION_VERSION 1

struct service {
    unsigned int version;
    unsigned int index;
    char spath[PATH_MAX];
    FILE *spipe;
};

void srv_init (struct service *srv, const char *sname);

int srv_get_listen_raw (const struct service *srv, char **buf, size_t *msize);
int srv_get_new_process (const struct service *srv, struct process *proc);

/*
 * returns
 *  0 : ok
 *  1 : no service name
 *  2 : service name too long
 *  3 : unable to create fifo
 */
int srv_create (struct service *srv);
int srv_close (struct service *srv);

int srv_read_cb (struct process *p, char ** buf, size_t * msize
        , int (*cb)(FILE *f, char ** buf, size_t * msize));
int srv_read (struct process *, char ** buf, size_t *);
int srv_write (struct process *, void * buf, size_t);

// APPLICATION

int app_create (struct process *, int index); // called by the application
int app_destroy (struct process *); // called by the application

int app_read (struct process *, void * buf, size_t *);
int app_write (struct process *, void * buf, size_t);

#endif
