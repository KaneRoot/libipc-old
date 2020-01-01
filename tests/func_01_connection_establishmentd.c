#include "../src/ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVICE_NAME "pong"

int main(int argc, char * argv[], char **env)
{
	argc = (int) argc;
	argv = (char **) argv;

	SECURE_DECLARATION(struct ipc_connection_info,srv);
	long timer = 10;

	printf ("func 01 - server init...\n");
	TEST_IPC_Q(ipc_server_init (env, &srv, SERVICE_NAME), EXIT_FAILURE);
	
	printf ("func 01 - server init ok\n");
	SECURE_DECLARATION(struct ipc_connection_infos, clients);
	SECURE_DECLARATION(struct ipc_event,event);

	printf ("func 01 - service polling...\n");

	// listen only for a single client
	TEST_IPC_Q(ipc_wait_event (&clients, &srv, &event, &timer), EXIT_FAILURE);

	switch (event.type) {
		case IPC_EVENT_TYPE_TIMER : {
				fprintf(stderr, "time up!\n");

				timer = 10;
			};
			break;
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
	TEST_IPC_Q(ipc_server_close(&srv), EXIT_FAILURE);

    return EXIT_SUCCESS;
}
