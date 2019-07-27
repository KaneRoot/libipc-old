#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/ipc.h"


int main(int argc, char * argv[], char *env[])
{

	(void) argc;
	(void) argv;

	enum ipc_errors ret;
	SECURE_DECLARATION (struct ipc_connection_info, srv);
	SECURE_DECLARATION (struct ipc_connection_info, client);
	SECURE_DECLARATION (struct ipc_connection_info, contacted_service);


	// service start
	TIPC_P_RR (ipc_server_init (env, &srv, "network"), "Networkd cannot be initialized");

	printf ("service initialized, waiting for a client\n");

	// accept a new client
	TIPC_P_RR (ipc_accept (&srv, &client), "cannot accept the client during handle_new_connection");

	// TODO: read a message to know the requested service
	SECURE_DECLARATION (struct ipc_message, msg);
    ret = ipc_read (&client, &msg);
	printf ("received message: %s\n", msg.payload);

	/** TODO: contact the service */
	printf ("WARNING: currently this program only ask for pong service %d\n", ret);
	TIPC_P_RR (ipc_connection (env, &contacted_service, "pong"), "cannot connect to the requested service");

	ipc_provide_fd (client.fd, contacted_service.fd);

	TIPC_P_RR (ipc_server_close (&srv), "Networkd cannot be stopped!!");
    return EXIT_SUCCESS;
}
