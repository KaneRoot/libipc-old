#include "tcpdserver.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


#define PORT 6000


int main(int argc, char ** argv) {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1)
	{
	    perror("socket()");
	    exit(errno);
	}

	struct sockaddr_in sin = {0};
	sin.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	sin.sin_port = htons(PORT); /* on utilise htons pour le port */
	sin.sin_family = AF_INET;

	if(connect(sock,(struct sockaddr *) &sin, sizeof(struct sockaddr)) == -1)
	{
	    perror("connect()");
	    exit(errno);
	}

	close(sock);

	return 0;
}