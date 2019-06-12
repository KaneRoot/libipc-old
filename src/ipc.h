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
#define IPC_HEADER_SIZE  6
#define IPC_MAX_MESSAGE_SIZE  8000000-IPC_HEADER_SIZE
// #define IPC_MAX_MESSAGE_SIZE  100-IPC_HEADER_SIZE
// #include "queue.h"

#define SECURE_DECLARATION(t,v)   t v; memset(&v,0,sizeof(t));

#define IPC_VERSION 1


enum msg_types {
	MSG_TYPE_SERVER_CLOSE = 0
	, MSG_TYPE_ERR
	, MSG_TYPE_DATA
} message_types;

enum ipc_event_type {
	IPC_EVENT_TYPE_NOT_SET
	, IPC_EVENT_TYPE_ERROR

	, IPC_EVENT_TYPE_EXTRA_SOCKET

	, IPC_EVENT_TYPE_CONNECTION
	, IPC_EVENT_TYPE_DISCONNECTION
	, IPC_EVENT_TYPE_MESSAGE
};

enum ipc_errors {
	/* general errors */
	IPC_ERROR_NONE
	, IPC_ERROR_NOT_ENOUGH_MEMORY
	, IPC_ERROR_CLOSED_RECIPIENT

	, IPC_ERROR_SERVER_INIT__NO_ENVIRONMENT_PARAM
	, IPC_ERROR_SERVER_INIT__NO_SERVICE_PARAM
	, IPC_ERROR_SERVER_INIT__NO_SERVER_NAME_PARAM
	, IPC_ERROR_SERVER_INIT__MALLOC

	, IPC_ERROR_CONNECTION__NO_SERVER
	, IPC_ERROR_CONNECTION__NO_SERVICE_NAME
	, IPC_ERROR_CONNECTION__NO_ENVIRONMENT_PARAM

	, IPC_ERROR_CONNECTION_GEN__NO_CINFO

	, IPC_ERROR_ACCEPT__NO_SERVICE_PARAM
	, IPC_ERROR_ACCEPT__NO_CLIENT_PARAM
	, IPC_ERROR_ACCEPT

	, IPC_ERROR_HANDLE_NEW_CONNECTION__NO_CINFO_PARAM
	, IPC_ERROR_HANDLE_NEW_CONNECTION__NO_CINFOS_PARAM

	, IPC_ERROR_WAIT_EVENT__SELECT
	, IPC_ERROR_WAIT_EVENT__NO_CLIENTS_PARAM
	, IPC_ERROR_WAIT_EVENT__NO_EVENT_PARAM

	, IPC_ERROR_HANDLE_NEW_CONNECTION__MALLOC

	, IPC_ERROR_ADD__EMPTY_LIST
	, IPC_ERROR_ADD__NO_PARAM_CLIENTS
	, IPC_ERROR_ADD__NO_PARAM_CLIENT

	, IPC_ERROR_ADD_FD__NO_PARAM_CINFOS
	, IPC_ERROR_ADD_FD__EMPTY_LIST

	, IPC_ERROR_DEL_FD__NO_PARAM_CINFOS
	, IPC_ERROR_DEL_FD__EMPTIED_LIST
	, IPC_ERROR_DEL_FD__EMPTY_LIST
	, IPC_ERROR_DEL_FD__CANNOT_FIND_CLIENT

	, IPC_ERROR_DEL__EMPTY_LIST
	, IPC_ERROR_DEL__EMPTIED_LIST
	, IPC_ERROR_DEL__CANNOT_FIND_CLIENT
	, IPC_ERROR_DEL__NO_CLIENTS_PARAM
	, IPC_ERROR_DEL__NO_CLIENT_PARAM
	

	/* unix socket */

	, IPC_ERROR_USOCK_SEND

	, IPC_ERROR_USOCK_CONNECT__SOCKET
	, IPC_ERROR_USOCK_CONNECT__WRONG_FILE_DESCRIPTOR
	, IPC_ERROR_USOCK_CONNECT__EMPTY_PATH
	, IPC_ERROR_USOCK_CONNECT__CONNECT

	, IPC_ERROR_USOCK_CLOSE

	, IPC_ERROR_USOCK_REMOVE__UNLINK
	, IPC_ERROR_USOCK_REMOVE__NO_FILE

	, IPC_ERROR_USOCK_INIT__EMPTY_FILE_DESCRIPTOR
	, IPC_ERROR_USOCK_INIT__WRONG_FILE_DESCRIPTOR
	, IPC_ERROR_USOCK_INIT__EMPTY_PATH
	, IPC_ERROR_USOCK_INIT__BIND
	, IPC_ERROR_USOCK_INIT__LISTEN

	, IPC_ERROR_USOCK_ACCEPT__PATH_FILE_DESCRIPTOR
	, IPC_ERROR_USOCK_ACCEPT

	, IPC_ERROR_USOCK_RECV__NO_BUFFER
	, IPC_ERROR_USOCK_RECV__NO_LENGTH
	, IPC_ERROR_USOCK_RECV


	/* message function errors */

	, IPC_ERROR_MESSAGE_NEW__NO_MESSAGE_PARAM
	, IPC_ERROR_MESSAGE_READ__NO_MESSAGE_PARAM

	, IPC_ERROR_MESSAGE_WRITE__NO_MESSAGE_PARAM
	, IPC_ERROR_MESSAGE_WRITE__NOT_ENOUGH_DATA

