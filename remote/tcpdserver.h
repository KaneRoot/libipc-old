#ifndef __TCPDSERVER_H__
#define __TCPDSERVER_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct {
	struct 	sockaddr_in c_sock;
	int sfd;
	int index;
} p_data;

int initConnection ();
void endConnection (int sock);

void printClientAddr (struct sockaddr_in *csin);

void write_message(int sock, const char *buffer);
int read_message(int sock, char *buffer);

//2 threads for listen and send data
void * service_thread(void * pdata);

//parse the first message from client in service and version
void parseServiceVersion(char * buf, char ** service, int *version);

//create 2 pathnames such as : pid-index-version-in/out
void inOutPathCreate(char ** pathname, int index, int version);

//create a fifo file
int fifo_create (char * path);

//create first message for a service : pid index version
void makePivMessage(char ** piv, int pid, int index, int version);


#endif