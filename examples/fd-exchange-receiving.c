#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../src/ipc.h"
#include "../src/usocket.h"

// This program receives an open file descriptor from another running program.
// see examples/fd-exchange-providing.c

int main(int argc, char * argv[])
{
	if (argc != 1) {
		fprintf (stderr, "usage: %s", argv[0]);
		exit (EXIT_FAILURE);
	}

	int usock = 0;
	int usockclient = 0;
	int fd = 0;

	TIPC_P_Q (usock_init (&usock, "SOCKET_FD_EXCHANGE_TEST"), "trying to connect to the unix socket", EXIT_FAILURE);
    TIPC_P_Q (usock_accept (usock, &usockclient), "cannot accept a client from the unix socket", EXIT_FAILURE);
	TIPC_P_Q (ipc_receive_fd (usockclient, &fd), "cannot receive the file descriptor", EXIT_FAILURE);

	T_PERROR_R ((write (fd, "coucou\n", 7) < 0), "cannot write a message in the file", EXIT_FAILURE);
	T_PERROR_R ((close (fd) < 0), "cannot close the file descriptor", EXIT_FAILURE);

	TIPC_P_Q (usock_close (usock), "cannot close the unix socket", EXIT_FAILURE);

    return EXIT_SUCCESS;
}
