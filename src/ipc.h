#ifndef __IPC_H__
#define __IPC_H__

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>		// error numbers
#include <time.h>

#include <poll.h>

/***
 * global defaults
 **/

#define RUNDIR "/run/ipc/"
#define PATH_MAX 4096
#define IPC_HEADER_SIZE 6
// #define __IPC_BASE_SIZE       500000  // 500 KB
#define __IPC_BASE_SIZE       2000000  // 2 MB, plenty enough space for messages
#define IPC_MAX_MESSAGE_SIZE  __IPC_BASE_SIZE-IPC_HEADER_SIZE

#define IPC_VERSION 4

#if ! defined(IPC_WITHOUT_ERRORS) && ! defined(IPC_WITH_ERRORS)
#define IPC_WITH_ERRORS 2
#endif
#define IPC_WITH_UNIX_SOCKETS

#ifdef  IPC_WITH_UNIX_SOCKETS
#include "usocket.h"
#endif

/***
 * structures and enumerations
 **/

enum msg_types {
	MSG_TYPE_SERVER_CLOSE     = 0
	, MSG_TYPE_ERR            = 1
	, MSG_TYPE_DATA           = 2
	, MSG_TYPE_NETWORK_LOOKUP = 3
};

/**
 * Event types.
 * In the main event loop, servers and clients can receive connections,
 * disconnections, errors or messages from their pairs. They also can
 * set a timer so the loop will allow a periodic routine (sending ping
 * messages for websockets, for instance).
 *
 **
 *
 * A few other events can occur.
 *
 * Extra socket
 *   The main loop waiting for an event can be used as an unique entry
 *   point for socket management. libipc users can register sockets via
 *   ipc_add_fd allowing them to trigger an event, so events unrelated
 *   to libipc are managed the same way.
 * Switch
 *   libipc can be used to create protocol-related programs, such as a
 *   websocket proxy allowing libipc services to be accessible online.
 *   To help those programs (with TCP-complient sockets), two sockets
 *   can be bound together, each message coming from one end will be
 *   automatically transfered to the other socket and a Switch event
 *   will be triggered.
 * Look Up
 *   When a client establishes a connection to a service, it asks the
 *   ipc daemon (ipcd) to locate the service and establish a connection
 *   to it. This is a lookup.
 */

enum ipc_event_type {
	IPC_EVENT_TYPE_NOT_SET         = 0
	, IPC_EVENT_TYPE_ERROR         = 1
	, IPC_EVENT_TYPE_EXTRA_SOCKET  = 2 // Message received from a non IPC socket.
	, IPC_EVENT_TYPE_SWITCH        = 3 // Message to send to a corresponding fd.
	, IPC_EVENT_TYPE_CONNECTION    = 4 // New user.
	, IPC_EVENT_TYPE_DISCONNECTION = 5 // User disconnected.
	, IPC_EVENT_TYPE_MESSAGE       = 6 // New message.
	, IPC_EVENT_TYPE_LOOKUP        = 7 // Client asking for a service through ipcd.
	, IPC_EVENT_TYPE_TIMER         = 8 // Timeout in the poll(2) function.
	, IPC_EVENT_TYPE_TX            = 9 // Message sent.
};

// For IO callbacks (switching).
enum ipccb {
	  IPC_CB_NO_ERROR         = 0 // No error. A message was generated.
	, IPC_CB_FD_CLOSING       = 1 // The fd is closing.
	, IPC_CB_FD_ERROR         = 2 // Generic error.
	, IPC_CB_PARSING_ERROR    = 3 // The message was read but with errors.
	, IPC_CB_IGNORE           = 4 // The message should be ignored (protocol specific).
};

/**
 * Error codes.
 * libipc tend to use unique error codes in the whole library, allowing easier debugging.
 */
