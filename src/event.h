#ifndef __IPC_EVENT__
#define __IPC_EVENT__

#include "ipc.h"
#include "message.h"

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
