#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // error numbers
#include "msg.h"

#include "process.h"

#define COMMUNICATION_VERSION 1

#define ER_MEM_ALLOC                                100
#define ER_PARAMS                                   101

#define TMPDIR "/run/ipc/"

#define PATH_MAX BUFSIZ

#define CONNECTION  0
#define APPLICATION 1
#define CON_APP     2

struct service {
    unsigned int version;
    unsigned int index;
    char spath[PATH_MAX];
    int service_fd;
};


// SERVICE

// srv->version and srv->index must be already set
// init unix socket + fill srv->spath
int ipc_server_init (int argc, char **argv, char **env
		, struct service *srv, const char *sname);
int ipc_server_close (struct service *srv);
int ipc_server_close_proc (struct process *p);
int ipc_server_accept (struct service *srv, struct process *p);

int ipc_server_read (const struct process *, struct msg *m);
int ipc_server_write (const struct process *, const struct msg *m);

int ipc_server_select (struct array_proc *, struct service *, struct array_proc *);

// APPLICATION

// Initialize connection with unix socket
// send the connection string to $TMP/<service>
// fill srv->spath && srv->service_fd
int ipc_application_connection (int argc, char **argv, char **env
		, struct service *, const char *, const char *, size_t);
int ipc_application_close (struct service *);

int ipc_application_read (struct service *srv, struct msg *m);
int ipc_application_write (struct service *, const struct msg *m);



#endif
