libipc(7)

# NAME

libipc - Simple, easy-to-use IPC library

# DESCRIPTION

**libipc** is a library that provides interprocess communication medium between applications.
It provides both client and server code.

# SYNOPSIS

**#include <ipc.h>**

## Initialization, exchanges, disconnection

// server initialization

_enum ipc_errors_ **ipc_server_init** (*char* \*\*env , *struct ipc_connection_info* \*srv, *const char* \*sname);

// connection establishement to a server

_enum ipc_errors_ **ipc_connection** (*char* \*\*env, *struct ipc_connection_info* \*, *const char* \*);

// closing a server

_enum ipc_errors_ **ipc_server_close** (*struct ipc_connection_info* \*srv);

// closing a connection

_enum ipc_errors_ **ipc_close** (*struct ipc_connection_info* \*p);++
_enum ipc_errors_ **ipc_accept** (*struct ipc_connection_info* \*srv, *struct ipc_connection_info* \*p);

_enum ipc_errors_ **ipc_read** (*const struct ipc_connection_info* \*, *struct ipc_message* \*m);++
_enum ipc_errors_ **ipc_write** (*const struct ipc_connection_info* \*, *const struct ipc_message* \*m);++
_enum ipc_errors_ **ipc_wait_event** (*struct ipc_ctx* \*clients, *struct ipc_connection_info* \*srv, *struct ipc_event* \*event);


// store and remove only pointers on allocated structures

_enum ipc_errors_ **ipc_add** (*struct ipc_ctx* \*cinfos, *struct ipc_connection_info* \*cinfo);++
_enum ipc_errors_ **ipc_del** (*struct ipc_ctx* \*cinfos, *struct ipc_connection_info* \*cinfo);

// add an arbitrary file descriptor to read

_enum ipc_errors_ **ipc_add_fd** (*struct ipc_ctx* \*cinfos, *int* fd);


## Message functions

// create msg structure from buffer

_enum ipc_errors_ **ipc_message_format_read** (*struct ipc_message* \*m, *const char* \*buf, *ssize_t* msize);

// create buffer from msg structure

_enum ipc_errors_ **ipc_message_format_write** (*const struct ipc_message* \*m, *char* \*\*buf, *ssize_t* \*msize);

_enum ipc_errors_ **ipc_message_format** (*struct ipc_message* \*m, *char* type, *const char* \*payload, *ssize_t* length);++
_enum ipc_errors_ **ipc_message_format_data** (*struct ipc_message* \*m, *const char* \*payload, *ssize_t* length);++
_enum ipc_errors_ **ipc_message_format_server_close** (*struct ipc_message* \*m);

_enum ipc_errors_ **ipc_message_empty** (*struct ipc_message* \*m);


# STRUCTURES

```
	struct ipc_connection_info {
		uint32_t version;
		uint32_t index;
		int32_t fd;
		char type;   // may be an arbitrary fd
		char *spath; // max size: PATH_MAX, used to store unix socket path
	};

	struct ipc_ctx {
		struct ipc_connection_info ** cinfos;
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
```


# ENUMERATIONS

```
	enum msg_types {
		MSG_TYPE_SERVER_CLOSE = 0
		, MSG_TYPE_ERR
		, MSG_TYPE_DATA
	} message_types;
```

Function **ipc_wait_event** returns an *event type* structure.\
The event may be a (dis)connection, received data or an error.\
It also can be *IPC_EVENT_TYPE_EXTRA_SOCKET* since an arbitrary file descriptor can be added to the *ipc_ctx* structure with **ipc_add_fd**.

```
	enum ipc_event_type {
		IPC_EVENT_TYPE_NOT_SET
		, IPC_EVENT_TYPE_ERROR
		, IPC_EVENT_TYPE_EXTRA_SOCKET
		, IPC_EVENT_TYPE_CONNECTION
		, IPC_EVENT_TYPE_DISCONNECTION
		, IPC_EVENT_TYPE_MESSAGE
	};

	enum ipc_errors {
		...
	};
```


# EXAMPLES

Examples are available in the */examples* directory.

# NOTES

# SEE ALSO

# BUGS & LIMITATIONS

- Documentation is currently limited.
- Tests are currently limited.
- No code audit has been made.