enum ipc_error_code {
	IPC_ERROR_NONE                                      = 0
	, IPC_ERROR_HANDLE_MESSAGE__NOT_ENOUGH_MEMORY       = 3
	, IPC_ERROR_CLOSED_RECIPIENT                        = 4
	, IPC_ERROR_SERVICE_PATH__NO_PATH                   = 5
	, IPC_ERROR_SERVICE_PATH__NO_SERVICE_NAME           = 6
	, IPC_ERROR_SERVER_INIT__NO_ENVIRONMENT_PARAM       = 7
	, IPC_ERROR_SERVER_INIT__NO_SERVICE_PARAM           = 8
	, IPC_ERROR_SERVER_INIT__NO_SERVER_NAME_PARAM       = 9
	, IPC_ERROR_SERVER_INIT__MALLOC                     = 10
	, IPC_ERROR_WRITE__NO_MESSAGE_PARAM                 = 11
	, IPC_ERROR_WRITE_FD__NOT_ENOUGH_DATA               = 12
	, IPC_ERROR_READ__NO_MESSAGE_PARAM                  = 13
	, IPC_ERROR_CONNECTION__NO_SERVICE_NAME             = 15
	, IPC_ERROR_CONNECTION__NO_ENVIRONMENT_PARAM        = 16
	, IPC_ERROR_ACCEPT__NO_SERVICE_PARAM                = 18
	, IPC_ERROR_ACCEPT__NO_CLIENT_PARAM                 = 19
	, IPC_ERROR_HANDLE_NEW_CONNECTION__NO_CINFO_PARAM   = 20
	, IPC_ERROR_HANDLE_NEW_CONNECTION__NO_CINFOS_PARAM  = 21
	, IPC_ERROR_WAIT_EVENT__SELECT                      = 22
	, IPC_ERROR_WAIT_EVENT__NO_CLIENTS_PARAM            = 23
	, IPC_ERROR_WAIT_EVENT__NO_EVENT_PARAM              = 24
	, IPC_ERROR_HANDLE_NEW_CONNECTION__MALLOC           = 25
	, IPC_ERROR_ADD__EMPTY_LIST                         = 26
	, IPC_ERROR_ADD__NO_PARAM_CLIENTS                   = 27
	, IPC_ERROR_ADD__NO_PARAM_CLIENT                    = 28
	, IPC_ERROR_ADD__MALLOC                             = 29
	, IPC_ERROR_ADD_FD__NO_PARAM_CINFOS                 = 30
	, IPC_ERROR_ADD_FD__NOT_ENOUGH_MEMORY               = 31
	, IPC_ERROR_DEL_FD__NO_PARAM_CINFOS                 = 32
	, IPC_ERROR_DEL_FD__EMPTIED_LIST                    = 33
	, IPC_ERROR_DEL_FD__EMPTY_LIST                      = 34
	, IPC_ERROR_DEL_FD__CANNOT_FIND_CLIENT              = 35
	, IPC_ERROR_CONTACT_IPCD__NO_SERVICE_NAME_PARAM = 36
	, IPC_ERROR_CONTACT_IPCD__NO_SERVER_PARAM       = 37
	, IPC_ERROR_DEL__EMPTY_LIST                         = 38
	, IPC_ERROR_DEL__EMPTIED_LIST                       = 39
	, IPC_ERROR_DEL__CANNOT_FIND_CLIENT                 = 40
	, IPC_ERROR_DEL__NO_CLIENTS_PARAM                   = 41
	, IPC_ERROR_DEL__NO_CLIENT_PARAM                    = 42
	, IPC_ERROR_USOCK_SEND                              = 43
	, IPC_ERROR_USOCK_CONNECT__SOCKET                   = 44
	, IPC_ERROR_USOCK_CONNECT__WRONG_FILE_DESCRIPTOR    = 45
	, IPC_ERROR_USOCK_CONNECT__EMPTY_PATH               = 46
	, IPC_ERROR_USOCK_CONNECT__CONNECT                  = 47
	, IPC_ERROR_USOCK_CLOSE                             = 48
	, IPC_ERROR_USOCK_REMOVE__UNLINK                    = 49
	, IPC_ERROR_USOCK_REMOVE__NO_FILE                   = 50
	, IPC_ERROR_USOCK_INIT__EMPTY_FILE_DESCRIPTOR       = 51
	, IPC_ERROR_USOCK_INIT__WRONG_FILE_DESCRIPTOR       = 52
	, IPC_ERROR_USOCK_INIT__EMPTY_PATH                  = 53
	, IPC_ERROR_USOCK_INIT__BIND                        = 54
	, IPC_ERROR_USOCK_INIT__LISTEN                      = 55
	, IPC_ERROR_USOCK_ACCEPT__PATH_FILE_DESCRIPTOR      = 56
	, IPC_ERROR_USOCK_ACCEPT                            = 57
	, IPC_ERROR_USOCK_RECV__NO_BUFFER                   = 58
	, IPC_ERROR_USOCK_RECV__NO_LENGTH                   = 59
	, IPC_ERROR_USOCK_RECV                              = 60
	, IPC_ERROR_USOCK_RECV__UNRECOGNIZED_ERROR          = 61
	, IPC_ERROR_USOCK_RECV__HEAP_ALLOCATION             = 62
	, IPC_ERROR_USOCK_RECV__MESSAGE_SIZE                = 63
	, IPC_ERROR_RECEIVE_FD__NO_PARAM_FD                 = 64
	, IPC_ERROR_RECEIVE_FD__RECVMSG                     = 65
	, IPC_ERROR_PROVIDE_FD__SENDMSG                     = 66
	, IPC_ERROR_MESSAGE_FORMAT_WRITE__MESSAGE_LENGTH    = 67
	, IPC_ERROR_MESSAGE_FORMAT__MESSAGE_SIZE            = 68
	, IPC_ERROR_MESSAGE_FORMAT__NO_MESSAGE_PARAM        = 69
	, IPC_ERROR_MESSAGE_FORMAT__INCONSISTENT_PARAMS     = 70
	, IPC_ERROR_MESSAGE_FORMAT__HEAP_ALLOCATION         = 71
	, IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_MESSAGE     = 72
	, IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_MSIZE       = 73
	, IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_BUFFER      = 74
	, IPC_ERROR_MESSAGE_FORMAT_READ__EMPTY_MESSAGE      = 75
	, IPC_ERROR_MESSAGE_FORMAT_READ__EMPTY_BUFFER       = 76
	, IPC_ERROR_MESSAGE_FORMAT_READ__PARAM_MESSAGE_SIZE = 77
	, IPC_ERROR_MESSAGE_FORMAT_READ__READ_MESSAGE_SIZE  = 78
	, IPC_ERROR_MESSAGE_FORMAT_READ__MESSAGE_TOO_LONG   = 79
	, IPC_ERROR_MESSAGE_EMPTY__EMPTY_MESSAGE_LIST       = 80
	, IPC_ERROR_MKDIR__CANNOT_CREATE_DIR                = 81
	, IPC_ERROR_MKDIR__NAME_TOO_LONG                    = 82
	, IPC_ERROR_DIR_SETUP__NOT_A_DIRECTORY              = 83
	, IPC_ERROR_DIR_SETUP__DIRECTORY_NOT_WRITABLE       = 84
	, IPC_ERROR_DIRECTORY_SETUP__PATH_PARAM             = 85

