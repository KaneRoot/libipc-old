#ifndef __TCPDSERVER_H__
#define __TCPDSERVER_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../../lib/communication.h"

typedef struct {
	struct 	sockaddr_in c_addr;
	int sfd;
	int index;
} client_data;

//informations for server to listen at a address
typedef struct {
	struct 	sockaddr_in addr;
	char * request;
	struct process *p;
} info_request;

int initConnection (const info_request *req);
void endConnection (int sock);

void printAddr (struct sockaddr_in *csin);

void write_message(int sock, const char *buffer, size_t size);
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

void * server_thread(void *reqq);

void * client_thread(void *reqq);

int srv_get_new_request(char *buf, info_request *req);

void request_print (const info_request *req);

void main_loop(struct service *srv);


#endif
