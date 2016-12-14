#include "../../lib/communication.h"
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>

#define PONGD_SERVICE_NAME "pongd"
#define LISTEN_BACKLOG 50
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char * argv[]) {
    char *srv_message = malloc(BUFSIZ);
    char *pidfile = malloc(BUFSIZ);
    char *buffer = malloc(BUFSIZ);

    snprintf(srv_message, BUFSIZ, "%s %d 1 1", "connect 127.0.0.1 6000", getpid());
    snprintf(pidfile, BUFSIZ, "%s%d-1-1", "/tmp/ipc/", getpid());

    char *proc_message = "hello frero";

    int sfd;
    struct sockaddr_un my_addr;
    socklen_t peer_addr_size;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        return -1;

    memset(&my_addr, 0, sizeof(struct sockaddr_un));
    // Clear structure 
    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, "/tmp/ipc/tcpd", sizeof(my_addr.sun_path) - 1);

    peer_addr_size = sizeof(struct sockaddr_un);
    if(connect(sfd,(struct sockaddr *) &my_addr, peer_addr_size) == -1)
    {
        perror("connect()");
        exit(errno);
    }
    //printf("connected...\n");
    file_write(sfd, srv_message, strlen(srv_message));
    //printf("%s\n", proc_message);
    //sleep(1);
    file_write(sfd, proc_message, strlen(proc_message));
    file_read(sfd, &buffer);
    printf("%s\n", buffer);
    //sleep(1);
    file_write(sfd, "exit", 4);

    close(sfd);

    // //sleep(1);
    // cfd = socket(AF_UNIX, SOCK_STREAM, 0);
    // if (sfd == -1)
    //     return -1;
    // strncpy(my_addr.sun_path, pidfile, sizeof(my_addr.sun_path) - 1);
    // if(connect(cfd,(struct sockaddr *) &my_addr, peer_addr_size) == -1)
    // {
    //     perror("connect()");
    //     exit(errno);
    // }

    // close(cfd);

    return 0;
}
