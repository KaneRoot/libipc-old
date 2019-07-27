#include "../src/ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVICE_NAME "example"
#define VERSION		0
#define INDEX		0

int main(int argc, char * argv[])
{
	char path[PATH_MAX];
	char * sname = SERVICE_NAME;
	int32_t index = INDEX;
	int32_t version = VERSION;

	if (argc == 4) {
		sname = argv[1];
		index = atoi(argv[2]);
		version = atoi(argv[3]);
	}
	else if (argc != 1) {
		fprintf (stderr, "usage: %s [service-name index version]\n", argv[0]);
		return EXIT_FAILURE;
	}

	service_path (path, sname, index, version);

	// printf ("servicename: %s, index: %d, version: %d\n", sname, index, version);
	printf ("%s\n", path);

	return EXIT_SUCCESS;
}
