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

#define ER_FILE_OPEN                                1
#define ER_FILE_CLOSE                               2
#define ER_FILE_READ                                3
#define ER_FILE_WRITE                               4
#define ER_FILE_WRITE_PARAMS                        5

#define ER_MEM_ALLOC        100
#define ER_PARAMS           101

struct service {
    unsigned int version;
    unsigned int index;
    char spath[PATH_MAX];
    FILE *spipe;
};

int srv_init (int argc, char **argv, char **env
        , struct service *srv, const char *sname
        , int (*cb)(int argc, char **argv, char **env
            , struct service *srv, const char *sname));

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

int srv_read (struct process *, char ** buf, size_t *);
int srv_write (struct process *, char * buf, size_t);

// APPLICATION

// send the connection string to $TMP/<service>
int app_srv_connection (struct service *, const char *, size_t);

int app_create (struct process *, pid_t pid, int index, int version);
int app_destroy (struct process *);

int app_read (struct process *, char ** buf, size_t *);
int app_write (struct process *, char * buf, size_t);

// wrappers
int file_read (const char *path, char **buf, size_t *msize);
int file_write (const char *path, const char *buf, size_t msize);

#endif
