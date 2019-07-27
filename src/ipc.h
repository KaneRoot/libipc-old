#ifndef __IPC_H__
#define __IPC_H__

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // error numbers
#include <time.h>

/***
 * global defaults
 **/

#define RUNDIR "/run/ipc/"
#define PATH_MAX 4096
#define IPC_HEADER_SIZE  6
#define IPC_MAX_MESSAGE_SIZE  8000000-IPC_HEADER_SIZE

#define IPC_VERSION 1

#if ! defined(IPC_WITHOUT_ERRORS) && ! defined(IPC_WITH_ERRORS)
#define IPC_WITH_ERRORS                               2
#endif
#define IPC_WITH_UNIX_SOCKETS

#ifdef  IPC_WITH_UNIX_SOCKETS
#include "usocket.h"
#endif

/***
 * grooming macros
 **/

#define SECURE_DECLARATION(t,v)   t v; memset(&v,0,sizeof(t));
#define SECURE_BUFFER_DECLARATION(type,name,size) type name[size]; memset(&name, 0, sizeof(type) * size);
#define SECURE_BUFFER_HEAP_ALLOCATION(p,len,instr,r)   { p = malloc (len); if (p == NULL) { instr; r; } ; memset(p, 0, len); }
#define SECURE_BUFFER_HEAP_ALLOCATION_R(p,len,instr,r) SECURE_BUFFER_HEAP_ALLOCATION(p,len,instr, return r )
#define SECURE_BUFFER_HEAP_ALLOCATION_Q(p,len,instr,r) SECURE_BUFFER_HEAP_ALLOCATION(p,len,instr, exit(r))

#if defined(IPC_WITH_ERRORS) && IPC_WITH_ERRORS > 2
// print error string
#define PRINT_ERR(code)           const char *estr = ipc_errors_get (code); LOG_ERROR ("%s", estr);
#define PRINT_ERR_STR(code,err) { const char *estr = ipc_errors_get (code); LOG_ERROR ("%s - %s", err, estr); }
#else
#define PRINT_ERR(code)
#define PRINT_ERR_STR(code,err)
#endif

// Test macros, requiring the variable `enum ipc_errors ret`

// one macro to rule them all!
//   1. function to test
//   2. Test IPC error based (test itself)
//   3. Print error (then ipc-error message)
//   4. Instructions to exec on failure
//   5. Return something
#define TIPC_T_P_I_R(f, t, err, instr, r) { enum ipc_errors ret = f;\
	if (t) {\
		PRINT_ERR_STR(ret, err); \
		instr;\
		r;\
	} }

// P  = print somehting with LOG_ERROR
// NP = no LOG_ERROR print
// R  = return r param
// RR = return "ret" variable
// RV = return void
// Q  = quit
// I  = additionnal instructions before returning on error

#define TIPC_P_I_R(f, t, err, instr, r) TIPC_T_P_I_R(f, ret != IPC_ERROR_NONE, err, instr, return r)
#define TIPC_P_I_Q(f, t, err, instr, r) TIPC_T_P_I_R(f, ret != IPC_ERROR_NONE, err, instr, quit(r))
#define TIPC_P_Q(f, err,r)             TIPC_T_P_I_R(f, ret != IPC_ERROR_NONE, err, ;, exit(r))
#define TIPC_P_R(f, err,r)             TIPC_T_P_I_R(f, ret != IPC_ERROR_NONE, err, ;, return r)
#define TIPC_P_RR(f, err)              TIPC_P_R(f,err,ret)
#define TIPC_F_RR(f, format)           TIPC_T_P_I_R(f, ret != IPC_ERROR_NONE,   "", LOG_ERROR format ;, return ret)
#define TIPC_F_Q(f, format, r)         TIPC_T_P_I_R(f, ret != IPC_ERROR_NONE,   "", LOG_ERROR format ;, exit(r))
#define TIPC_F_R(f, format, r)         TIPC_T_P_I_R(f, ret != IPC_ERROR_NONE,   "", LOG_ERROR format ;, return r)
#define TIPC_P_RV(f, err)              TIPC_P_R(f,err,;)
#define TIPC_P(f, err)                 TIPC_T_P_I_R(f, ret != IPC_ERROR_NONE,  err,       ;,            ;)
#define TIPC_T_I_P_Q(f,t,instr,err,r)  TIPC_T_P_I_R(f,                     t,  err,   instr,      exit(r))
#define TIPC_T_I_RR(f,t,instr)         TIPC_T_P_I_R(f,                     t,   "",   instr,   return ret)
#define TIPC_I_RR(f,instr)             TIPC_T_P_I_R(f, ret != IPC_ERROR_NONE,   "",   instr,   return ret)
#define TIPC_RR(f)                     TIPC_T_P_I_R(f, ret != IPC_ERROR_NONE,   "",       ;,   return ret)
#define TIPC_NP_RR(f)                  { enum ipc_errors ret = f; if (ret != IPC_ERROR_NONE) { return ret; } }

