#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/ipc.h"

int main (int argc, char *argv[])
{
	if (argc != 2) {
		fprintf (stderr, "usage: %s service_name\n", argv[0]);
		exit (1);
	}

	SECURE_DECLARATION (struct ipc_error, ret);
	int fd = 0;

	char *service_name = argv[1];

	ret = ipc_contact_ipcd (&fd, service_name);

	printf ("ret = %d\n", ret.error_code);

	if (ret.error_code == IPC_ERROR_NONE && fd > 0) {
		printf ("Success\n");
	} else {
		printf ("Ow. :(\n");
	}

	usock_close (fd);

	return EXIT_SUCCESS;
}