	, IPC_ERROR_SERVER_INIT__NOT_ENOUGH_MEMORY            = 86
	, IPC_ERROR_CONNECTION__NOT_ENOUGH_MEMORY             = 87
	, IPC_ERROR_CTX_INIT__NO_CONTEXT_PARAM                = 88
	, IPC_ERROR_CTX_INIT__CONTEXT_ALREADY_INIT            = 89
	, IPC_ERROR_ADD__MALLOC_POLLFD                        = 90
	, IPC_ERROR_ADD_MESSAGE_TO_SEND__EMPTY_LIST           = 91
	, IPC_ERROR_ADD_MESSAGE_TO_SEND__MALLOC               = 92
	, IPC_ERROR_ADD_MESSAGE_TO_SEND__NO_PARAM_MESSAGE     = 93
	, IPC_ERROR_ADD_MESSAGE_TO_SEND__NO_PARAM_MESSAGES    = 94
	, IPC_ERROR_CONNECTION__NO_CTX                        = 95
	, IPC_ERROR_CTX_INIT__MALLOC_CTX                      = 96
	, IPC_ERROR_CTX_INIT__MALLOC_POLLFD                   = 97
	, IPC_ERROR_CONTACT_IPCD__NO_FD_PARAM                 = 98
	, IPC_ERROR_HANDLE_NEW_CONNECTION__INCONSISTENT_INDEX = 99
	, IPC_ERROR_DEL_MESSAGE_TO_SEND__NO_PARAM_MESSAGES    = 100
	, IPC_ERROR_MESSAGE_DEL__INDEX_ERROR                  = 101
	, IPC_ERROR_MESSAGE_DEL__EMPTY_LIST                   = 102
	, IPC_ERROR_ADD__NO_PARAM_POLLFD                      = 103
	, IPC_ERROR_WRITE__FD_NOT_FOUND                       = 104
	, IPC_ERROR_ADD__NOT_ENOUGH_MEMORY                    = 105
	, IPC_ERROR_WAIT_EVENT__POLL                          = 106
	, IPC_ERROR_FD_SWITCHING__NO_FD_RECORD                = 107
	, IPC_ERROR_CLOSE_ALL__NO_CTX_PARAM                   = 108
	, IPC_ERROR_CLOSE__NO_CTX_PARAM                       = 109
};

