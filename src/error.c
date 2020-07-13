#include "ipc.h"

#define NTAB(t)     ((int) (sizeof (t) / sizeof (t)[0]))

struct ipc_errors_verbose {
	enum ipc_error_code error_code;
	char *explanation_string;
};

static struct ipc_errors_verbose ipc_errors_verbose[] = {

	{IPC_ERROR_NONE, "no error"}
	, {IPC_ERROR_CLOSED_RECIPIENT, "closed recipient"}

	, {IPC_ERROR_SERVER_INIT__NO_ENVIRONMENT_PARAM, "ipc_server_init: no environment param"}
	, {IPC_ERROR_SERVER_INIT__NO_SERVICE_PARAM    , "ipc_server_init: no service param"}
	, {IPC_ERROR_SERVER_INIT__NO_SERVER_NAME_PARAM, "ipc_server_init: no server name param"}
	, {IPC_ERROR_SERVER_INIT__MALLOC              , "ipc_server_init: error on malloc function"}

	, {IPC_ERROR_CONNECTION__NO_SERVICE_NAME, "ipc_connection: no service name parameter"}
	, {IPC_ERROR_CONNECTION__NO_ENVIRONMENT_PARAM, "ipc_connection: no environment param"}
	, {IPC_ERROR_USOCK_CONNECT__CONNECT, "ipc_connection: error on the connect function"}

	, {IPC_ERROR_ACCEPT__NO_SERVICE_PARAM, "ipc_accept: no service param"}
	, {IPC_ERROR_ACCEPT__NO_CLIENT_PARAM , "ipc_accept: no client param"}

	, {IPC_ERROR_RECEIVE_FD__RECVMSG    , "ipc_receive_fd: recvmsg function"}
	, {IPC_ERROR_RECEIVE_FD__NO_PARAM_FD, "ipc_receive_fd: no fd param"}
	, {IPC_ERROR_PROVIDE_FD__SENDMSG    , "ipc_provide_fd: sendmsg function"}

	, {IPC_ERROR_WRITE__NO_MESSAGE_PARAM, "ipc_write: no message param"}
	, {IPC_ERROR_WRITE_FD__NOT_ENOUGH_DATA , "ipc_write: no enough data sent"}
	, {IPC_ERROR_READ__NO_MESSAGE_PARAM , "ipc_read: no message param"}

	, {IPC_ERROR_HANDLE_MESSAGE__NOT_ENOUGH_MEMORY     , "handle_message: not enough memory"}
	, {IPC_ERROR_HANDLE_NEW_CONNECTION__NO_CINFO_PARAM , "ipc_handle_new_connection: no cinfo param"}
	, {IPC_ERROR_HANDLE_NEW_CONNECTION__NO_CINFOS_PARAM, "ipc_handle_new_connection: no cinfos param"}

	, {IPC_ERROR_WAIT_EVENT__SELECT          , "ipc_wait_event: error on the select function"}
	, {IPC_ERROR_WAIT_EVENT__NO_CLIENTS_PARAM, "ipc_wait_event: no clients param"}
	, {IPC_ERROR_WAIT_EVENT__NO_EVENT_PARAM  , "ipc_wait_event: no event param"}

	, {IPC_ERROR_CONTACT_IPCD__NO_SERVICE_NAME_PARAM, "ipc_contact_ipcd: no service name param"}
	, {IPC_ERROR_CONTACT_IPCD__NO_SERVER_PARAM      , "ipc_contact_ipcd: no server param"}

	, {IPC_ERROR_HANDLE_NEW_CONNECTION__MALLOC, "ipc_handle_new_connection: error on malloc function"}

	, {IPC_ERROR_ADD__MALLOC              , "ipc_add: first memory allocation failed"}
	, {IPC_ERROR_ADD__EMPTY_LIST          , "ipc_add: empty list: realloc failed"}
	, {IPC_ERROR_ADD__NO_PARAM_CLIENTS    , "ipc_add: no param client list"}
	, {IPC_ERROR_ADD__NO_PARAM_CLIENT     , "ipc_add: no param client"}
	, {IPC_ERROR_ADD_FD__NOT_ENOUGH_MEMORY, "ipc_add_fd: not enough memory"}

	, {IPC_ERROR_ADD_FD__NO_PARAM_CINFOS, "ipc_add_fd: no cinfos param"}

	, {IPC_ERROR_DEL_FD__NO_PARAM_CINFOS   , "ipc_del_fd: no cinfos param"}
	, {IPC_ERROR_DEL_FD__EMPTIED_LIST      , "ipc_del_fd: empty list after realloc (memory problem)"}
	, {IPC_ERROR_DEL_FD__EMPTY_LIST        , "ipc_del_fd: empty list"}
	, {IPC_ERROR_DEL_FD__CANNOT_FIND_CLIENT, "ipc_del_fd: cannot find user"}

