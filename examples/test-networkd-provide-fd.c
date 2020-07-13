#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/ipc.h"

int main (void)
{
	SECURE_DECLARATION (struct ipc_error, ret);
	SECURE_DECLARATION (struct ipc_ctx, ctx);
	SECURE_DECLARATION (struct ipc_event, event);

	// service start
	TEST_IPC_Q (ipc_server_init (&ctx, "ipcd"), EXIT_FAILURE);

	printf ("service initialized, waiting for a client\n");

	// accept a new client
	TEST_IPC_Q (ipc_accept_add (&event, &ctx, 0 /* the only valid index right now */), EXIT_FAILURE);
	int client_fd = ctx.pollfd[ctx.size-1].fd;

	// TODO: read a message to know the requested service
	SECURE_DECLARATION (struct ipc_message, msg);
	TEST_IPC_Q (ipc_read (&ctx, 1 /* 1 = our client */, &msg), EXIT_FAILURE);
	printf ("received message: %s\n", msg.payload);

	/** TODO: contact the service */
	printf ("WARNING: currently this program only ask for pong service %d\n", ret.error_code);
	TEST_IPC_Q (ipc_connection (&ctx, "pong", NULL), EXIT_FAILURE);

	ipc_provide_fd (client_fd, ctx.pollfd[ctx.size-1].fd);

	TEST_IPC_Q (ipc_close_all (&ctx), EXIT_FAILURE);
	return EXIT_SUCCESS;
}