struct ipc_error {
	enum ipc_error_code error_code;
	char error_message[BUFSIZ];
};

// get explanation about an error
// This only returns the generic error based on its code.
// Library's users have a more meaningful insight on the error
// with the error_message string in the ipc_error structure.
const char *ipc_errors_get (enum ipc_error_code e);


enum ipc_connection_type {
	IPC_CONNECTION_TYPE_IPC        = 0
	, IPC_CONNECTION_TYPE_EXTERNAL = 1
	  /** Messages received = new connections. */
	, IPC_CONNECTION_TYPE_SERVER   = 2
	  /** IO operations should go through registered callbacks. */
	, IPC_CONNECTION_TYPE_SWITCHED = 3
};

struct ipc_connection_info {
	enum ipc_connection_type type;
	short int more_to_read;
	char *spath; // max size: PATH_MAX
};

struct ipc_message {
	char type;        // Internal message type.
	char user_type;   // User-defined message type.
	int fd;           // File descriptor concerned about this message.
	uint32_t length;  // Payload length.
	char *payload;
};

struct ipc_messages {
	struct ipc_message *messages;
	size_t size;
};

struct ipc_switching {
	int orig;
	int dest;
	enum ipccb (*orig_in)  (int origin_fd, struct ipc_message *m, short int *more_to_read);
	enum ipccb (*orig_out) (int origin_fd, struct ipc_message *m);
	enum ipccb (*dest_in)  (int origin_fd, struct ipc_message *m, short int *more_to_read);
	enum ipccb (*dest_out) (int origin_fd, struct ipc_message *m);
};

struct ipc_switchings {
	struct ipc_switching *collection;
	size_t size;
};

void ipc_message_copy (struct ipc_message *m
	, uint32_t fd
	, uint8_t type
	, uint8_t utype
	, char *payload
	, uint32_t paylen);
struct ipc_error ipc_messages_del  (struct ipc_messages *messages, uint32_t index);

/**
 * Context of the whole networking state.
 */
struct ipc_ctx {
	/**
	 * Keep track of connections.
	 */
	struct ipc_connection_info *cinfos;
	/**
	 * List of "pollfd" structures within cinfos, so we can pass it to poll(2).
	 */
	struct pollfd              *pollfd;
	/**
	 * Size of the connection list.
	 */
	size_t size;
	/**
	 * List of messages to send, once the fd are available.
	 */
	struct ipc_messages tx;
	/**
	 * Relations between fd.
	 */
	struct ipc_switchings switchdb;
};

struct ipc_event {
	enum ipc_event_type type;
	uint32_t index;
	int      origin;
	void     *m;  // message pointer
};

/***
 * ipc event macros
 **/

#define IPC_EVENT_SET(pevent,type_,index_, origin_fd_,message_) {\
	pevent->type = type_; \
	pevent->index = index_; \
	pevent->origin = origin_fd_; \
	pevent->m = message_; \
};

