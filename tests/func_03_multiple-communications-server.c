#include "../src/ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define SERVICE_NAME "pong"

int main_loop(int argc, char * argv[], char **env)
{
	enum ipc_errors ret;
	SECURE_DECLARATION (struct ipc_connection_info, srv);

	printf ("func 03 - server init...\n");
	ret = ipc_server_init (env, &srv, SERVICE_NAME);
	if (ret != IPC_ERROR_NONE) {
		exit(EXIT_FAILURE);
	}
	printf ("func 03 - server init ok\n");

	SECURE_DECLARATION (struct ipc_connection_infos, clients);
	SECURE_DECLARATION (struct ipc_event, event);

	printf ("func 01 - service polling...\n");
	// listen only for a single client
	while (1) {
		ret = ipc_wait_event (&clients, &srv, &event);

		switch (event.type) {
			case IPC_EVENT_TYPE_CONNECTION : {
					printf ("connection establishment: %d \n", event.origin->fd);
				}
				break;
			case IPC_EVENT_TYPE_DISCONNECTION : {
					printf ("client %d disconnecting\n", event.origin->fd);
				};
				break;
			case IPC_EVENT_TYPE_MESSAGE : {
					printf ("received message: %s\n", ((struct ipc_message*) event.m)->payload);
					ipc_write (event.origin, (struct ipc_message*) event.m);
				}
				break;
			case IPC_EVENT_TYPE_NOT_SET :
			case IPC_EVENT_TYPE_ERROR :
			case IPC_EVENT_TYPE_EXTRA_SOCKET :
			default :
				printf ("not ok - should not happen\n");
				exit (EXIT_FAILURE);
				break;
		}
	}

	printf ("func 03 - closing clients...\n");
	ipc_connections_free (&clients);
	printf ("func 03 - closing server...\n");
	ret = ipc_server_close(&srv);
	if (ret != IPC_ERROR_NONE)
	{
		const char * error_explanation = ipc_errors_get (ret);
		printf ("ipc_server_close: %s\n", error_explanation);
	}

	return 0;
}

void exit_program(int signal)
{
	printf("Quitting, signal: %d\n", signal);
	exit(EXIT_SUCCESS);
}


int main(int argc, char * argv[], char **env)
{
	signal (SIGHUP, exit_program);
	signal (SIGALRM, exit_program);
	signal (SIGUSR1, exit_program);
	signal (SIGUSR2, exit_program);
	signal (SIGTERM, exit_program);
	signal (SIGINT, exit_program);
	main_loop (argc, argv, env);
    return EXIT_SUCCESS;
}
