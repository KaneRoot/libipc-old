#include "communication.h"
#include "usocket.h"
#include "utils.h"
#include "error.h"

#include <assert.h>
#include <stdio.h>
#include <errno.h>

void service_path (char *path, const char *sname, int index, int version)
{
    assert (path != NULL);
    assert (sname != NULL);
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

int srv_accept (struct service *srv, struct process *p)
{
    assert (srv != NULL);
    assert (p != NULL);

    usock_accept (srv->service_fd, &p->proc_fd);

    struct msg m_con;
    memset (&m_con, 0, sizeof (struct msg));
    srv_read (p, &m_con);
    // TODO: handle the parameters in the first message
    msg_free (&m_con);

    struct msg m_ack;
    memset (&m_ack, 0, sizeof (struct msg));
    msg_format_ack (&m_ack, NULL, 0);
    srv_write (p, &m_ack);
    msg_free (&m_ack);

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

int srv_read (const struct process *p, struct msg *m)
{
    return msg_read (p->proc_fd, m);
}

int srv_write (const struct process *p, const struct msg *m)
{
    return msg_write (p->proc_fd, m);
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

    // send connection string and receive acknowledgement
    struct msg m_con;
    memset (&m_con, 0, sizeof (struct msg));

    // format the connection msg
    if (msg_format_con (&m_con, connectionstr, msize) < 0) {
        handle_err ("app_connection", "msg_format_con");
        return -1;
    }

    // send connection msg
    app_write (srv, &m_con);
    msg_free (&m_con);

    // receive ack msg
    struct msg m_ack;
    memset (&m_ack, 0, sizeof (struct msg));
    app_read (srv, &m_ack);

    assert (m_ack.type == MSG_TYPE_ACK);
    assert (m_ack.valsize == 0);
    msg_free (&m_ack);

    return 0;
}

int app_close (struct service *srv)
{
    struct msg m;
    memset (&m, 0, sizeof (struct msg));
    m.type = MSG_TYPE_DIS;
    if (msg_write (srv->service_fd, &m) < 0) {
        handle_err ("app_close", "msg_write < 0");
    }

    return usock_close (srv->service_fd);
}

int app_read (struct service *srv, struct msg *m)
{   
    return msg_read (srv->service_fd, m);
}

int app_write (struct service *srv, const struct msg *m)
{
    return msg_write (srv->service_fd, m);
}


/*
 * srv_select prend en parametre
 *  * un tableau de process qu'on écoute
 *  * le service qui attend de nouvelles connexions
 *  * un tableau de process qui souhaitent parler
 *
 * la fonction trouve le processus/service actif et renvoie 
 * un entier correspondant à quel descripteur de fichier il faut lire
 *  * celui du serveur = nouvelle connexion entrante        (CONNECTION)
 *  * celui d'un ou plusieurs processus = ils nous parlent  (APPLICATION)
 *  * les deux à la fois                                    (CON_APP)
 */

int srv_select (struct array_proc *ap, struct service *srv
        , struct process **proc)
{
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
        if(select(fdmax+1, &readf, NULL, NULL, NULL) == -1) {
            perror("select");
            return -1;
        }

        /*run through the existing connections looking for data to be read*/
        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &readf)) {
                if (i == listener) {
                    return CONNECTION;
                } else {
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
int getMaxFd(struct array_proc *ap)
{

    int i;
    int max = 0;

    for (i = 0; i < ap->size; i++ ) {
        if (ap->tab_proc[i]->proc_fd > max) {
            max = ap->tab_proc[i]->proc_fd;
        } 
    }

    return max;
}
