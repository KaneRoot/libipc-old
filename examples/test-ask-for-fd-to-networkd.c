#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/ipc.h"

#define DEFAULT_IPC_NETWORK "IPC_NETWORK=\"pong local:lolwat\""

int main(int argc, char * argv[])
{

	(void) argc;
	(void) argv;

	enum ipc_errors ret;
	SECURE_DECLARATION (struct ipc_connection_info, srv);

	if (argc != 2) {
		fprintf (stderr, "usage: %s service_name\n", argv[0]);
		exit (1);
	}

	char *service_name = argv[1];

	// ask for a local service "pong"
	// inform the network service that it's now named "lolwat"
	char *ipc_network = getenv("IPC_NETWORK");
	if (ipc_network == NULL) {
		ipc_network = DEFAULT_IPC_NETWORK;
	}

	ret = ipc_contact_networkd (&srv, service_name, ipc_network);

	printf ("ret = %d\n", ret);

	if (ret == 0 && srv.fd != 0) {
		printf ("Success\n");
	}
	else {
		printf ("Ow. :(\n");
	}

	usock_close (srv.fd);

    return EXIT_SUCCESS;
}
