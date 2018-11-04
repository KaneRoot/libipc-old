#ifndef __IPC_H__
#define __IPC_H__

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // error numbers
#include <time.h>

#define RUNDIR "/run/ipc/"
#define PATH_MAX 4096
#define IPC_HEADER_SIZE  5
#define IPC_MAX_MESSAGE_SIZE  8000000-IPC_HEADER_SIZE
// #include "queue.h"

#define IPC_VERSION 1


enum msg_types {
	MSG_TYPE_SERVER_CLOSE = 0
	, MSG_TYPE_ERR
	, MSG_TYPE_DATA
} message_types;

enum ipc_event_type {
	IPC_EVENT_TYPE_NOT_SET
	, IPC_EVENT_TYPE_ERROR
	, IPC_EVENT_TYPE_STDIN
	, IPC_EVENT_TYPE_CONNECTION
	, IPC_EVENT_TYPE_DISCONNECTION
	, IPC_EVENT_TYPE_MESSAGE
};

enum ipc_errors {
	IPC_ERROR_NOT_ENOUGH_MEMORY
	, IPC_ERROR_WRONG_PARAMETERS
	, IPC_ERROR_READ
};

struct ipc_service {
    uint32_t version;
    uint32_t index;
    char spath[PATH_MAX];
    int32_t service_fd;
};

struct ipc_services {
	struct ipc_service ** services;
	int32_t size;
};

struct ipc_client {
    uint32_t version;
    uint32_t index;
    int32_t proc_fd;
};

struct ipc_clients {
	struct ipc_client **clients;
	int32_t size;
};

struct ipc_message {
    char type;
    uint32_t length;
    char *payload;
};

struct ipc_event {
	enum ipc_event_type type;
	void* origin; // currently used as an client or service pointer
	void* m; // message pointer
};



/*
 * SERVICE
 *
 **/

// srv->version and srv->index must be already set
// init unix socket + fill srv->spath
int32_t ipc_server_init (char **env , struct ipc_service *srv, const char *sname);
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

/**
 * SERVICES
 */

// store and remove only pointers on allocated structures
int32_t ipc_services_add (struct ipc_services *, struct ipc_service *);
int32_t ipc_services_del (struct ipc_services *, struct ipc_service *);

void ipc_services_free  (struct ipc_services *);

struct ipc_service * ipc_client_server_copy (const struct ipc_service *p);
int32_t ipc_service_eq (const struct ipc_service *p1, const struct ipc_service *p2);
// create the client service structure
void ipc_client_server_gen (struct ipc_service *p, uint32_t index, uint32_t version);

static inline int32_t ipc_service_empty (struct ipc_service *srv) { srv = srv; return 0 ;};


/*
 * APPLICATION
 *
 **/

// Initialize connection with unix socket
// send the connection string to $TMP/<service>
// fill srv->spath && srv->service_fd
int32_t ipc_application_connection (char **env, struct ipc_service *, const char *);
int32_t ipc_application_close (struct ipc_service *);

// 1 on a recipient socket close
int32_t ipc_application_read (struct ipc_service *srv, struct ipc_message *m);
int32_t ipc_application_write (struct ipc_service *, const struct ipc_message *m);

int32_t ipc_application_select (struct ipc_services *services, struct ipc_services *active_services);
int32_t ipc_application_poll_event (struct ipc_services *services, struct ipc_event *event);
int32_t ipc_application_peek_event (struct ipc_services *services, struct ipc_event *event);



/*
 * MESSAGE
 *
 **/

// used to create msg structure from buffer
int32_t ipc_message_format_read (struct ipc_message *m, const char *buf, ssize_t msize);
// used to create buffer from msg structure
int32_t ipc_message_format_write (const struct ipc_message *m, char **buf, ssize_t *msize);

// read a structure msg from fd
int32_t ipc_message_read (int32_t fd, struct ipc_message *m);
// write a structure msg to fd
int32_t ipc_message_write (int32_t fd, const struct ipc_message *m);

int32_t ipc_message_format (struct ipc_message *m, char type, const char *payload, ssize_t length);
int32_t ipc_message_format_data (struct ipc_message *m, const char *payload, ssize_t length);
int32_t ipc_message_format_server_close (struct ipc_message *m);

int32_t ipc_message_empty (struct ipc_message *m);



/*
 * CLIENT
 *
 **/

// store and remove only pointers on allocated structures
int32_t ipc_clients_add (struct ipc_clients *, struct ipc_client *);
int32_t ipc_clients_del (struct ipc_clients *, struct ipc_client *);

void ipc_clients_free  (struct ipc_clients *);

struct ipc_client * ipc_server_client_copy (const struct ipc_client *p);
int32_t ipc_server_client_eq (const struct ipc_client *p1, const struct ipc_client *p2);
// create the service client structure
void ipc_server_client_gen (struct ipc_client *p, uint32_t index, uint32_t version);


#endif
