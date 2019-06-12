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


int connection(int port)
{
	int sockfd;
	struct sockaddr_in server;
	socklen_t addrlen;

	// socket factory
	if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// init remote addr structure and other params
	server.sin_family = AF_INET;
	server.sin_port   = htons(port);
	addrlen           = sizeof(struct sockaddr_in);

	// get addr from command line and convert it
	if(inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0)
	{
		perror("inet_pton");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	printf("Trying to connect to the remote host\n");
	if(connect(sockfd, (struct sockaddr *) &server, addrlen) == -1)
	{
		perror("connect");
		exit(EXIT_FAILURE);
	}

	printf("Connection OK\n");

	return sockfd;
}


void send_receive (int sockfd) {
	unsigned char buf[BUFSIZ];
	memset (buf, 0, BUFSIZ);

	// first, send service name "pong"
	// send string
	if(send(sockfd, "pong", 4, 0) == -1) {
		perror("send pong");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	printf ("message 'pong' sent\n");

	memset (buf, 0, BUFSIZ);

	int paylen;
	paylen = recv(sockfd, buf, BUFSIZ, 0);
	if (paylen <= 0) {
		printf ("cannot connect to networkd\n");
		exit (EXIT_FAILURE);
	}
	printf ("should receive 'OK': %*.s\n", paylen, buf);

	memset (buf, 0, BUFSIZ);

	buf[0] = MSG_TYPE_DATA;

	// uint32_t v = htonl(6);
	// memcpy (buf+1, &v, sizeof (uint32_t));
	
	uint32_t v = 6;
	uint32_t net_paylen = htonl (v);
	memcpy (buf+1, &net_paylen, sizeof (uint32_t));

	buf[5] = 0;
	memcpy (buf+6, "coucou", 6);

	print_hexa ("SENT MESSAGE", buf, 12);

	// 2    | 6    | 0 | "coucou"
	// 1 B  | 4 B  | 1 | 6 B

	if(send(sockfd, buf, 12, 0) == -1) {
		perror("send coucou");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	printf ("message 'coucou' sent\n");

	memset (buf, 0, BUFSIZ);

	paylen = recv (sockfd, buf, BUFSIZ, 0);
	if(paylen < 0) {
		perror("recv a message");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	if (paylen > 0) {
		print_hexa ("RECEIVED MESSAGE", buf, paylen);
	}
	else {
		fprintf (stderr, "error: disconnection from the server\n");
		exit (EXIT_FAILURE);
	}

#if 0
	// send string
	if(sendto(sockfd, , ), 0) == -1)
	{
		perror("sendto");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
#endif
}


int main(int argc, char * argv[])
{
	int port = 9000;
	if (argc > 1) {
		port = atoi (argv[1]);
	}

	int sockfd = connection (port);

	send_receive (sockfd);

	printf("Disconnection\n");

	// close the socket
	close(sockfd);


	return EXIT_SUCCESS;
}
