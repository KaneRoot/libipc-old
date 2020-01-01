#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/ipc.h"

int main (int argc, char *argv[], char *env[])
{
	(void)argc;
	(void)argv;

	SECURE_DECLARATION (struct ipc_error, ret);
	SECURE_DECLARATION (struct ipc_connection_info, srv);
	SECURE_DECLARATION (struct ipc_connection_info, client);
	SECURE_DECLARATION (struct ipc_connection_info, contacted_service);

	// service start
	TEST_IPC_Q (ipc_server_init (env, &srv, "network"), EXIT_FAILURE);

	printf ("service initialized, waiting for a client\n");

	// accept a new client
	TEST_IPC_Q (ipc_accept (&srv, &client), EXIT_FAILURE);

	// TODO: read a message to know the requested service
	SECURE_DECLARATION (struct ipc_message, msg);
	TEST_IPC_Q (ipc_read (&client, &msg), EXIT_FAILURE);
	printf ("received message: %s\n", msg.payload);

	/** TODO: contact the service */
	printf ("WARNING: currently this program only ask for pong service %d\n", ret.error_code);
	TEST_IPC_Q (ipc_connection (env, &contacted_service, "pong"), EXIT_FAILURE);

	ipc_provide_fd (client.fd, contacted_service.fd);

	TEST_IPC_Q (ipc_server_close (&srv), EXIT_FAILURE);
	return EXIT_SUCCESS;
}
