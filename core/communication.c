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

int ipc_server_init (int argc, char **argv, char **env
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

    return usock_init (&srv->service_fd, srv->spath);
}

int ipc_server_accept (struct service *srv, struct process *p)
{
    assert (srv != NULL);
    assert (p != NULL);

    usock_accept (srv->service_fd, &p->proc_fd);

    struct msg m_con;
    memset (&m_con, 0, sizeof (struct msg));
    ipc_server_read (p, &m_con);
    // TODO: handle the parameters in the first message
    msg_free (&m_con);

    struct msg m_ack;
    memset (&m_ack, 0, sizeof (struct msg));
    msg_format_ack (&m_ack, NULL, 0);
    ipc_server_write (p, &m_ack);
    msg_free (&m_ack);

    return 0;
}

int ipc_server_close (struct service *srv)
{
    usock_close (srv->service_fd);
    return usock_remove (srv->spath);
}

int ipc_server_close_proc (struct process *p)
{
    // struct msg m_ack_dis;
    // memset (&m_ack_dis, 0, sizeof (struct msg));
    // m_ack_dis.type = MSG_TYPE_CLOSE;

    // if (msg_write (p->proc_fd, &m_ack_dis) < 0) {
    //     handle_err ("server_close_proc", "msg_write < 0");
    // }

    return usock_close (p->proc_fd);
}

int ipc_server_read (const struct process *p, struct msg *m)
{
    return msg_read (p->proc_fd, m);
}

int ipc_server_write (const struct process *p, const struct msg *m)
{
    return msg_write (p->proc_fd, m);
}

int ipc_application_connection (int argc, char **argv, char **env
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
        handle_err ("application_connection", "msg_format_con");
        return -1;
    }

    // send connection msg
    ipc_application_write (srv, &m_con);
    msg_free (&m_con);

    // receive ack msg
    struct msg m_ack;
    memset (&m_ack, 0, sizeof (struct msg));
    ipc_application_read (srv, &m_ack);

    assert (m_ack.type == MSG_TYPE_ACK);
    assert (m_ack.valsize == 0);
    msg_free (&m_ack);

    return 0;
}

// send a CLOSE message then close the socket
int ipc_application_close (struct service *srv)
{
    struct msg m;
    memset (&m, 0, sizeof (struct msg));
    m.type = MSG_TYPE_CLOSE;
    if (msg_write (srv->service_fd, &m) < 0) {
        handle_err ("application_close", "msg_write < 0");
    }

    return usock_close (srv->service_fd);
}

int ipc_application_read (struct service *srv, struct msg *m)
{   
    return msg_read (srv->service_fd, m);
}

int ipc_application_write (struct service *srv, const struct msg *m)
{
    return msg_write (srv->service_fd, m);
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

/*
 * ipc_server_select prend en parametre
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

int ipc_server_select (struct array_proc *ap, struct service *srv
        , struct array_proc *proc)
{
    assert (ap != NULL);
    assert (proc != NULL);

    // delete previous read process array
    array_proc_free (proc);

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

    for (i=0; i < ap->size; i++) {
        FD_SET(ap->tab_proc[i]->proc_fd, &master);
    }

    /* keep track of the biggest file descriptor */
    fdmax = getMaxFd(ap) > srv->service_fd ? getMaxFd(ap) : srv->service_fd; 

    int is_listener = 0;

    do {
        // printf ("loop ipc_server_select main_loop\n");
        readf = master;
        if(select(fdmax+1, &readf, NULL, NULL, NULL) == -1) {
            perror("select");
            return -1;
        }

        /*run through the existing connections looking for data to be read*/
        for (i = 0; i <= fdmax; i++) {
            // printf ("loop ipc_server_select inner loop\n");
            if (FD_ISSET(i, &readf)) {
                if (i == listener) {
                    is_listener = 1;
                } else {
                    for(j = 0; j < ap->size; j++) {
                        // printf ("loop ipc_server_select inner inner loop\n");
                        if(i == ap->tab_proc[j]->proc_fd ) {
                            add_proc (proc, ap->tab_proc[j]);
                        }
                    }
                }
            }
        }
    } while (0);

    if (proc->size > 0 && is_listener)
        return CON_APP;
    if (proc->size > 0)
        return APPLICATION;
    return CONNECTION;
} 
