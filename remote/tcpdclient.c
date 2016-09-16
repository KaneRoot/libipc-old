#include "tcpdserver.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 1024
#define PORT 6000

void write_message(int sock, const char *buffer)
{
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

int read_message(int sock, char *buffer)
{
   int n = 0;

   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}


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

	write_message(sock, "pongd 5");

	close(sock);

	return 0;
}