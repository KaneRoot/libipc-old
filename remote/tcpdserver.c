#include "tcpdserver.h"
#include "../lib/communication.h"

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
#define NBCLIENT 5

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

void endConnection(int sock) {
	close(sock);
}

void printClientAddr(struct sockaddr_in *csin) {
	printf("New client\n");
	printf("IP Addr : %s\n", inet_ntoa(csin->sin_addr));
	printf("Port : %u\n", ntohs(csin->sin_port));
}


void * service_thread(void * c_data) {
	client_data *cda = (client_data*) c_data;
	char buffer[BUF_SIZE];
	char *service;
	int version;
	int clientSock = cda->sfd;

	if (read_message(clientSock, buffer) == -1) {
		perror("read_message()");
		return NULL;
	}else {
		parseServiceVersion(buffer, &service, &version);
	}
	
	/* TODO : service correspond au service que le client veut utiliser 
	** il faut comparer service à un tableau qui contient les services 
	** disponibles
	*/

	//path service
	char * servicePath = (char*) calloc((strlen(TMPDIR) + strlen(service) + 1), sizeof(char));
	if (servicePath == NULL) {
		perror("calloc()");
	}
	strcat(servicePath, TMPDIR);
	strcat(servicePath, service);

	//pid index version
	char * piv =  (char*) calloc(PATH_MAX, sizeof(char));
	if (piv == NULL) {
		perror("calloc()");
	}
	makePivMessage(&piv, getpid(), cda->index, version);
	printf("piv : %s\n",piv );

	//write pid index version in T/I/S of service
	int ret = file_write(servicePath, piv, strlen(piv));
	if(ret == 0) {
		perror("file_write()");
		free(servicePath);
		free(piv);
		return NULL;
	}

	// gets the service path, such as /tmp/ipc/pid-index-version-in/out
	char *pathname[2];
	pathname[0] = (char*) calloc(PATH_MAX, sizeof(char));
	if (pathname[0] == NULL) {
		perror("calloc()");
	}
	pathname[1] = (char*) calloc(PATH_MAX, sizeof(char));
	if (pathname[1] == NULL) {
		perror("calloc()");
	}
	inOutPathCreate(pathname, cda->index, version);

	//create in out files
	if(fifo_create(pathname[0]) != 0) {
		perror("fifo_create()");
		return NULL;
	}

	if(fifo_create(pathname[1]) != 0) {
		perror("fifo_create()");
		return NULL;
	}

	//open -in fifo file
	int fdin = open (pathname[0], O_RDWR);
    if (fdin <= 0) {
        printf("open: fd < 0\n");
        perror ("open()");
        return NULL;
    }

	//utilisation du select() pour surveiller la socket du client et fichier in
	fd_set rdfs;
	int max = clientSock > fdin ? clientSock : fdin;

	printf("Waitting for new messages :\n" );
	while(1) {
		FD_ZERO(&rdfs);

		/* add STDIN_FILENO */
		FD_SET(STDIN_FILENO, &rdfs);

		//add client's socket
		FD_SET(clientSock, &rdfs);

		//add in file
		FD_SET(fdin, &rdfs);

		if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
		{
			perror("select()");
			exit(errno);
		}

		/* something from standard input : i.e keyboard */
		if(FD_ISSET(STDIN_FILENO, &rdfs))
		{
			/* stop process when type on keyboard */
			printf("thread %d shutdown\n", cda->index );
			break; 
		}else /*if (FD_ISSET(fdin, &rdfs))*/{
			if (FD_ISSET(clientSock, &rdfs)) {
				if(read_message(clientSock, buffer) > 0) {
					printf("message : %s\n",buffer );
					if(file_write(pathname[1], buffer, strlen(buffer)) < 0) {
						perror("file_write");
					}
					printf("ok\n");
				}
			}

			if (FD_ISSET(fdin, &rdfs)) {

				if(read(fdin, &buffer, BUF_SIZE) < 0) {
					perror("read()");
				}
				printf("message from file in : %s\n", buffer );
			}
		}
	}

	//free
	free(servicePath);
	free(piv);
	free(service);
	free(pathname[0]);
	free(pathname[1]);
	
	
	//close the files descriptors
	close(fdin);
	close(clientSock);

	return NULL;
}