// Tests macros, do not require `enum ipc_errors ret` variable
// test => return error code
#define T_R(t,r)          if t { return r; }
// test => perror, formatted message then return (for system functions)
#define T_PERROR_F_R(t,m,fmt,r) if t { perror(m); LOG_ERROR fmt;  return r; }
// test => perror then return (for system functions)
#define T_PERROR_R(t,m,r) if t { perror(m); return r; }
// test => perror then exit (for system functions)
#define T_PERROR_Q(t,m,r) if t { perror(m); exit(r); }


// Switch cases macros
// print on error
#define ERROR_CASE(e,f,m) case e : { LOG_ERROR ("function %s: %s", f, m); } break;


/***
 * structures and enumerations
 **/

enum msg_types {
	MSG_TYPE_SERVER_CLOSE     = 0
	, MSG_TYPE_ERR            = 1
	, MSG_TYPE_DATA           = 2
	, MSG_TYPE_NETWORK_LOOKUP = 3
} message_types;

enum ipc_event_type {
	  IPC_EVENT_TYPE_NOT_SET        = 0
	, IPC_EVENT_TYPE_ERROR          = 1
	, IPC_EVENT_TYPE_EXTRA_SOCKET   = 2
	, IPC_EVENT_TYPE_SWITCH         = 3
	, IPC_EVENT_TYPE_CONNECTION     = 4
	, IPC_EVENT_TYPE_DISCONNECTION  = 5
	, IPC_EVENT_TYPE_MESSAGE        = 6
	, IPC_EVENT_TYPE_LOOKUP         = 7
};

enum ipc_errors {

	/* general errors */
	IPC_ERROR_NONE                                      = 0
	, IPC_ERROR_NOT_ENOUGH_MEMORY                       = 1
	, IPC_ERROR_CLOSED_RECIPIENT                        = 2
	, IPC_ERROR_SERVICE_PATH__NO_PATH                   = 3
	, IPC_ERROR_SERVICE_PATH__NO_SERVICE_NAME           = 4
	, IPC_ERROR_SERVER_INIT__NO_ENVIRONMENT_PARAM       = 5
	, IPC_ERROR_SERVER_INIT__NO_SERVICE_PARAM           = 6
	, IPC_ERROR_SERVER_INIT__NO_SERVER_NAME_PARAM       = 7
	, IPC_ERROR_SERVER_INIT__MALLOC                     = 8
	, IPC_ERROR_WRITE__NO_MESSAGE_PARAM                 = 9
	, IPC_ERROR_WRITE__NOT_ENOUGH_DATA                  = 10
	, IPC_ERROR_READ__NO_MESSAGE_PARAM                  = 11
	, IPC_ERROR_CONNECTION__NO_SERVER                   = 12
	, IPC_ERROR_CONNECTION__NO_SERVICE_NAME             = 13
	, IPC_ERROR_CONNECTION__NO_ENVIRONMENT_PARAM        = 14
	, IPC_ERROR_CONNECTION_GEN__NO_CINFO                = 15
	, IPC_ERROR_ACCEPT__NO_SERVICE_PARAM                = 16
	, IPC_ERROR_ACCEPT__NO_CLIENT_PARAM                 = 17
	, IPC_ERROR_ACCEPT                                  = 18
	, IPC_ERROR_HANDLE_NEW_CONNECTION__NO_CINFO_PARAM   = 19
	, IPC_ERROR_HANDLE_NEW_CONNECTION__NO_CINFOS_PARAM  = 20
	, IPC_ERROR_WAIT_EVENT__SELECT                      = 21
	, IPC_ERROR_WAIT_EVENT__NO_CLIENTS_PARAM            = 22
	, IPC_ERROR_WAIT_EVENT__NO_EVENT_PARAM              = 23
	, IPC_ERROR_HANDLE_NEW_CONNECTION__MALLOC           = 24
	, IPC_ERROR_ADD__EMPTY_LIST                         = 25
	, IPC_ERROR_ADD__NO_PARAM_CLIENTS                   = 26
	, IPC_ERROR_ADD__NO_PARAM_CLIENT                    = 27
	, IPC_ERROR_ADD__MALLOC                             = 28
	, IPC_ERROR_ADD_FD__NO_PARAM_CINFOS                 = 29
	, IPC_ERROR_ADD_FD__EMPTY_LIST                      = 30
	, IPC_ERROR_DEL_FD__NO_PARAM_CINFOS                 = 31
	, IPC_ERROR_DEL_FD__EMPTIED_LIST                    = 32
	, IPC_ERROR_DEL_FD__EMPTY_LIST                      = 33
	, IPC_ERROR_DEL_FD__CANNOT_FIND_CLIENT              = 34
	, IPC_ERROR_CONTACT_NETWORKD__NO_SERVICE_NAME_PARAM = 35
	, IPC_ERROR_CONTACT_NETWORKD__NO_SERVER_PARAM       = 36
	, IPC_ERROR_CONTACT_NETWORKD__NETWORKD              = 37
	, IPC_ERROR_DEL__EMPTY_LIST                         = 38
	, IPC_ERROR_DEL__EMPTIED_LIST                       = 39
	, IPC_ERROR_DEL__CANNOT_FIND_CLIENT                 = 40
	, IPC_ERROR_DEL__NO_CLIENTS_PARAM                   = 41
	, IPC_ERROR_DEL__NO_CLIENT_PARAM                    = 42
	