	, IPC_ERROR_MESSAGE_FORMAT__NO_MESSAGE_PARAM
	, IPC_ERROR_MESSAGE_FORMAT__INCONSISTENT_PARAMS
	, IPC_ERROR_MESSAGE_FORMAT__LENGTH

	, IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_MESSAGE
	, IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_MSIZE
	, IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_BUFFER

	, IPC_ERROR_MESSAGE_FORMAT_READ__EMPTY_MESSAGE
	, IPC_ERROR_MESSAGE_FORMAT_READ__EMPTY_BUFFER
	, IPC_ERROR_MESSAGE_FORMAT_READ__MESSAGE_SIZE

	, IPC_ERROR_MESSAGE_EMPTY__EMPTY_MESSAGE_LIST
};


struct ipc_connection_info {
    uint32_t version;
    uint32_t index;
    int32_t fd;
	char type; // server, client, arbitrary fd
    char *spath; // max size: PATH_MAX
};

struct ipc_connection_infos {
	struct ipc_connection_info ** cinfos;
	int32_t size;
};


struct ipc_message {
    char type;
    char user_type;
    uint32_t length;
    char *payload;
};

struct ipc_event {
	enum ipc_event_type type;
	struct ipc_connection_info *origin;
	void* m; // message pointer
};


/**
 * MACROS
 **/

// #define IPC_WITH_ERRORS                               3

#ifdef IPC_WITH_ERRORS
#include "logger.h"

#define handle_err(fun,msg)\
    do { log_error ("%s: file %s line %d %s", fun, __FILE__, __LINE__, msg); } while (0)
#else
#define handle_err(fun,msg)
#endif

#define IPC_EVENT_SET(pevent,type_,message_,origin_) {\
	pevent->type = type_; \
	pevent->m = message_; \
	pevent->origin = origin_; \
};

#define IPC_EVENT_CLEAN(pevent) {\
	pevent->type = IPC_EVENT_TYPE_NOT_SET;\
	if (pevent->m != NULL) {\
		ipc_message_empty (pevent->m);\
		free(pevent->m);\
		pevent->m = NULL;\
	}\
};

#define IPC_WITH_UNIX_SOCKETS
#ifdef  IPC_WITH_UNIX_SOCKETS
#include "usocket.h"
#endif


#define LOG

void log_error (const char* message, ...);
void log_info  (const char* message, ...);
void log_debug (const char* message, ...);

/**
 * main public functions
 */

enum ipc_errors ipc_server_init  (char **env, struct ipc_connection_info *srv, const char *sname);
enum ipc_errors ipc_connection   (char **env, struct ipc_connection_info *srv, const char *sname);

enum ipc_errors ipc_server_close (struct ipc_connection_info *srv);
enum ipc_errors ipc_close (struct ipc_connection_info *p);

enum ipc_errors ipc_read (const struct ipc_connection_info *, struct ipc_message *m);
enum ipc_errors ipc_write (const struct ipc_connection_info *, const struct ipc_message *m);

enum ipc_errors ipc_wait_event (struct ipc_connection_infos *clients
		, struct ipc_connection_info *srv
        , struct ipc_event *event);

// store and remove only pointers on allocated structures
enum ipc_errors ipc_add (struct ipc_connection_infos *, struct ipc_connection_info *);
enum ipc_errors ipc_del (struct ipc_connection_infos *, struct ipc_connection_info *);

// add an arbitrary file descriptor to read
enum ipc_errors ipc_add_fd (struct ipc_connection_infos *cinfos, int fd);
enum ipc_errors ipc_del_fd (struct ipc_connection_infos *cinfos, int fd);

void ipc_connections_free  (struct ipc_connection_infos *);


struct ipc_connection_info * ipc_connection_copy (const struct ipc_connection_info *p);
int8_t ipc_connection_eq (const struct ipc_connection_info *p1, const struct ipc_connection_info *p2);
// create the client service structure
enum ipc_errors ipc_connection_gen (struct ipc_connection_info *cinfo
		, uint32_t index, uint32_t version, int fd, char type);

void ipc_connection_print (struct ipc_connection_info *cinfo);
void ipc_connections_print (struct ipc_connection_infos *cinfos);

// get explanation about an error
const char * ipc_errors_get (enum ipc_errors e);


/**
 * message functions
 **/

// used to create msg structure with a certain payload length (0 for no payload memory allocation)
enum ipc_errors ipc_message_new (struct ipc_message **m, ssize_t paylen);
// used to create msg structure from buffer
enum ipc_errors ipc_message_format_read (struct ipc_message *m, const char *buf, ssize_t msize);
// used to create buffer from msg structure
enum ipc_errors ipc_message_format_write (const struct ipc_message *m, char **buf, ssize_t *msize);

// read a structure msg from fd
enum ipc_errors ipc_message_read (int32_t fd, struct ipc_message *m);
// write a structure msg to fd
enum ipc_errors ipc_message_write (int32_t fd, const struct ipc_message *m);

enum ipc_errors ipc_message_format (struct ipc_message *m
		, char type, char utype, const char *payload, ssize_t length);
enum ipc_errors ipc_message_format_data (struct ipc_message *m
		, char utype, const char *payload, ssize_t length);
enum ipc_errors ipc_message_format_server_close (struct ipc_message *m);

enum ipc_errors ipc_message_empty (struct ipc_message *m);



// non public functions
void service_path (char *path, const char *sname, int32_t index, int32_t version);

#endif
