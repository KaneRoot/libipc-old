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

static
int recvsockfd (int socket)  // receive fd from socket
{
    struct ipc_messagehdr msg = {0};

    /* On Mac OS X, the struct iovec is needed, even if it points to minimal data */
    char m_buffer[1];
    struct iovec io = { .iov_base = m_buffer, .iov_len = sizeof(m_buffer) };
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;

    char c_buffer[256];
    msg.msg_control = c_buffer;
    msg.msg_controllen = sizeof(c_buffer);

    if (recvmsg(socket, &msg, 0) < 0)
        handle_err ("recvsockfd", "Failed to receive message\n");

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

    printf ("About to extract fd\n");
    int fd;
    memmove(&fd, CMSG_DATA(cmsg), sizeof(fd));
    printf ("Extracted fd %d\n", fd);

    return fd;
}

int usock_connect (int *fd, const char *path)
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

    int sfd;
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

int usock_init (int *fd, const char *path)
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

    int sfd;
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

int usock_accept (int fd, int *pfd)
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

int usock_close (int fd)
{
    int ret;
    ret = close (fd);
    if (ret < 0) {
        handle_err ("usock_close", "close ret < 0");
        perror ("closing");
    }
    return ret;
}

int usock_remove (const char *path)
{
    return unlink (path);
}

int main(int argc, char * argv[])
{
    int tcpsockfd;
    int usockfd;
    // check the number of args on command line
    if(argc != 1)
    {
        fprintf (stderr, "USAGE: %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("Connection to the unix socket\n");

    // 1. unix socket connection
    int ret = usock_connect (&usockfd, USOCK);
    if (ret != 0) {
        fprintf (stderr, "error: usock_connect\n");
        exit(EXIT_FAILURE);
    }

    printf("Receiving the tcp socket\n");

    // 2. receive the tcp socket
    tcpsockfd = recvsockfd (usockfd);

    printf("Sending 'hello world' to the tcp socket\n");

    // 3. send a message to check the connection is effective
    if (write(tcpsockfd, "hello world\n", strlen("hello world\n")) == -1) {
        perror("write");
        close(tcpsockfd);
        exit(EXIT_FAILURE);
    }

    printf("Disconnection of both sockets\n");

    // 4. close sockets
    close(usockfd);
    close(tcpsockfd);

    return EXIT_SUCCESS;
}
