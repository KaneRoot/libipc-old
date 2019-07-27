#include "../src/ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVICE_NAME "pong"

int main(int argc, char * argv[], char **env)
{
	enum ipc_errors ret;
	SECURE_DECLARATION(struct ipc_connection_info,service);
	SECURE_DECLARATION(struct ipc_event, event);
	
	ret = ipc_connection (env, &service, SERVICE_NAME);
	if (ret != IPC_ERROR_NONE) {
		return EXIT_FAILURE;
	}

	// ret = ipc_wait_event (services, struct ipc_event *event);
	// if (ret != IPC_ERROR_NONE) {
	// 	return EXIT_FAILURE;
	// }

	ret = ipc_close (&service);
	if (ret != IPC_ERROR_NONE) {
		return EXIT_FAILURE;
	}

    return EXIT_SUCCESS;
}
