#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "../src/ipc.h"
#include "../src/utils.h"

/**
 * tcp connection to localhost port argv[1]
 * message 1: pong
 * message 2: echo with libipc format
 *
 *  [messagetype | len | usertype | payload ]
 *   1 B         | 4 B | 1 B      |         ]
 */

int connection (char *ipstr, int port)
{
	int sockfd;
	SECURE_DECLARATION (struct sockaddr_in, server);
	socklen_t addrlen;

	// socket factory
	T_PERROR_Q (((sockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1), "socket creation", EXIT_FAILURE);

	// init remote addr structure and other params
	server.sin_family = AF_INET;
	server.sin_port = htons (port);
	addrlen = sizeof (struct sockaddr_in);

	// get addr from command line and convert it
	T_PERROR_Q ((inet_pton (AF_INET, ipstr, &server.sin_addr) <= 0), "inet_pton", EXIT_FAILURE);

	printf ("Trying to connect to the remote host\n");

	T_PERROR_Q ((connect (sockfd, (struct sockaddr *)&server, addrlen) == -1), "connection", EXIT_FAILURE);

	printf ("Connection OK\n");

	return sockfd;
}

void send_receive (int sockfd)
{
	SECURE_BUFFER_DECLARATION (unsigned char, buf, BUFSIZ);
	int paylen;

	// first, send service name "pong"
	// send string
	T_PERROR_Q ((send (sockfd, "pong", 4, 0) == -1), "sending a message", EXIT_FAILURE);
	printf ("message 'pong' sent\n");

	T_PERROR_Q (((paylen = recv (sockfd, buf, BUFSIZ, 0)) <= 0), "cannot connect to networkd", EXIT_FAILURE);
	print_hexa ("should be 'OK'", buf, paylen);
	memset (buf, 0, BUFSIZ);

	// 2    | 6    | 0 | "coucou"
	// 1 B  | 4 B  | 1 | 6 B
	ipc_message_raw_serialize ((char *)buf, MSG_TYPE_DATA, 42, "coucou", 6);
	print_hexa ("WAITING 10 seconds then message to send", buf, 12);
	// sleep (1);
	T_PERROR_Q ((send (sockfd, buf, 12, 0) == -1), "sending a message", EXIT_FAILURE);
	printf ("message 'coucou' sent\n");
	memset (buf, 0, BUFSIZ);

	// receiving a message
	T_PERROR_Q (((paylen = recv (sockfd, buf, BUFSIZ, 0)) < 0), "receiving a message", EXIT_FAILURE);

	if (paylen == 0) {
		fprintf (stderr, "error: disconnection from the server\n");
		exit (EXIT_FAILURE);
	}

	print_hexa ("RECEIVED MESSAGE", buf, paylen);
}

int main (int argc, char *argv[])
{
	char *ipstr = "127.0.0.1";
	int port = 9000;
	if (argc == 2) {
		port = atoi (argv[1]);
	} else if (argc == 3) {
		ipstr = argv[1];
		port = atoi (argv[2]);
	}

	int sockfd = connection (ipstr, port);

	send_receive (sockfd);

	printf ("Disconnection\n");

	// close the socket
	close (sockfd);

	return EXIT_SUCCESS;
}
