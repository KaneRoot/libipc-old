#include "../src/ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVICE_NAME "pong"

int main(int argc, char * argv[])
{
	argc = (int) argc;
	argv = (char **) argv;

	SECURE_DECLARATION(struct ipc_error, ret);
	SECURE_DECLARATION(struct ipc_ctx,   ctx);
	SECURE_DECLARATION(struct ipc_event, event);

	ret = ipc_connection (&ctx, SERVICE_NAME, NULL);
	if (ret.error_code != IPC_ERROR_NONE) {
		printf ("error: %s\n", ipc_errors_get(ret.error_code));
		return EXIT_FAILURE;
	}

	// int timer = 10000; // 10 seconds
	// ret = ipc_wait_event (services, struct ipc_event *event, &timer);
	// if (ret.error_code != IPC_ERROR_NONE) {
	// 	return EXIT_FAILURE;
	// }

	ret = ipc_close_all (&ctx);
	if (ret.error_code != IPC_ERROR_NONE) {
		printf ("error: %s\n", ipc_errors_get(ret.error_code));
		return EXIT_FAILURE;
	}
	ipc_ctx_free (&ctx);

    return EXIT_SUCCESS;
}
