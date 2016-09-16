#ifndef __TCPDSERVER_H__
#define __TCPDSERVER_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int initConnection ();
void endConnection (int sock, int csock);
void printClientAddr (struct sockaddr_in *csin);
void write_message(int sock, const char *buffer);
int read_message(int sock, char *buffer);


#endif