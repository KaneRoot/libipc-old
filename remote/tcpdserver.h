#ifndef __TCPDSERVER_H__
#define __TCPDSERVER_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct {
	struct 	sockaddr_in c_sock;
	int sfd;
	char * fifoOut;
} p_data;

int initConnection ();
void endConnection (int sock, int csock);
void printClientAddr (struct sockaddr_in *csin);
void write_message(int sock, const char *buffer);
int read_message(int sock, char *buffer);
void * listen_thread(void * p_data);
void parseServiceVersion(char * buf, char ** service, int *version);
void inOutPathCreate(char ** pathname, int version);
int fifo_create (char * path);


#endif