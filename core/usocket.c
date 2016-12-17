#include "usocket.h"
#include <assert.h>

#define handle_err(fun,msg)\
    fprintf (stderr, "%s: file %s line %d %s\n", fun, __FILE__, __LINE__, msg);

int usock_send (const int fd, const char *buf, const int msize)
{
    int ret = 0;
    //printf ("%ld bytes to write\n", msize);
    ret = send (fd, buf, msize, 0);
    if (ret <= 0) {
        fprintf (stderr, "usock_send: file %s line %d send ret <= 0\n"
                , __FILE__, __LINE__);
    }
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

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
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
}

int usock_listen (int *fd, const char *path)
{
    assert (fd != NULL);
    assert (path != NULL);

    if (fd == NULL) {
        handle_err ("usock_listen", "fd == NULL");
        return -1;
    }

    if (path == NULL) {
        handle_err ("usock_listen", "path == NULL");
        return -1;
    }

    int sfd;
    struct sockaddr_un my_addr;
    socklen_t peer_addr_size;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        handle_err ("usock_listen", "sfd == -1");
        return -1;
    }

    // clear structure 
    memset(&my_addr, 0, sizeof(struct sockaddr_un));

    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, path, strlen (path));

    // TODO FIXME
    // delete the unix socket if already created
    unlink(my_addr.sun_path);

    peer_addr_size = sizeof(struct sockaddr_un);
    if (bind(sfd, (struct sockaddr *) &my_addr, peer_addr_size) == -1) {
        handle_err ("usock_listen", "bind == -1");
        perror("bind()");
        return -1;
    }

    if (listen(sfd, LISTEN_BACKLOG) == -1) {
        handle_err ("usock_listen", "listen == -1");
        perror("listen()");
        return -1;
    }

    *fd = sfd;

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
