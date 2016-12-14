#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <cbor.h>

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

// wrappers
int msg_recv (int fd, char **buf);
int msg_send (int fd, const char *buf, const int m_size);
int close_socket (int fd);


// SERVICE

int srv_init (int argc, char **argv, char **env
        , struct service *srv, const char *sname);
int srv_close (struct service *srv);

int srv_read (const struct service *, char ** buf);
int srv_write (const struct service *, const char * buf, size_t);

// APPLICATION

// send the connection string to $TMP/<service>
int app_connection (struct service *, const char *, const char *, size_t);
int app_close (struct service *);

int app_read (struct service *, char ** buf);
int app_write (struct service *, char * buf, size_t);

#endif
