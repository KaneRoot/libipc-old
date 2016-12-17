#include "communication.h"
#include "usocket.h"

#include <assert.h>

#include <stdio.h>
#include <errno.h>

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

void service_path (char *path, const char *sname, int index, int version)
{
    memset (path, 0, PATH_MAX);
    snprintf (path, PATH_MAX, "%s/%s-%d-%d", TMPDIR, sname, index, version);
}

int srv_init (int argc, char **argv, char **env
        , struct service *srv, const char *sname)
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

    // gets the service path
    service_path (srv->spath, sname, srv->index, srv->version);

    usock_init (&srv->service_fd, srv->spath);

    return 0;
}

int srv_close (struct service *srv)
{
    usock_close (srv->service_fd);
    return usock_remove (srv->spath);
}

int srv_read (const struct service *srv, char ** buf, size_t *msize)
{
    return usock_recv (srv->service_fd, buf, msize);
}

int srv_write (const struct service *srv, const char * buf, size_t msize)
{
    return usock_send (srv->service_fd, buf, msize);
}

int app_connection (struct service *srv, const char *sname
        , const char *connectionstr, size_t msize)
{

    assert (srv != NULL);
    assert (sname != NULL);

    if (srv == NULL) {
        return -1;
    }

    // gets the service path
    service_path (srv->spath, sname, srv->index, srv->version);

    usock_connect(&srv->service_fd, srv->spath);

    // TODO: connection algorithm
    // send connection string and receive acknowledgement
    srv_write(srv, connectionstr, msize);

    return 0;
}

int app_close (struct service *srv)
{
    return usock_close (srv->service_fd);
}

int app_read (struct service *srv, char ** buf, size_t *msize)
{   
    return usock_recv (srv->service_fd, buf, msize);
}

int app_write (struct service *srv, char * buf, size_t msize)
{
    return usock_send (srv->service_fd, buf, msize);
}
