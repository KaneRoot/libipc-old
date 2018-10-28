#ifndef __IPC_EVENT__
#define __IPC_EVENT__

#include "message.h"

enum ipc_event_type {
	IPC_EVENT_TYPE_NOT_SET
	, IPC_EVENT_TYPE_ERROR
	, IPC_EVENT_TYPE_STDIN
	, IPC_EVENT_TYPE_CONNECTION
	, IPC_EVENT_TYPE_DISCONNECTION
	, IPC_EVENT_TYPE_MESSAGE
};

struct ipc_event {
	enum ipc_event_type type;
	void* origin; // currently used as an client or service pointer
	void* m; // message pointer
};

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

#endif
