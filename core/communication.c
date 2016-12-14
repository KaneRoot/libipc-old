#include "communication.h"
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#define LISTEN_BACKLOG 50
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int msg_send (const int fd, const char *buf, const int msize)
{
    int ret = 0;
    //printf ("%ld bytes to write\n", msize);
    ret = send (fd, buf, msize, 0);
    if (ret <= 0) {
        fprintf (stderr, "err: written %d\n", fd);
    }

    return ret;
}

int msg_recv (const int fd, char **buf)
{
    int ret = 0;
    ret = recv (fd, *buf, BUFSIZ, 0);
    if (ret < 0) {
        fprintf (stderr, "err: read %d\n", fd);
    }

    return ret;
}

int close_socket (int fd)
{
    int ret;

    ret = close (fd);
    if (ret < 0) {
        fprintf (stderr, "err: close [err: %d] %d\n", ret, fd);
        perror ("closing");
    }

    return ret;
}

// SERVICE

// init unix socket + srv->spath filled
int srv_init (int argc, char **argv, char **env, struct service *srv, const char *sname)
{
    if (srv == NULL)
        return ER_PARAMS;

    // TODO
    //      use the argc, argv and env parameters
    //      it will be useful to change some parameters transparently
    //      ex: to get resources from other machines, choosing the
    //          remote with environment variables

    argc = argc;
    argv = argv;
    env = env;

    // srv->version => already set
    // srv->index => already set

    // gets the service path, such as /tmp/ipc/<service>
    memset (srv->spath, 0, PATH_MAX);
    snprintf (srv->spath, PATH_MAX, "%s/%s-%d-%d"
            , TMPDIR, sname, srv->index, srv->version);

    // TODO TEST create a unix socket
    int sfd;
    struct sockaddr_un my_addr;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        return -1;

    // clear structure
    memset(&my_addr, 0, sizeof(struct sockaddr_un));

    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, srv->spath, strlen (srv->spath)); // TODO check size

    // delete the unix socket if already created
    // TODO FIXME
    unlink(my_addr.sun_path);
    if (bind(sfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_un)) == -1)
        return -1;

    if (listen(sfd, LISTEN_BACKLOG) == -1)
        return -1;

    srv->service_fd = sfd;

    return 0;
}

int srv_close (struct service *srv)
{
    close_socket (srv->service_fd);

    // TODO FIXME is unlink really necessary
    if (unlink (srv->spath)) {
        return 1;
    }

    return 0;
}

int srv_read (const struct service *srv, char ** buf)
{
    //printf("---%s\n", srv->spath);
    return msg_recv (srv->service_fd, buf);
}

int srv_write (const struct service *srv, const char * buf, size_t msize)
{
    //printf("---%s\n", srv->spath);
    return msg_send (srv->service_fd, buf, msize);
}

// APPLICATION

// Initialize connection with unix socket
// send the connection string to $TMP/<service>

// fill srv->spath && srv->service_fd
int app_connection (struct service *srv, const char *sname
        , const char *connectionstr, size_t msize)
{
    if (srv == NULL) {
        return -1;
    }

    // srv->version => already set
    // srv->index => already set

    // gets the service path, such as /tmp/ipc/<service>
    memset (srv->spath, 0, PATH_MAX);
    snprintf (srv->spath, PATH_MAX, "%s/%s-%d-%d"
            , TMPDIR, sname, srv->index, srv->version);

    int sfd;
    struct sockaddr_un my_addr;
    socklen_t peer_addr_size;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        return -1;

    // clear structure 
    memset(&my_addr, 0, sizeof(struct sockaddr_un));

    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, srv->spath, strlen (srv->spath)); // TODO check size

    peer_addr_size = sizeof(struct sockaddr_un);
    if(connect(sfd, (struct sockaddr *) &my_addr, peer_addr_size) == -1)
    {
        perror("connect()");
        exit(errno);
    }
    srv->service_fd = sfd;
    
    // TODO FIXME
    // send connection string and receive acknowledgement
    srv_write(srv, connectionstr, msize);

    return 0;
}

int app_close (struct service *srv)
{
    return close_socket (srv->service_fd);
}

int app_read (struct service *srv, char ** buf)
{   
    return msg_recv (srv->service_fd, buf);
}

int app_write (struct service *srv, char * buf, size_t msize)
{
    return msg_send (srv->service_fd, buf, msize);
}