	/* unix socket */
    , IPC_ERROR_USOCK_SEND = 1
	, IPC_ERROR_USOCK_CONNECT__SOCKET                = 43
	, IPC_ERROR_USOCK_CONNECT__WRONG_FILE_DESCRIPTOR = 44
	, IPC_ERROR_USOCK_CONNECT__EMPTY_PATH            = 45
	, IPC_ERROR_USOCK_CONNECT__CONNECT               = 46
	, IPC_ERROR_USOCK_CLOSE                          = 47
	, IPC_ERROR_USOCK_REMOVE__UNLINK                 = 48
	, IPC_ERROR_USOCK_REMOVE__NO_FILE                = 49
	, IPC_ERROR_USOCK_INIT__EMPTY_FILE_DESCRIPTOR    = 50
	, IPC_ERROR_USOCK_INIT__WRONG_FILE_DESCRIPTOR    = 51
	, IPC_ERROR_USOCK_INIT__EMPTY_PATH               = 52
	, IPC_ERROR_USOCK_INIT__BIND                     = 53
	, IPC_ERROR_USOCK_INIT__LISTEN                   = 54
	, IPC_ERROR_USOCK_ACCEPT__PATH_FILE_DESCRIPTOR   = 55
	, IPC_ERROR_USOCK_ACCEPT                         = 56
	, IPC_ERROR_USOCK_RECV__NO_BUFFER                = 57
	, IPC_ERROR_USOCK_RECV__NO_LENGTH                = 58
	, IPC_ERROR_USOCK_RECV                           = 59
	, IPC_ERROR_USOCK_RECV__MESSAGE_SIZE             = 60
	, IPC_ERROR_RECEIVE_FD__NO_PARAM_FD              = 61
	, IPC_ERROR_RECEIVE_FD__RECVMSG                  = 62
	, IPC_ERROR_PROVIDE_FD__SENDMSG                  = 63