	, {IPC_ERROR_DEL__EMPTY_LIST        , "ipc_del: empty list"}
	, {IPC_ERROR_DEL__EMPTIED_LIST      , "ipc_del: cannot realloc"}
	, {IPC_ERROR_DEL__CANNOT_FIND_CLIENT, "ipc_del: cannot find client"}
	, {IPC_ERROR_DEL__NO_CLIENTS_PARAM  , "ipc_del: no clients param"}
	, {IPC_ERROR_DEL__NO_CLIENT_PARAM   , "ipc_del: no client param"}

	, {IPC_ERROR_USOCK_SEND, "usock_send: cannot send message"}

	, {IPC_ERROR_USOCK_CONNECT__SOCKET               , "usock_connect: error on socket function"}
	, {IPC_ERROR_USOCK_CONNECT__WRONG_FILE_DESCRIPTOR, "usock_connect: wrong file descriptor"}
	, {IPC_ERROR_USOCK_CONNECT__EMPTY_PATH           , "usock_connect: empty path"}

	, {IPC_ERROR_USOCK_CLOSE, "usock_close: close function"}

	, {IPC_ERROR_USOCK_REMOVE__UNLINK , "usock_remove: unlink function"}
	, {IPC_ERROR_USOCK_REMOVE__NO_FILE, "usock_remove: file not found"}

	, {IPC_ERROR_USOCK_INIT__EMPTY_FILE_DESCRIPTOR, "usock_init: no file descriptor"}
	, {IPC_ERROR_USOCK_INIT__WRONG_FILE_DESCRIPTOR, "usock_init: wrong file descriptor"}
	, {IPC_ERROR_USOCK_INIT__EMPTY_PATH           , "usock_init: empty path"}
	, {IPC_ERROR_USOCK_INIT__BIND                 , "usock_init: error on bind function"}
	, {IPC_ERROR_USOCK_INIT__LISTEN               , "usock_init: error on listen function"}

	, {IPC_ERROR_USOCK_ACCEPT__PATH_FILE_DESCRIPTOR, "ipc_usock_accept: no path file descriptor"}
	, {IPC_ERROR_USOCK_ACCEPT                      , "ipc_usock_accept: error on accept function"}

	, {IPC_ERROR_USOCK_RECV__NO_BUFFER         , "ipc_usock_recv: no buffer in usock_recv"}
	, {IPC_ERROR_USOCK_RECV__NO_LENGTH         , "ipc_usock_recv: no length in usock_recv"}
	, {IPC_ERROR_USOCK_RECV                    , "ipc_usock_recv: cannot receive message in usock_recv"}
	, {IPC_ERROR_USOCK_RECV__UNRECOGNIZED_ERROR, "ipc_usock_recv: unrecognized error during recv(2)"}
	, {IPC_ERROR_USOCK_RECV__MESSAGE_SIZE      , "ipc_usock_recv: message length > maximum allowed"}
	, {IPC_ERROR_USOCK_RECV__HEAP_ALLOCATION   , "ipc_usock_recv: heap allocation failed"}

	, {IPC_ERROR_MESSAGE_FORMAT_WRITE__MESSAGE_LENGTH, "ipc_message_write: message is longer than accepted"}
	, {IPC_ERROR_MESSAGE_FORMAT__NO_MESSAGE_PARAM    , "ipc_message_format: no message param"}
	, {IPC_ERROR_MESSAGE_FORMAT__INCONSISTENT_PARAMS , "ipc_message_format: inconsistent params"}
	, {IPC_ERROR_MESSAGE_FORMAT__MESSAGE_SIZE        , "ipc_message_format: length param > maximum allowed"}
	, {IPC_ERROR_MESSAGE_FORMAT__HEAP_ALLOCATION     , "ipc_message_format: heap allocation failed"}

	, {IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_MESSAGE, "ipc_message_format_write: empty message"}
	, {IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_MSIZE, "ipc_message_format_write: empty message size"}
	, {IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_BUFFER, "ipc_message_format_write: empty buffer"}

	, {IPC_ERROR_MESSAGE_FORMAT_READ__EMPTY_MESSAGE, "ipc_message_format_read: empty message"}
	, {IPC_ERROR_MESSAGE_FORMAT_READ__EMPTY_BUFFER , "ipc_message_format_read: empty buffer"}
	, {IPC_ERROR_MESSAGE_FORMAT_READ__PARAM_MESSAGE_SIZE,
		"ipc_message_format_read: said buffer size (in parameter) > maximum allowed"}
	, {IPC_ERROR_MESSAGE_FORMAT_READ__READ_MESSAGE_SIZE, "ipc_message_format: read message size is not correct"}
	, {IPC_ERROR_MESSAGE_FORMAT_READ__MESSAGE_TOO_LONG , "ipc_message_format: read message size > maximum allowed"}

	, {IPC_ERROR_MESSAGE_EMPTY__EMPTY_MESSAGE_LIST, "ipc_message_empty: empty message list"}

	, {IPC_ERROR_SERVICE_PATH__NO_PATH        , "ipc_service_path: no path"}
	, {IPC_ERROR_SERVICE_PATH__NO_SERVICE_NAME, "ipc_service_path: no service name"}

