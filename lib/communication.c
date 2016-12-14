#include "communication.h"
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#define LISTEN_BACKLOG 50
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int file_write (const int fd, const char *buf, const int msize)
{
    int ret = 0;
    //printf ("%ld bytes to write\n", msize);
    ret = send (fd, buf, msize, 0);
    if (ret <= 0) {
        fprintf (stderr, "err: written %d\n", fd);
    }

    return ret;
}

int file_read (const int fd, char **buf)
{
    int ret = 0;
    ret = recv (fd, *buf, BUFSIZ, 0);
    if (ret < 0) {
        fprintf (stderr, "err: read %d\n", fd);
    }

    return ret;
}

int close_socket(int fd) {
    int ret;

    ret = close (fd);
    if (ret < 0) {
        fprintf (stderr, "err: close [err: %d] %d\n", ret, fd);
        perror ("closing");
    }

    return ret;
}

int srv_init (int argc, char **argv, char **env, struct service *srv, const char *sname, int (*cb)(int argc, char **argv, char **env, struct service *srv, const char *sname))
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

    // gets the service path, such as /tmp/<service>
    memset (srv->spath, 0, PATH_MAX);
    strncat (srv->spath, TMPDIR, PATH_MAX -1);
    strncat (srv->spath, sname, PATH_MAX -1);

    srv->version = COMMUNICATION_VERSION;
    srv->index = 0; // TODO

    if (cb != NULL) {
        int ret = (*cb) (argc, argv, env, srv, sname);
        if (ret != 0)
            return ret;
    }

    return 0;
}

// SERVICE

int srv_create (struct service *srv)
{
    int ret;
    if ((ret = mkfifo (srv->spath, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
        switch (errno) {
            case EACCES :
                printf ("file %s : EACCES\n", srv->spath);
                return 1;
            case EEXIST :
                printf ("file %s : EEXIST\n", srv->spath);
                break;
            case ENAMETOOLONG :
                printf ("file %s : ENAMETOOLONG\n", srv->spath);
                return 2;
            case ENOENT :
                printf ("file %s : ENOENT\n", srv->spath);
                return 3;
            case ENOSPC :
                printf ("file %s : ENOSPC\n", srv->spath);
                return 4;
            case ENOTDIR :
                printf ("file %s : ENOTDIR\n", srv->spath);
                return 5;
            case EROFS :
                printf ("file %s : EROFS\n", srv->spath);
                return 6;
            default :
                printf ("err file %s unknown\n", srv->spath);
                return 7;
        }
    }

    return 0;
}

int srv_close (struct service *srv)
{
    if (unlink (srv->spath)) {
        return 1;
    }

    return 0;
}

int srv_get_new_process (char *buf, struct process *p)
{
    char *token = NULL, *saveptr = NULL;
    char *str = NULL;
    int i = 0;

    pid_t pid = 0;
    int index = 0;
    int version = 0;

    for (str = buf, i = 1; ; str = NULL, i++) {
        token = strtok_r(str, " ", &saveptr);
        if (token == NULL)
            break;

        if (i == 1) {
            pid = strtoul(token, NULL, 10);
        }
        else if (i == 2) {
            index = strtoul(token, NULL, 10);
        }
        else if (i == 3) {
            version = strtoul(token, NULL, 10);
        }
    }

    //if (buf != NULL)
    //    free (buf);
    srv_process_gen (p, pid, index, version);

    return 0;
}

int srv_read (const struct service *srv, char ** buf)
{
    //printf("---%s\n", srv->spath);
    return file_read (srv->service_fd, buf);
}

int srv_write (const struct service *srv, const char * buf, size_t msize)
{
    //printf("---%s\n", srv->spath);
    return file_write (srv->service_fd, buf, msize);
}

// APPLICATION

//Init connection with unix socket
// send the connection string to $TMP/<service>
int app_srv_connection (struct service *srv, const char *connectionstr, size_t msize)
{
    if (srv == NULL) {
        return -1;
    }

    int sfd;
    struct sockaddr_un my_addr;
    socklen_t peer_addr_size;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        return -1;

    memset(&my_addr, 0, sizeof(struct sockaddr_un));
    // Clear structure 
    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, srv->spath, sizeof(my_addr.sun_path) - 1);

    peer_addr_size = sizeof(struct sockaddr_un);
    if(connect(sfd,(struct sockaddr *) &my_addr, peer_addr_size) == -1)
    {
        perror("connect()");
        exit(errno);
    }
    srv->service_fd = sfd;
    
    return srv_write(srv, connectionstr, msize);

}

int proc_connection(struct process *p)  {
    int sfd;
    struct sockaddr_un my_addr;
    socklen_t peer_addr_size;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        return -1;

    memset(&my_addr, 0, sizeof(struct sockaddr_un));
    // Clear structure 
    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, p->path_proc, sizeof(my_addr.sun_path) - 1);

    peer_addr_size = sizeof(struct sockaddr_un);
    if(connect(sfd,(struct sockaddr *) &my_addr, peer_addr_size) == -1)
    {
        perror("connect()");
        exit(errno);
    }
    p->proc_fd = sfd;

    return 0;
}

int app_create (struct process *p, pid_t pid, int index, int version)
{
    if (version == 0) {
        version = COMMUNICATION_VERSION;
    }

    // then creates the structure
    srv_process_gen (p, pid, index, version);

    return 0;
}

int app_destroy (struct process *p)
{
    if (unlink (p->path_proc)) {
        return 1;
    }

    return 0;
}

int app_read (struct process *p, char ** buf)
{   
    //printf("---%s\n", p->path_proc);
    return file_read (p->proc_fd, buf);
}

int app_write (struct process *p, char * buf, size_t msize)
{
    //printf("---%s\n", p->path_proc);
    return file_write (p->proc_fd, buf, msize);
}

/*init a unix socket : bind, listen
 *and return a socket 
 */
int set_listen_socket(const char *path) {
    int sfd;
    struct sockaddr_un my_addr;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        return -1;

    memset(&my_addr, 0, sizeof(struct sockaddr_un));
    /* Clear structure */
    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, path, sizeof(my_addr.sun_path) - 1);

    unlink(my_addr.sun_path);
    if (bind(sfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_un)) == -1)
        return -1;

    if (listen(sfd, LISTEN_BACKLOG) == -1)
        return -1;

    return sfd;
}
