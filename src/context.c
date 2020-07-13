#include "ipc.h"

/**
 * PERFORMANCE POINT:
 *   Realloc is performed at each new user. There is plenty of room for improvement,
 *   for example by managing allocations of thousands of structures at once.
 * WARNING: Store and remove only pointers on allocated structures.
 */
struct ipc_error ipc_ctx_new_alloc (struct ipc_ctx *ctx)
{
	ctx->size++;

	// Memory could be not allocated, yet.
	if (ctx->size == 1 && ctx->cinfos == NULL && ctx->pollfd == NULL) {
		SECURE_BUFFER_HEAP_ALLOCATION_R (ctx->cinfos, sizeof (struct ipc_connection_info),,
						IPC_ERROR_ADD__MALLOC);
		SECURE_BUFFER_HEAP_ALLOCATION_R (ctx->pollfd, sizeof (struct pollfd),, IPC_ERROR_ADD__MALLOC_POLLFD);
	} else {
		ctx->cinfos = realloc (ctx->cinfos, sizeof (struct ipc_connection_info) * ctx->size);
		ctx->pollfd = realloc (ctx->pollfd, sizeof (struct pollfd             ) * ctx->size);
	}

	T_R ((ctx->cinfos == NULL), IPC_ERROR_ADD__EMPTY_LIST);
	T_R ((ctx->pollfd == NULL), IPC_ERROR_ADD__EMPTY_LIST);

	// Clean the last entry.
	memset (&ctx->cinfos[ctx->size -1], 0, sizeof (struct ipc_connection_info));
	memset (&ctx->pollfd[ctx->size -1], 0, sizeof (struct pollfd));

	IPC_RETURN_NO_ERROR;
}

void ipc_ctx_free (struct ipc_ctx *ctx)
{
	if (ctx->cinfos != NULL) {
		free (ctx->cinfos);
		ctx->cinfos = NULL;
	}
	if (ctx->pollfd != NULL) {
		free (ctx->pollfd);
		ctx->pollfd = NULL;
	}
	ctx->size = 0;

	ipc_switching_free(&ctx->switchdb);

	ipc_messages_free (&ctx->tx);
}