	, {IPC_ERROR_MKDIR__CANNOT_CREATE_DIR, "mkdir_p_: cannot create a directory"}
	, {IPC_ERROR_MKDIR__NAME_TOO_LONG    , "mkdir_p_: path parameter has a too long name"}

	, {IPC_ERROR_DIR_SETUP__NOT_A_DIRECTORY, "directory_setup_: path cannot be created"}

	, {IPC_ERROR_DIR_SETUP__DIRECTORY_NOT_WRITABLE, "directory_setup_: directory not writable"}
	, {IPC_ERROR_DIRECTORY_SETUP__PATH_PARAM      , "directory_setup_: no path param"}

	, {IPC_ERROR_SERVER_INIT__NOT_ENOUGH_MEMORY,            "IPC_ERROR_SERVER_INIT__NOT_ENOUGH_MEMORY"}
	, {IPC_ERROR_CONNECTION__NOT_ENOUGH_MEMORY,             "IPC_ERROR_CONNECTION__NOT_ENOUGH_MEMORY"}
	, {IPC_ERROR_CTX_INIT__NO_CONTEXT_PARAM,                "IPC_ERROR_CTX_INIT__NO_CONTEXT_PARAM"}
	, {IPC_ERROR_CTX_INIT__CONTEXT_ALREADY_INIT,            "IPC_ERROR_CTX_INIT__CONTEXT_ALREADY_INIT"}
	, {IPC_ERROR_ADD__MALLOC_POLLFD,                        "IPC_ERROR_ADD__MALLOC_POLLFD"}
	, {IPC_ERROR_ADD_MESSAGE_TO_SEND__EMPTY_LIST,           "IPC_ERROR_ADD_MESSAGE_TO_SEND__EMPTY_LIST"}
	, {IPC_ERROR_ADD_MESSAGE_TO_SEND__MALLOC,               "IPC_ERROR_ADD_MESSAGE_TO_SEND__MALLOC"}
	, {IPC_ERROR_ADD_MESSAGE_TO_SEND__NO_PARAM_MESSAGE,     "IPC_ERROR_ADD_MESSAGE_TO_SEND__NO_PARAM_MESSAGE"}
	, {IPC_ERROR_ADD_MESSAGE_TO_SEND__NO_PARAM_MESSAGES,    "IPC_ERROR_ADD_MESSAGE_TO_SEND__NO_PARAM_MESSAGES"}
	, {IPC_ERROR_CONNECTION__NO_CTX,                        "IPC_ERROR_CONNECTION__NO_CTX"}
	, {IPC_ERROR_CTX_INIT__MALLOC_CTX,                      "IPC_ERROR_CTX_INIT__MALLOC_CTX"}
	, {IPC_ERROR_CTX_INIT__MALLOC_POLLFD,                   "IPC_ERROR_CTX_INIT__MALLOC_POLLFD"}
	, {IPC_ERROR_CONTACT_IPCD__NO_FD_PARAM,             "IPC_ERROR_CONTACT_IPCD__NO_FD_PARAM"}
	, {IPC_ERROR_HANDLE_NEW_CONNECTION__INCONSISTENT_INDEX, "IPC_ERROR_HANDLE_NEW_CONNECTION__INCONSISTENT_INDEX"}
	, {IPC_ERROR_DEL_MESSAGE_TO_SEND__NO_PARAM_MESSAGES,    "IPC_ERROR_DEL_MESSAGE_TO_SEND__NO_PARAM_MESSAGES"}
	, {IPC_ERROR_MESSAGE_DEL__INDEX_ERROR,                  "IPC_ERROR_MESSAGE_DEL__INDEX_ERROR"}
	, {IPC_ERROR_MESSAGE_DEL__EMPTY_LIST,                   "IPC_ERROR_MESSAGE_DEL__EMPTY_LIST"}
	, {IPC_ERROR_ADD__NO_PARAM_POLLFD, "IPC_ERROR_ADD__NO_PARAM_POLLFD"}
	, {IPC_ERROR_WRITE__FD_NOT_FOUND, "IPC_ERROR_WRITE__FD_NOT_FOUND"}
	, {IPC_ERROR_FD_SWITCHING__NO_FD_RECORD, "IPC_ERROR_FD_SWITCHING__NO_FD_RECORD"}
	, {IPC_ERROR_CLOSE_ALL__NO_CTX_PARAM, "IPC_ERROR_CLOSE_ALL__NO_CTX_PARAM" }
	, {IPC_ERROR_CLOSE__NO_CTX_PARAM, "IPC_ERROR_CLOSE__NO_CTX_PARAM"}
};

const char *ipc_errors_get (enum ipc_error_code e)
{
	for (int i = 0; i < NTAB (ipc_errors_verbose); i++) {
		if (ipc_errors_verbose[i].error_code == e) {
			return ipc_errors_verbose[i].explanation_string;
		}
	}

	return NULL;
}
