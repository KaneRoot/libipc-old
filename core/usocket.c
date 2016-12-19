#include "usocket.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#define handle_err(fun,msg)\
    fprintf (stderr, "%s: file %s line %d %s\n", fun, __FILE__, __LINE__, msg);

int usock_send (const int fd, const char *buf, const int msize)
{
    print_hexa ("msg send", (unsigned char *)buf, msize);
    printf ("msg to send: (%d) \n", msize);
    fflush(stdout);

    int ret = 0;
    //printf ("%ld bytes to write\n", msize);
    ret = send (fd, buf, msize, 0);
    if (ret <= 0)
        handle_err ("usock_send", "send ret <= 0");
    return ret;
}

int usock_recv (const int fd, char **buf, size_t *msize)
{
    assert(buf != NULL);
    assert(msize != NULL);

    if (buf == NULL) {
        handle_err ("usock_recv", "buf == NULL");
        return -1;
    }

    if (msize == NULL) {
        handle_err ("usock_recv", "msize == NULL");
        return -1;
    }

    if (*buf == NULL) {
        // do not allocate too much memory
        if (*msize > BUFSIZ)
            handle_err ("usock_recv", "msize > BUFSIZ");
        *buf = malloc ((*msize < BUFSIZ) ? *msize : BUFSIZ);
    }

    int ret = 0;
    ret = recv (fd, *buf, *msize, 0);
    if (ret < 0) {
        handle_err ("usock_recv", "recv ret < 0");
        *msize = 0;
        return ret;
    }
    *msize = (size_t) ret;
    print_hexa ("msg recv", (unsigned char *)*buf, *msize);
    return ret;
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

    if (listen (sfd, LISTEN_BACKLOG) == -1) {
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
    socklen_t peer_addr_size;

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
