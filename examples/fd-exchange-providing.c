#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../src/ipc.h"
#include "../src/usocket.h"

// This program opens a file then provide it to another running program.
// see examples/fd-exchange-receiving.c

int main(int argc, char * argv[])
{
	if (argc != 2) {
		fprintf (stderr, "usage: %s file", argv[0]);
		exit (EXIT_FAILURE);
	}

	int sock = 0;
	int fd = 0;

	T_PERROR_R (((fd = open (argv[1], O_CREAT | O_RDWR)) < 0), "cannot open the file", EXIT_FAILURE);

	TIPC_P_Q (usock_connect (&sock, "SOCKET_FD_EXCHANGE_TEST"), "trying to connect to the unix socket", EXIT_FAILURE);
	TIPC_P_Q (ipc_provide_fd (sock, fd), "cannot send the file descriptor", EXIT_FAILURE);

    return EXIT_SUCCESS;
}
