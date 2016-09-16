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
#include <pthread.h>
#include <sys/stat.h> // mkfifo
#include <linux/limits.h>

#define PORT 6000
#define BUF_SIZE 1024
#define TMPDIR "/tmp/ipc/"

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

void endConnection(int sock, int sfd) {
	close(sock);
	close(sfd);
}

void printClientAddr(struct sockaddr_in *csin) {
	printf("New client\n");
	printf("IP Addr : %s\n", inet_ntoa(csin->sin_addr));
	printf("Port : %u\n", ntohs(csin->sin_port));
}


int main(int argc, char * argv[], char **env) {

	int sock = init_connection();
	// char buffer[BUF_SIZE];

	struct sockaddr_in csin = { 0 };
	socklen_t sinsize = sizeof csin;
	int sfd = accept(sock, (struct sockaddr *)&csin, &sinsize);
	if(sfd == -1)
	{
		perror("accept()");
		close(sock);
		exit(errno);
	}

	printClientAddr(&csin);
	printf("%d \n",getpid());

	pthread_t listenPid;
	p_data *p_d_listen = malloc (sizeof(p_data));
	//p_d_listen.c_sock = NULL;
	p_d_listen->sfd = sfd;
	
	int ret = pthread_create( &listenPid, NULL, &listen_thread, (void *) p_d_listen);
	if (ret) {
		perror("pthread_create()");
		endConnection(sock, sfd);
		exit(errno);
	} else {
		printf("Creation of listen thread \n");
	}

	pthread_join(listenPid, NULL);

	endConnection(sock, sfd);

	return 0;
}

void * listen_thread(void * pdata) {
	p_data *pda = (p_data*) pdata;
	char buffer[BUF_SIZE];
	char *service;
	int version;

	int n = read_message(pda->sfd, buffer);
	if (n == -1) {
		perror("read_message()");
		return NULL;
	}else {
		parseString(buffer, &service, &version);
	}
	// printf("%s\n", service);
	// printf("%d\n", version);
	/* TODO : service correspond au service que le client veut utiliser 
	** il faut comparer service à un tableau qui contient les services 
	** disponibles
	*/

	// printf("%d\n", getpid());
	// printf("%zu\n", pthread_self());

	// gets the service path, such as /tmp/ipc/pid-index-version-in/out
	char *pathname[2];
	inOutPathCreate(pathname, version);

	printf("%s\n",pathname[0] );
	printf("%s\n",pathname[1] );

	/*if(fifo_create(pathname[0]) != 0) {
		perror("fifo_create()");
		return NULL;
	}

	if(fifo_create(pathname[1]) != 0) {
		perror("fifo_create()");
		return NULL;
	}*/

	return NULL;
}

void parseString(char * buf, char ** service, int *version) {
	char *token = NULL, *saveptr = NULL;
    char *str = NULL;
    int i = 0;

    for (str = buf, i = 1; ; str = NULL, i++) {
        token = strtok_r(str, " ", &saveptr);
        if (token == NULL)
            break;

        if (i == 1) {
            *service = token;
        }
        else if (i == 2) {
            *version = strtoul(token, NULL, 10);
        }
    }
}

int fifo_create (char * path)
{
    int ret;
    if ((ret = mkfifo (path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
        switch (errno) {
            case EACCES :
                printf ("file %s : EACCES\n", path);
                return 1;
            case EEXIST :
                printf ("file %s : EEXIST\n", path);
                break;
            case ENAMETOOLONG :
                printf ("file %s : ENAMETOOLONG\n", path);
                return 2;
            case ENOENT :
                printf ("file %s : ENOENT\n", path);
                return 3;
            case ENOSPC :
                printf ("file %s : ENOSPC\n", path);
                return 4;
            case ENOTDIR :
                printf ("file %s : ENOTDIR\n", path);
                return 5;
            case EROFS :
                printf ("file %s : EROFS\n", path);
                return 6;
            default :
                printf ("err file %s unknown\n", path);
                return 7;
        }
    }

    return ret;
}

void inOutPathCreate(char ** pathname, int version) {
	pathname[0] = malloc(sizeof PATH_MAX);
	pathname[1] = malloc(sizeof PATH_MAX);

	int length = snprintf( NULL, 0, "%d", version );
	char* str = malloc( length + 2 );
	snprintf( str, length + 1, "%d", version );

	int length2 = snprintf( NULL, 0, "%zu", pthread_self() );
	char * str2 = malloc( length2 + 1 );
	snprintf( str2, length2 + 1, "%zu", pthread_self() );

	int length3 = snprintf( NULL, 0, "%d", 1 );
	char* str3 = malloc( length3 + 1 );
	snprintf( str3, length3 + 1, "%d", 1 );	

	strcat(pathname[0], TMPDIR);
	strcat(pathname[0], str2);
	strcat(pathname[0], "-");
	strcat(pathname[0], str3);
	strcat(pathname[0], "-");
	strcat(pathname[0], str);
	strcat(pathname[0], "-in");
	
	strcat(pathname[1], TMPDIR);
	strcat(pathname[1], str2);
	strcat(pathname[1], "-");
	strcat(pathname[1], str3);
	strcat(pathname[1], "-");
	strcat(pathname[1], str);
	strcat(pathname[1], "-out");
}


