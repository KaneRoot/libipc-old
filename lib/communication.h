#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <cbor.h>

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
    int service_fd;
};

int srv_init (int argc, char **argv, char **env
        , struct service *srv, const char *sname
        , int (*cb)(int argc, char **argv, char **env
            , struct service *srv, const char *sname));

int srv_get_new_process (char *buf, struct process *proc);

/*
 * returns
 *  0 : ok
 *  1 : no service name
 *  2 : service name too long
 *  3 : unable to create fifo
 */
int srv_create (struct service *srv);
int srv_close (struct service *srv);

int srv_read (const struct service *, char ** buf);
int srv_write (const struct service *, const char * buf, size_t);

// APPLICATION

// send the connection string to $TMP/<service>
int app_srv_connection (struct service *, const char *, size_t);

int app_create (struct process *, pid_t pid, int index, int version);
int app_destroy (struct process *);

int app_read (struct process *, char ** buf);
int app_write (struct process *, char * buf, size_t);

// wrappers
int file_read (int fd, char **buf);
int file_write (int fd, const char *buf, const int m_size);

//close socket
int close_socket(int fd);

//set and return a listen socket
int set_listen_socket(const char *path);

//init a proc connection
int proc_connection(struct process *p);

//open, close, read, write

#endif
