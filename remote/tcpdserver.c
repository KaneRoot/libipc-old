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


// void printClientAddr(struct sockaddr_in &csin) {
// 	printf("%s\n", );
// }


int main(int argc, char * argv[], char **env) {

	int sock = init_connection();

	struct sockaddr_in csin = { 0 };
	socklen_t sinsize = sizeof csin;
	int csock = accept(sock, (struct sockaddr *)&csin, &sinsize);
	if(csock == -1)
	{
		perror("accept()");
		exit(errno);
	}

	printf("new client\n");

	close(sock);
	close(csock);
	return 0;
}