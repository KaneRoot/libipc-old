#ifndef __IPC_COMMUNICATION_H__
#define __IPC_COMMUNICATION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // error numbers
#include "msg.h"

#include "client.h"

#define COMMUNICATION_VERSION 1

#define ER_MEM_ALLOC                                100
#define ER_PARAMS                                   101

#define TMPDIR "/run/ipc/"

#define PATH_MAX 4096

#define IPC_MESSAGE_CONNECTION              0
#define IPC_MESSAGE_DATA                    1
#define IPC_MESSAGE_DATA_AND_CONNECTION     2
#define IPC_MESSAGE_SERVICE_STOP            3

struct ipc_service {
    unsigned int version;
    unsigned int index;
    char spath[PATH_MAX];
    int service_fd;
};


// SERVICE

// srv->version and srv->index must be already set
// init unix socket + fill srv->spath
int ipc_server_init (int argc, char **argv, char **env
		, struct ipc_service *srv, const char *sname);
int ipc_server_close (struct ipc_service *srv);
int ipc_server_close_client (struct ipc_client *p);
int ipc_server_accept (struct ipc_service *srv, struct ipc_client *p);

int ipc_server_read (const struct ipc_client *, struct ipc_message *m);
int ipc_server_write (const struct ipc_client *, const struct ipc_message *m);

int ipc_server_select (struct ipc_clients * clients, struct ipc_service *srv
        , struct ipc_clients *active_clients, int *new_connection);

// APPLICATION

// Initialize connection with unix socket
// send the connection string to $TMP/<service>
// fill srv->spath && srv->service_fd
int ipc_application_connection (int argc, char **argv, char **env
		, struct ipc_service *, const char *, const char *, size_t);
int ipc_application_close (struct ipc_service *);

int ipc_application_read (struct ipc_service *srv, struct ipc_message *m);
int ipc_application_write (struct ipc_service *, const struct ipc_message *m);



#endif
