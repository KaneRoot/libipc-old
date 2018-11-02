#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <time.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>

#define USOCK                               "./.socket"

#define handle_error(msg) \
    do { log_error (msg); exit(EXIT_FAILURE); } while (0)

#define handle_err(fun,msg)\
    do { log_error ("%s: file %s line %d %s", fun, __FILE__, __LINE__, msg); } while (0)

void log_format (const char* tag, const char* message, va_list args) {
    time_t now;

    time(&now);

    char * date =ctime(&now);
    date[strlen(date) - 1] = '\0';
    printf("%s:%s: ", date, tag);
    vprintf(message, args);

    printf("\n");
}

void log_error (const char* message, ...) {
    va_list args;
    va_start(args, message);

    log_format("error", message, args);

    va_end(args);
}

void log_info (const char* message, ...) {
    va_list args;
    va_start(args, message);

    log_format("info", message, args);
    va_end(args);
}

void log_debug (const char* message, ...) {
    va_list args;
    va_start(args, message);

    log_format("debug", message, args);

    va_end(args);
}

int32_t build_socket (char *servername, char * serverport)
{
    int32_t sockfd;
    struct sockaddr_in6 server;
    socklen_t addrlen;

    // socket factory
    if((sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // init remote addr structure and other params
    server.sin6_family = AF_INET6;
    server.sin6_port   = htons(atoi(serverport));
    addrlen           = sizeof(struct sockaddr_in6);

    // get addr from command line and convert it
    if(inet_pton(AF_INET6, servername, &server.sin6_addr) != 1)
    {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Trying to connect to the remote host\n");
    if(connect(sockfd, (struct sockaddr *) &server, addrlen) == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

int32_t usock_connect (int32_t *fd, const char *path)
{
    assert (fd != NULL);
    assert (path != NULL);

    if (fd == NULL) {
        handle_err ("usock_connect", "fd == NULL");
        return -1;
    }

    if (path == NULL) {
        handle_err ("usock_connect", "path == NULL");
        return -1;
    }

    int32_t sfd;
    struct sockaddr_un my_addr;
    socklen_t peer_addr_size;

    sfd = socket (AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        handle_err ("usock_connect", "sfd == -1");
        return -1;
    }

    // clear structure 
    memset(&my_addr, 0, sizeof(struct sockaddr_un));

    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, path, strlen (path));

    peer_addr_size = sizeof(struct sockaddr_un);
    if(connect(sfd, (struct sockaddr *) &my_addr, peer_addr_size) == -1) {
        handle_err ("usock_connect", "connect == -1");
        perror("connect()");
        exit(errno);
    }

    *fd = sfd;

    return 0;
}

int32_t usock_init (int32_t *fd, const char *path)
{
    assert (fd != NULL);
    assert (path != NULL);

    if (fd == NULL) {
        handle_err ("usock_init", "fd == NULL");
        return -1;
    }

    if (path == NULL) {
        handle_err ("usock_init", "path == NULL");
        return -1;
    }

    int32_t sfd;
    struct sockaddr_un my_addr;
    socklen_t peer_addr_size;

    sfd = socket (AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        handle_err ("usock_init", "sfd == -1");
        return -1;
    }

    // clear structure 
    memset(&my_addr, 0, sizeof(struct sockaddr_un));

    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, path, strlen (path));

    // TODO FIXME
    // delete the unix socket if already created

    peer_addr_size = sizeof(struct sockaddr_un);

    if (bind (sfd, (struct sockaddr *) &my_addr, peer_addr_size) == -1) {
        handle_err ("usock_init", "bind == -1");
        perror("bind()");
        return -1;
    }

    if (listen (sfd, 5) == -1) {
        handle_err ("usock_init", "listen == -1");
        perror("listen()");
        return -1;
    }

    *fd = sfd;

    return 0;
}

int32_t usock_accept (int32_t fd, int32_t *pfd)
{
    assert (pfd != NULL);

    if (pfd == NULL) {
        handle_err ("usock_accept", "pfd == NULL");
        return -1;
    }

    struct sockaddr_un peer_addr;
    memset (&peer_addr, 0, sizeof (struct sockaddr_un));
    socklen_t peer_addr_size = 0;

    *pfd = accept (fd, (struct sockaddr *) &peer_addr, &peer_addr_size);
    if (*pfd < 0) {
        handle_err ("usock_accept", "accept < 0");
        perror("listen()");
        return -1;
    }

    return 0;
}

int32_t usock_close (int32_t fd)
{
    int32_t ret;
    ret = close (fd);
    if (ret < 0) {
        handle_err ("usock_close", "close ret < 0");
        perror ("closing");
    }
    return ret;
}

int32_t usock_remove (const char *path)
{
    return unlink (path);
}

int32_t build_unix_socket (char * path)
{
    int32_t remotefd, localfd;

    usock_init (&localfd, path);
    usock_accept (localfd, &remotefd);

    return remotefd;
}

static
void sendfd (int32_t socket, int32_t fd)  // send fd by socket
{
    struct ipc_messagehdr msg = { 0 };
    char buf[CMSG_SPACE(sizeof(fd))];
    memset(buf, '\0', sizeof(buf));

    /* On Mac OS X, the struct iovec is needed, even if it points to minimal data */
    struct iovec io = { .iov_base = "", .iov_len = 1 };

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    struct cmsghdr * cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd));

    memmove(CMSG_DATA(cmsg), &fd, sizeof(fd));

    msg.msg_controllen = cmsg->cmsg_len;

    if (sendmsg(socket, &msg, 0) < 0)
        handle_err("sendfd", "Failed to send message\n");
}

int32_t main(int32_t argc, char * argv[])
{
    // check the number of args on command line
    if(argc != 3)
    {
        fprintf (stderr, "USAGE: %s @server port_num\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *servername = argv[1];
    char *serverport = argv[2];

    printf("Connection to the tcp socket\n");
    // 1. socket creation (tcp), connection to the server
    int32_t sockfd = build_socket (servername, serverport);

    printf("Sending 'coucou' to the tcp socket\n");
    // send a message to check the connection is effective
    if (write(sockfd, "coucou\n", strlen("coucou\n")) == -1) {
        perror("write");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf ("Connection to the unix socket\n");
    // 2. socket creation (unix)
    int32_t usockfd = build_unix_socket (USOCK);

    printf ("Passing the tcp socket to the unix socket\n");
    // 3. tcp socket passing to the client
    sendfd (usockfd, sockfd);

    // send a message to check the connection is (still) effective
    if (write(sockfd, "bye\n", strlen("bye\n")) == -1) {
        perror("write");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Disconnection\n");

    // 4. close sockets
    close(usockfd);
    close(sockfd);

    usock_remove (USOCK);

    return EXIT_SUCCESS;
}