	/* message function errors */
	, IPC_ERROR_MESSAGE_FORMAT_WRITE__MESSAGE_LENGTH   = 64
	, IPC_ERROR_MESSAGE_FORMAT__MESSAGE_SIZE           = 65
	, IPC_ERROR_MESSAGE_FORMAT__NO_MESSAGE_PARAM       = 66
	, IPC_ERROR_MESSAGE_FORMAT__INCONSISTENT_PARAMS    = 67
	, IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_MESSAGE    = 68
	, IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_MSIZE      = 69
	, IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_BUFFER     = 70
	, IPC_ERROR_MESSAGE_FORMAT_READ__EMPTY_MESSAGE     = 71
	, IPC_ERROR_MESSAGE_FORMAT_READ__EMPTY_BUFFER      = 72
	, IPC_ERROR_MESSAGE_FORMAT_READ__MESSAGE_SIZE      = 73
	, IPC_ERROR_MESSAGE_FORMAT_READ__READ_MESSAGE_SIZE = 74
	, IPC_ERROR_MESSAGE_EMPTY__EMPTY_MESSAGE_LIST      = 75
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
	size_t size;
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


/***
 * logging macros
 **/

#ifdef IPC_WITH_ERRORS
#include "logger.h"

// XXX: ##__VA_ARGS__ is a GNU extension to avoid requiring more arguments
#define LOG_ERROR(format, ...) log_error ( __FILE__ ":%d:" format, __LINE__, ##__VA_ARGS__ )
#define LOG_INFO(format, ...)  log_info  ( __FILE__ ":%d:" format, __LINE__, ##__VA_ARGS__ )
#else
#define LOG_ERROR(format, ...)
#define LOG_INFO(format, ...)
#endif

#if IPC_WITH_ERRORS > 2
#define LOG_DEBUG(format, ...) log_debug ( __FILE__ ":%d:" format, __LINE__, ##__VA_ARGS__ )
#else
#define LOG_DEBUG(format, ...)
#endif

/***
 * ipc event macros
 **/

#define IPC_EVENT_SET(pevent,type_,message_,origin_) {\
	pevent->type = type_; \
	pevent->m = message_; \
	pevent->origin = origin_; \
};

enum ipc_connection_types {
	IPC_CONNECTION_TYPE_IPC = 0
	, IPC_CONNECTION_TYPE_EXTERNAL = 1
};

#define IPC_EVENT_CLEAN(pevent) {\
	pevent->type = IPC_EVENT_TYPE_NOT_SET;\
	if (pevent->m != NULL) {\
		ipc_message_empty (pevent->m);\
		free(pevent->m);\
		pevent->m = NULL;\
	}\
};


/***
 * logging functions
 **/

void log_error (const char* message, ...);
void log_info  (const char* message, ...);
void log_debug (const char* message, ...);


/**
 * main public functions
 **/

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

void ipc_connection_print   (struct ipc_connection_info *cinfo);
void ipc_connections_print  (struct ipc_connection_infos *cinfos);
void ipc_connections_close  (struct ipc_connection_infos *cinfos);

// get explanation about an error
const char * ipc_errors_get (enum ipc_errors e);


/***
 * message functions
 **/

uint32_t ipc_message_raw_serialize (char *buffer, char type, char user_type, char * message, uint32_t message_size);

// used to create msg structure from buffer
enum ipc_errors ipc_message_format_read (struct ipc_message *m, const char *buf, size_t msize);
// used to create buffer from msg structure
enum ipc_errors ipc_message_format_write (const struct ipc_message *m, char **buf, size_t *msize);

enum ipc_errors ipc_message_format (struct ipc_message *m, char type, char utype, const char *payload, size_t length);
enum ipc_errors ipc_message_format_data (struct ipc_message *m, char utype, const char *payload, size_t length);
enum ipc_errors ipc_message_format_server_close (struct ipc_message *m);
enum ipc_errors ipc_message_empty (struct ipc_message *m);


/***
 * non public functions
 **/


enum ipc_errors ipc_accept (struct ipc_connection_info *srv, struct ipc_connection_info *p);
enum ipc_errors ipc_contact_networkd (struct ipc_connection_info *srv, const char *sname);
enum ipc_errors service_path (char *path, const char *sname, int32_t index, int32_t version);
char * log_get_logfile_dir (char *buf, size_t size);
void log_get_logfile_name (char *buf, size_t size);


/***
 * networkd enumerations, structures and functions
 **/

struct ipc_switching {
	int orig;
	int dest;
};

struct ipc_switchings {
	struct ipc_switching *collection;
	size_t size;
};

struct networkd {
	int cpt;
	struct ipc_connection_info *srv;
	struct ipc_connection_infos *clients;
	struct ipc_switchings * TCP_TO_IPC;
};

enum ipc_errors ipc_wait_event_networkd (struct ipc_connection_infos *cinfos
        , struct ipc_connection_info *cinfo // NULL for clients
        , struct ipc_event *event
		, struct ipc_switchings *switchdb);


void ipc_switching_add (struct ipc_switchings *is, int orig, int dest);
int ipc_switching_del (struct ipc_switchings *is, int fd);
int ipc_switching_get (struct ipc_switchings *is, int fd);
void ipc_switching_free (struct ipc_switchings *is);
void ipc_switching_print (struct ipc_switchings *is);


enum ipc_errors ipc_receive_fd (int sock, int *fd);
enum ipc_errors ipc_provide_fd (int sock, int fd);

#endif
