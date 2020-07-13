#include "../src/ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVICE_NAME "example"

int main(int argc, char * argv[])
{
	char path[PATH_MAX];
	char * sname = SERVICE_NAME;

	if (argc == 2) {
		sname = argv[1];
	}
	else if (argc != 1) {
		fprintf (stderr, "usage: %s [service-name index version]\n", argv[0]);
		return EXIT_FAILURE;
	}

	service_path (path, sname);

	// printf ("servicename: %s, index: %d, version: %d\n", sname, index, version);
	printf ("%s\n", path);

	return EXIT_SUCCESS;
}
