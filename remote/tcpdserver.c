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

#define PORT 6000
#define BUF_SIZE 1024

int init_connection(void)
{
   int sock = socket(AF_INET, SOCK_STREAM, 0);
   struct sockaddr_in sin = { 0 };

   if(sock == -1)
   {
		perror("socket()");
		exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(struct sockaddr *) &sin, sizeof sin) == -1)
   {
		perror("bind()");
		exit(errno);
   }

   if(listen(sock, 5) == -1)
   {
		perror("listen()");
		exit(errno);
   }

   return sock;
}

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

void printClientAddr(struct sockaddr_in *csin) {
	printf("%s\n", inet_ntoa(csin->sin_addr));
	printf("%u\n", ntohs(csin->sin_port));
}


int main(int argc, char * argv[], char **env) {

	int sock = init_connection();
	char buffer[BUF_SIZE];

	struct sockaddr_in csin = { 0 };
	socklen_t sinsize = sizeof csin;
	int csock = accept(sock, (struct sockaddr *)&csin, &sinsize);
	if(csock == -1)
	{
		perror("accept()");
		exit(errno);
	}

	//printf("new client\n");
	printClientAddr(&csin);

	int n = read_message(csock, buffer);
	if (n == -1) {
		perror("read_message()");
	}else {
		printf("%s\n",buffer );
	}


	close(sock);
	close(csock);
	return 0;
}