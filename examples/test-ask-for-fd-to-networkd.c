#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/ipc.h"

int main (int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	SECURE_DECLARATION (struct ipc_error, ret);
	SECURE_DECLARATION (struct ipc_connection_info, srv);

	if (argc != 2) {
		fprintf (stderr, "usage: %s service_name\n", argv[0]);
		exit (1);
	}

	char *service_name = argv[1];

	ret = ipc_contact_networkd (&srv, service_name);

	printf ("ret = %d\n", ret.error_code);

	if (ret.error_code == IPC_ERROR_NONE && srv.fd != 0) {
		printf ("Success\n");
	} else {
		printf ("Ow. :(\n");
	}

	usock_close (srv.fd);

	return EXIT_SUCCESS;
}