#define IPC_EVENT_CLEAN(pevent) {\
	pevent->type = IPC_EVENT_TYPE_NOT_SET;\
	pevent->origin = 0;\
	pevent->index  = 0;\
	if (pevent->m != NULL) {\
		ipc_message_empty (pevent->m);\
		free(pevent->m);\
		pevent->m = NULL;\
	}\
};

/**
 * main public functions
 **/

struct ipc_error ipc_wait_event (struct ipc_ctx *, struct ipc_event *, int *timer);

struct ipc_error ipc_server_init (struct ipc_ctx *ctx, const char *sname);
struct ipc_error ipc_connection  (struct ipc_ctx *ctx, const char *sname, int *fd);
struct ipc_error ipc_connection_switched (struct ipc_ctx *ctx, const char *sname, int clientfd, int *serverfd);

struct ipc_error ipc_close     (struct ipc_ctx *ctx, uint32_t index);
struct ipc_error ipc_close_all (struct ipc_ctx *ctx);

void ipc_ctx_free (struct ipc_ctx *ctx);

struct ipc_error ipc_read     (const struct ipc_ctx *, uint32_t index, struct ipc_message *m);
struct ipc_error ipc_read_fd  (int32_t fd, struct ipc_message *m);
struct ipc_error ipc_write (struct ipc_ctx *, const struct ipc_message *m);

struct ipc_error fd_switching_read  (struct ipc_event *event, struct ipc_ctx *ctx, int index);
struct ipc_error fd_switching_write (struct ipc_event *event, struct ipc_ctx *ctx, int index);

// store and remove only pointers on allocated structures
struct ipc_error ipc_add (struct ipc_ctx *, struct ipc_connection_info *, struct pollfd *);
struct ipc_error ipc_del (struct ipc_ctx *, uint32_t index);

// add an arbitrary file descriptor to read
struct ipc_error ipc_add_fd (struct ipc_ctx *ctx, int fd);
// add a switched file descriptor to read
struct ipc_error ipc_add_fd_switched (struct ipc_ctx *ctx, int fd);
struct ipc_error ipc_del_fd (struct ipc_ctx *ctx, int fd);

/***
 * message functions
 **/

uint32_t ipc_message_raw_serialize (char *buffer, char type, char user_type, char *message, uint32_t message_size);

// used to create msg structure from buffer
struct ipc_error ipc_message_format_read (struct ipc_message *m, const char *buf, size_t msize);
// used to create buffer from msg structure
struct ipc_error ipc_message_format_write (const struct ipc_message *m, char **buf, size_t * msize);

struct ipc_error ipc_message_format (struct ipc_message *m, char type, char utype, const char *payload, size_t length);
struct ipc_error ipc_message_format_data (struct ipc_message *m, char utype, const char *payload, size_t length);
struct ipc_error ipc_message_format_server_close (struct ipc_message *m);
struct ipc_error ipc_message_empty (struct ipc_message *m);

struct ipc_error ipc_messages_add (struct ipc_messages *, const struct ipc_message *);
void ipc_messages_free (struct ipc_messages *);

// Switch cases macros
// print on error
#define ERROR_CASE(e,f,m) case e : { fprintf (stderr, "function %s: %s\n", f, m); } break;

/***
 * non public functions
 **/

struct ipc_error ipc_write_fd (int fd, const struct ipc_message *m);
struct ipc_error ipc_ctx_init (struct ipc_ctx **);
struct ipc_error ipc_ctx_new_alloc (struct ipc_ctx *ctx);
struct ipc_error service_path (char *path, const char *sname);
struct ipc_error handle_writing_message (struct ipc_event *event, struct ipc_ctx *ctx, uint32_t index);

void ipc_ctx_print (struct ipc_ctx *ctx);

// Last parameter is the index for the server fd in the context structure.
struct ipc_error ipc_accept_add       (struct ipc_event *event, struct ipc_ctx *ctx, uint32_t index);
struct ipc_error ipc_contact_ipcd (int *pfd, const char *sname);
struct ipc_error service_path         (char *path, const char *sname);

/***
 * ipcd enumerations, structures and functions
 **/