void parseServiceVersion(char * buf, char ** service, int *version) {
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

void inOutPathCreate(char ** pathname, int index, int version) {
	int length = snprintf( NULL, 0, "%d", version );
	char* versionStr = malloc( length + 2 );
	snprintf( versionStr, length + 1, "%d", version );

	int length2 = snprintf( NULL, 0, "%d", getpid() );
	char * pidprocess = malloc( length2 + 1 );
	snprintf( pidprocess, length2 + 1, "%d", getpid() );

	int length3 = snprintf( NULL, 0, "%d", index );
	char* indexStr = malloc( length3 + 1 );
	snprintf( indexStr, length3 + 1, "%d", index );	

	strcat(pathname[0], TMPDIR);
	strcat(pathname[0], pidprocess);
	strcat(pathname[0], "-");
	strcat(pathname[0], indexStr);
	strcat(pathname[0], "-");
	strcat(pathname[0], versionStr);
	strcat(pathname[0], "-in");
	
	strcat(pathname[1], TMPDIR);
	strcat(pathname[1], pidprocess);
	strcat(pathname[1], "-");
	strcat(pathname[1], indexStr);
	strcat(pathname[1], "-");
	strcat(pathname[1], versionStr);
	strcat(pathname[1], "-out");

	free(pidprocess);
	free(indexStr);
	free(versionStr);
}

void makePivMessage (char ** piv, int pid, int index, int version) {
	int length1 = snprintf( NULL, 0, "%d", getpid() );
	char * pidprocess = malloc( length1 + 1 );
	snprintf( pidprocess, length1 + 1, "%d", getpid() );

	int length2 = snprintf( NULL, 0, "%d", index );
	char* indexStr = malloc( length2 + 1 );
	snprintf( indexStr, length2 + 1, "%d", index );	

	int length3 = snprintf( NULL, 0, "%d", version );
	char* versionStr = malloc( length3 + 2 );
	snprintf( versionStr, length3 + 1, "%d", version );

	strcat(*piv, pidprocess);
	strcat(*piv, " ");
	strcat(*piv, indexStr);
	strcat(*piv, " ");
	strcat(*piv, versionStr);

	free(pidprocess);
	free(indexStr);
	free(versionStr);
}

int main(int argc, char * argv[], char **env) {
	//client 
	client_data tab_client[NBCLIENT];
	pthread_t tab_service_threads[NBCLIENT];
	int actual = 0;
	int i;

	int sock = init_connection();
	fd_set rdfs;
	int max = sock;

	printf("Waitting for new clients :\n" );
	while(1) {
		FD_ZERO(&rdfs);

		/* add STDIN_FILENO */
		FD_SET(STDIN_FILENO, &rdfs);

		//add client's socket
		FD_SET(sock, &rdfs);

		if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
		{
			perror("select()");
			exit(errno);
		}

		/* something from standard input : i.e keyboard */
		if(FD_ISSET(STDIN_FILENO, &rdfs))
		{
			/* stop process when type on keyboard */
			// for (i = 0; i < actual; i++) {
			// 	if (pthread_cancel(tab_service_threads[i]) != 0) {
			// 		printf("Aucun thread correspond \n");
			// 	}	
			// }
			printf("server shutdown\n");
			break; 
		}
		else if (FD_ISSET(sock, &rdfs)){
			//New client
			socklen_t sinsize = sizeof (struct sockaddr_in);
			tab_client[actual].sfd = accept(sock, (struct sockaddr *)&tab_client[actual].c_addr, &sinsize);
			if(tab_client[actual].sfd == -1)
			{
				perror("accept()");
				close(sock);
				exit(errno);
			}
			printClientAddr(&tab_client[actual].c_addr);

			tab_client[actual].index = actual;
			
			int ret = pthread_create( &tab_service_threads[actual], NULL, &service_thread, (void *) &tab_client[actual]);
			if (ret) {
				perror("pthread_create()");
				endConnection(sock);
				exit(errno);
			} else {
				printf("Creation of listen thread %d\n", actual);
			}

			max = tab_client[actual].sfd > max ? tab_client[actual].sfd : max;
			actual++;
		}
	}

	for (i = 0; i < actual; i++) {
		pthread_join(tab_service_threads[i], NULL);	
	}

	endConnection(sock);

	return 0;
}
