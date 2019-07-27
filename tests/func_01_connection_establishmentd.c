#include "../src/ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVICE_NAME "pong"

int main(int argc, char * argv[], char **env)
{
	enum ipc_errors ret;
	SECURE_DECLARATION(struct ipc_connection_info,srv);

	printf ("func 01 - server init...\n");
	ret = ipc_server_init (env, &srv, SERVICE_NAME);
	if (ret != IPC_ERROR_NONE) {
		return EXIT_FAILURE;
	}
	printf ("func 01 - server init ok\n");

	SECURE_DECLARATION(struct ipc_connection_infos, clients);
	SECURE_DECLARATION(struct ipc_event,event);

	printf ("func 01 - service polling...\n");
	// listen only for a single client
	ret = ipc_wait_event (&clients, &srv, &event);

	switch (event.type) {
		case IPC_EVENT_TYPE_CONNECTION :
			{
				printf ("ok - connection establishment\n");
				break;
			}
		case IPC_EVENT_TYPE_NOT_SET :
		case IPC_EVENT_TYPE_ERROR :
		case IPC_EVENT_TYPE_EXTRA_SOCKET :
		case IPC_EVENT_TYPE_DISCONNECTION :
		case IPC_EVENT_TYPE_MESSAGE :
		default :
			printf ("not ok - should not happen\n");
			exit (EXIT_FAILURE);
			break;
	}

	printf ("func 01 - closing clients...\n");
	ipc_connections_free (&clients);
	printf ("func 01 - closing server...\n");
	ret = ipc_server_close(&srv);
	if (ret != IPC_ERROR_NONE)
	{
		const char * error_explanation = ipc_errors_get (ret);
		printf ("ipc_server_close: %s\n", error_explanation);
	}

    return EXIT_SUCCESS;
}
