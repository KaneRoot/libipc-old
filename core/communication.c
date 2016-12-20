#include "communication.h"
#include "usocket.h"
#include "msg-format.h"
#include "utils.h"

#include <assert.h>
#include <stdio.h>
#include <errno.h>


#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_err(fun,msg)\
    fprintf (stderr, "%s: file %s line %d %s\n", fun, __FILE__, __LINE__, msg);

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

// TODO
int srv_accept (struct service *srv, struct process *p)
{
    usock_accept (srv->service_fd, &p->proc_fd);
    char *buf = NULL;
    size_t msgsize = BUFSIZ;

    srv_read (p, &buf, &msgsize);

    msgsize = 0;
    msg_format_ack (buf, NULL, &msgsize);

    srv_write (p, buf, msgsize);

    free (buf);

    return 0;
}

int srv_close (struct service *srv)
{
    usock_close (srv->service_fd);
    return usock_remove (srv->spath);
}

int srv_close_proc (struct process *p)
{
    return usock_close (p->proc_fd);
}

int srv_read (const struct process *p, char **buf, size_t *msize)
{
    return usock_recv (p->proc_fd, buf, msize);
}

int srv_write (const struct process *p, const char * buf, size_t msize)
{
    return usock_send (p->proc_fd, buf, msize);
}

int app_connection (int argc, char **argv, char **env
        , struct service *srv, const char *sname
        , const char *connectionstr, size_t msize)
{
    argc = argc;
    argv = argv;
    env = env;

    assert (srv != NULL);
    assert (sname != NULL);

    if (srv == NULL) {
        return -1;
    }

    // gets the service path
    service_path (srv->spath, sname, srv->index, srv->version);

    usock_connect (&srv->service_fd, srv->spath);

    // TODO: connection algorithm
    // send connection string and receive acknowledgement
    char send_buffer [BUFSIZ];
    memset (send_buffer, 0, BUFSIZ);
    if (msg_format_con (send_buffer, connectionstr, &msize) < 0) {
        handle_err ("app_connection", "msg_format_con");
        return -1;
    }
    app_write (srv, send_buffer, msize);

    char *buffer = NULL;
    size_t read_msg_size = BUFSIZ;

    app_read (srv, &buffer, &read_msg_size);

    assert (buffer[0] == MSG_TYPE_ACK);
    assert (read_msg_size == 3);

    free (buffer);

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


/*prendre en parametre un tableau de process.
* trouver le processus/service actif et renvoyer CONNECTION/APPLICATION 
* si un processus il va etre placer dans proc
* si un service il va etre placer dans service.
*/
int srv_select(struct array_proc *ap, struct service *srv, struct process **proc) {
    int i, j;
    /* master file descriptor list */
    fd_set master;
    fd_set readf;

    /* maximum file descriptor number */
    int fdmax;
    /* listening socket descriptor */
    int listener = srv->service_fd;

    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&readf);
    /* add the listener to the master set */
    FD_SET(listener, &master);

    for(i=0; i < ap->size; i++) {
        FD_SET(ap->tab_proc[i]->proc_fd, &master);
    }

    /* keep track of the biggest file descriptor */
    fdmax = getMaxFd(ap) > srv->service_fd ? getMaxFd(ap) : srv->service_fd; 

    while (1) {
        readf = master;
        if(select(fdmax+1, &readf, NULL, NULL, NULL) == -1)
        {
            perror("Server-select() error lol!");
            exit(1);
        }

        /*run through the existing connections looking for data to be read*/
        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &readf)) {
                if (i == listener) {
                    return CONNECTION;
                }else {
                    for(j = 0; j < ap->size; j++) {
                        if(i == ap->tab_proc[j]->proc_fd ) {
                            *proc = ap->tab_proc[j];
                            return APPLICATION;
                        }
                    }
                }
            }
        }
    }
} 

/*calculer le max filedescriptor*/
int getMaxFd(struct array_proc *ap) {
    int i;
    int max = 0;
    for (i = 0; i < ap->size; i++ ) {
        if (ap->tab_proc[i]->proc_fd > max) {
            max = ap->tab_proc[i]->proc_fd;
        } 
    }

    return max;
}