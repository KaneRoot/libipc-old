#ifndef __IPC_COMMUNICATION_H__
#define __IPC_COMMUNICATION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // error numbers
#include "message.h"

#include "client.h"
#include "event.h"

#define COMMUNICATION_VERSION 1

#define IPC_WITH_UNIX_SOCKETS
#ifdef  IPC_WITH_UNIX_SOCKETS
#include "usocket.h"
#endif
// SERVICE

// srv->version and srv->index must be already set
// init unix socket + fill srv->spath
int32_t ipc_server_init (char **env
		, struct ipc_service *srv, const char *sname);
int32_t ipc_server_close (struct ipc_service *srv);
int32_t ipc_server_close_client (struct ipc_client *p);
int32_t ipc_server_accept (struct ipc_service *srv, struct ipc_client *p);

// 1 on a recipient socket close
int32_t ipc_server_read (const struct ipc_client *, struct ipc_message *m);
int32_t ipc_server_write (const struct ipc_client *, const struct ipc_message *m);

int32_t ipc_server_select (struct ipc_clients * clients, struct ipc_service *srv
        , struct ipc_clients *active_clients, int32_t *new_connection);

int32_t ipc_service_poll_event (struct ipc_clients *clients, struct ipc_service *srv
        , struct ipc_event *event);

// APPLICATION

// Initialize connection with unix socket
// send the connection string to $TMP/<service>
// fill srv->spath && srv->service_fd
int32_t ipc_application_connection (char **env
		, struct ipc_service *, const char *);
int32_t ipc_application_close (struct ipc_service *);

// 1 on a recipient socket close
int32_t ipc_application_read (struct ipc_service *srv, struct ipc_message *m);
int32_t ipc_application_write (struct ipc_service *, const struct ipc_message *m);

int32_t ipc_application_select (struct ipc_services *services, struct ipc_services *active_services);

int32_t ipc_application_poll_event (struct ipc_services *services, struct ipc_event *event);
int32_t ipc_application_peek_event (struct ipc_services *services, struct ipc_event *event);

#endif
