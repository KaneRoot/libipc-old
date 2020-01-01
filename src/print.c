#include "ipc.h"

void ipc_connection_print (struct ipc_connection_info *cinfo)
{
	T_R_NOTHING ((cinfo == NULL));

#if 0
	LOG_DEBUG ("fd %d: index %d, version %d, type %c, path %s"
		, cinfo->fd , cinfo->index, cinfo->version, cinfo->type
		, (cinfo->spath == NULL) ? "-" : cinfo->spath);
#endif
}

void ipc_connections_print (struct ipc_connection_infos *cinfos)
{
	for (size_t i = 0; i < cinfos->size; i++) {
		ipc_connection_print (cinfos->cinfos[i]);
	}
}

void ipc_message_print (const struct ipc_message *m)
{
	if (m == NULL)
		return;

#if defined(IPC_WITH_ERRORS) && IPC_WITH_ERRORS > 2
	LOG_INFO ("msg: type %d len %d\n", m->type, m->length);
#endif
}

void ipc_switching_print (struct ipc_switchings *is)
{
	for (size_t i = 0; i < is->size; i++) {
		printf ("client %d - %d", is->collection[i].orig, is->collection[i].dest);
	}
}
