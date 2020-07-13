#include "ipc.h"

void ipc_ctx_print (struct ipc_ctx *ctx)
{
	printf ("Context contains:\n");
	for (size_t i = 0; i < ctx->size; i++) {
		printf ("- fd %d\t", ctx->pollfd[i].fd);

		switch (ctx->cinfos[i].type) {
		case IPC_CONNECTION_TYPE_IPC: {
				printf ("- ipc\n");
				break;
			}
		case IPC_CONNECTION_TYPE_EXTERNAL: {
				printf ("- external\n");
				break;
			}
		case IPC_CONNECTION_TYPE_SERVER: {
				printf ("- external\n");
				break;
			}
		case IPC_CONNECTION_TYPE_SWITCHED: {
				printf ("- switched\n");
				break;
			}
		}
	}

	if (ctx->switchdb.size > 0) {
		printf ("Context.switchdb contains:\n");
		for (size_t i = 0; i < ctx->switchdb.size; i++) {
			printf ("- %d <-> %d\n"
				, ctx->switchdb.collection[i].orig
				, ctx->switchdb.collection[i].dest);
		}
	}
	else {
		printf ("Context.switchdb is empty\n");
	}

	if (ctx->tx.size > 0) {
		printf ("Context.tx contains:\n");
		for (size_t i = 0; i < ctx->tx.size; i++) {
			printf ("- message to %d\n", ctx->tx.messages[i].fd);
		}
	}
	else {
		printf ("Context.tx is empty\n");
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
