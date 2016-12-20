#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // error numbers

#include "process.h"

#define COMMUNICATION_VERSION 1

#define ER_FILE_OPEN                                1
#define ER_FILE_CLOSE                               2
#define ER_FILE_READ                                3
#define ER_FILE_WRITE                               4
#define ER_FILE_WRITE_PARAMS                        5

#define ER_MEM_ALLOC                                100
#define ER_PARAMS                                   101

#define TMPDIR "/tmp/ipc/"

#define PATH_MAX BUFSIZ

#define CONNECTION 0
#define APPLICATION 1

struct service {
    unsigned int version;
    unsigned int index;
    char spath[PATH_MAX];
    int service_fd;
};


// SERVICE

// srv->version and srv->index must be already set
// init unix socket + fill srv->spath
int srv_init (int argc, char **argv, char **env
        , struct service *srv, const char *sname);
int srv_close (struct service *srv);
int srv_close_proc (struct process *p);
int srv_accept (struct service *srv, struct process *p);

int srv_read (const struct process *, char **buf, size_t *msize);
int srv_write (const struct process *, const char * buf, size_t);

int srv_select(struct array_proc *, struct service *, struct process **);

int getMaxFd(struct array_proc *);

// APPLICATION

// Initialize connection with unix socket
// send the connection string to $TMP/<service>
// fill srv->spath && srv->service_fd
int app_connection (int argc, char **argv, char **env
        , struct service *, const char *, const char *, size_t);
int app_close (struct service *);

int app_read (struct service *srv, char ** buf, size_t *msize);
int app_write (struct service *, char * buf, size_t msize);



#endif