void ipc_ctx_switching_add (struct ipc_ctx *ctx, int orig, int dest);
int  ipc_ctx_switching_del (struct ipc_ctx *ctx, int fd);
void ipc_switching_add (struct ipc_switchings *is, int orig, int dest);
int ipc_switching_del (struct ipc_switchings *is, int fd);
int ipc_switching_get (struct ipc_switchings *is, int fd);
void ipc_switching_free (struct ipc_switchings *is);
void ipc_switching_callbacks_ (struct ipc_ctx *ctx, int fd
	, enum ipccb (*cb_in )(int fd, struct ipc_message *m, short int *more_to_read));
void ipc_switching_callbacks (
	  struct ipc_ctx *ctx
	, int fd
	, enum ipccb (*cb_in )(int fd, struct ipc_message *m, short int *more_to_read)
	, enum ipccb (*cb_out)(int fd, struct ipc_message *m));

int ipc_ctx_fd_type (struct ipc_ctx *ctx, int fd, enum ipc_connection_type type);

void ipc_switching_print (struct ipc_switchings *is);

struct ipc_error ipc_receive_fd (int sock, int *fd);
struct ipc_error ipc_provide_fd (int sock, int fd);

/***
 * grooming macros
 **/

#define SECURE_DECLARATION(t,v)                   t v; memset(&v,0,sizeof(t));
#define SECURE_BUFFER_DECLARATION(type,name,size) type name[size]; memset(&name, 0, sizeof(type) * size);
#define SECURE_BUFFER_HEAP_ALLOCATION(p,len,instr,r)\
	{ p = malloc (len); if (p == NULL) { instr; r; } ; memset(p, 0, len); }
#define SECURE_BUFFER_HEAP_ALLOCATION_R(p,len,instr,r) SECURE_BUFFER_HEAP_ALLOCATION(p,len,instr, IPC_RETURN_ERROR(r); )
#define SECURE_BUFFER_HEAP_ALLOCATION_R_NULL(p,len,instr) SECURE_BUFFER_HEAP_ALLOCATION(p,len,instr, return NULL; )
#define SECURE_BUFFER_HEAP_ALLOCATION_Q(p,len,instr,r)    SECURE_BUFFER_HEAP_ALLOCATION(p,len,instr, exit(r))

// Test macros, requiring the variable `enum ipc_error_code ret`

// one macro to rule them all!
//   1. function to test
//   2. Test IPC error based (test itself)
//   3. Instructions to exec on failure
//   4. Return something
#define TEST_IPC_T_P_I_R(function_to_test, test, instr, r) \
{\
	struct ipc_error ret = function_to_test;\
	if (test) {\
		instr;\
		r;\
	}\
}

// R  = return r param
// RR = return "ret" variable
// RV = return void
// Q  = quit
// I  = additionnal instructions before returning on error

#define TEST_IPC_T_I_P_Q(f,t,instr,err,r)  TEST_IPC_T_P_I_R(f,                                 t,  instr,     exit(r))
#define TEST_IPC_T_I_RR(f,t,instr)         TEST_IPC_T_P_I_R(f,                                 t,  instr,  return ret)
#define TEST_IPC_I_RR(f,instr)             TEST_IPC_T_P_I_R(f,  ret.error_code != IPC_ERROR_NONE,  instr,  return ret)

// Tests macros, do not require `enum ipc_error_code ret` variable
// test => return error code
#define T_R(t,r)          if t { IPC_RETURN_ERROR(r); }
#define T_R_NULL(t)       if t {         return NULL; }
#define T_R_NOTHING(t)    if t {         return     ; }

// test => perror then return (for system functions)
#define T_PERROR_R(t,m,r) if t { perror(m); return (r); }
// test => perror then exit (for system functions)
#define T_PERROR_Q(t,m,r) if t { perror(m); exit(r); }

#define T_PERROR_RIPC(t,m,r) if t { perror(m); IPC_RETURN_ERROR(r); }

#define TEST_IPC_QUIT_ON_ERROR(function_to_test,ec) {\
	struct ipc_error ret = function_to_test;\
	if (ret.error_code != IPC_ERROR_NONE) {\
		fprintf(stderr, "%s\n", ret.error_message);\
		exit(ec);\
	}\
}

