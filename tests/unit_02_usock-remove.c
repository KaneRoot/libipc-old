#include "../src/ipc.h"
#include "../src/usocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_TO_REMOVE "/tmp/FILE_TO_REMOVE"

int main(int argc, char * argv[])
{
	char * ftr = FILE_TO_REMOVE;

	if (argc == 2) {
		ftr = argv[1];
	}
	else if (argc > 2) {
		fprintf (stderr, "usage: %s [file-to-remove]\n", argv[0]);
		return EXIT_FAILURE;
	}

	enum ipc_errors ret = usock_remove (ftr);
	if (ret == IPC_ERROR_NONE)
		return 0;
	return 1;
}