#define TEST_IPC_RETURN_ON_ERROR_FREE(function_to_test, buffer_to_free) {\
	struct ipc_error ret = function_to_test; \
	if (ret.error_code != IPC_ERROR_NONE) {\
		if (buffer_to_free != NULL) {\
			free(buffer_to_free); \
		}\
		return ret;\
	}\
}

#define TEST_IPC_RETURN_ON_ERROR(function_to_test) {\
	struct ipc_error ret = function_to_test;\
	if (ret.error_code != IPC_ERROR_NONE) {\
		return ret;\
	}\
}
#define TEST_IPC_RETURN_ON_ERROR_FREE(function_to_test, buffer_to_free) {\
	struct ipc_error ret = function_to_test; \
	if (ret.error_code != IPC_ERROR_NONE) {\
		if (buffer_to_free != NULL) {\
			free(buffer_to_free); \
		}\
		return ret;\
	}\
}

// formatted version of TEST_IPC_RR
#define TEST_IPC_RR_F(function_to_test, format, ...) {\
	struct ipc_error ret = function_to_test;\
	if (ret.error_code != IPC_ERROR_NONE) { \
		error_message_format (ret.error_message + strlen(ret.error_message) \
			, "blocking error" \
			, ":" __FILE__ ":" format \
			, ##__VA_ARGS__ ); \
		return ret;\
	}\
}

// TODO: ret already contains error message, append the error_message
// TODO: currently, error message is not what it should be
#define TEST_IPC_RR(function_to_test, err_message) \
	TEST_IPC_RR_F(function_to_test, "%s", err_message)

// same as TEST_IPC_RR but do not return
// Also: print the error right away.
#define TEST_IPC_P(function_to_test, err_message) {\
	struct ipc_error ret = function_to_test;\
	if (ret.error_code != IPC_ERROR_NONE) {\
		error_message_format (ret.error_message + strlen(ret.error_message) \
			, "non blocking error" \
			, ":" __FILE__ "%s:%s" \
			, err_message );\
		\
		fprintf (stderr, "%s\n", ret.error_message);\
	}\
}

#define IPC_RETURN_NO_ERROR { \
	SECURE_DECLARATION (struct ipc_error, ret); \
	ret.error_code = IPC_ERROR_NONE; \
	return ret; \
}

#define IPC_FILL_DEFAULT_ERROR_MESSAGE(error_message_ptr,error_code) { \
	const char *estr = ipc_errors_get (error_code); \
	snprintf(error_message_ptr, BUFSIZ, "%s", estr); \
}

#define IPC_RETURN_ERROR(ec_) \
	SECURE_DECLARATION(struct ipc_error, ret);\
	IPC_FILL_DEFAULT_ERROR_MESSAGE(ret.error_message, ec_);\
	ret.error_code = ec_;\
	return ret;

#define IPC_RETURN_ERROR_FORMAT(v,format,...) { \
	SECURE_DECLARATION (struct ipc_error, ret); \
	ret.error_code = v; \
	error_message_format (ret.error_message \
		, "blocking error" \
		, format \
		, ##__VA_ARGS__ ); \
	return ret; \
}

#define TEST_IPC_Q(f,e) {\
	struct ipc_error ret = f; \
	if (ret.error_code != IPC_ERROR_NONE) { \
		fprintf(stderr, "%s", ret.error_message); \
		exit(e); \
	} \
}

#define TEST_IPC_WAIT_EVENT_RR(f, r) { \
	struct ipc_error ret = f; \
	if (ret.error_code != IPC_ERROR_NONE && \
		ret.error_code != IPC_ERROR_CLOSED_RECIPIENT) { \
		return ret; \
	} \
}

#define TEST_IPC_WAIT_EVENT_Q(f, r) { \
	struct ipc_error ret = f; \
	if (ret.error_code != IPC_ERROR_NONE && \
		ret.error_code != IPC_ERROR_CLOSED_RECIPIENT) { \
		exit(r); \
	} \
}

#endif
