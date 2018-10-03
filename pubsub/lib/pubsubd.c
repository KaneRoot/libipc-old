#include "../../core/communication.h"
#include "../../core/msg.h"
#include "../../core/process.h"
#include "../../core/utils.h"
#include "../../core/error.h"

#include "pubsubd.h"
#include "channels.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

void pubsubd_send (const struct ipc_process_array *ap, const struct pubsub_msg * m)
{
    if (ap == NULL) {
        fprintf (stderr, "pubsubd_send: ap == NULL");
        return;
    }

    if (m == NULL) {
        fprintf (stderr, "pubsubd_send: m == NULL");
        return;
    }

    char *buf = NULL;
    size_t msize = 0;
    pubsub_message_serialize (m, &buf, &msize);

    struct ipc_message m_data;
    memset (&m_data, 0, sizeof (struct ipc_message));
    ipc_message_format_data (&m_data, buf, msize);

    int i;
    for (i = 0; i < ap->size ; i++) {
        ipc_server_write (ap->tab_proc[i], &m_data);
    }
    ipc_message_free (&m_data);

    if (buf != NULL) {
        free (buf);
    }
}

// void pubsubd_recv (struct ipc_process *p, struct pubsub_msg *m)
// {
//     struct ipc_message m_data;
//     memset (&m_data, 0, sizeof (struct ipc_message));
// 
//     // read the message from the process
//     ipc_server_read (p, &m_data);
// 
//     pubsub_message_unserialize (m, m_data.val, m_data.valsize);
// 
//     ipc_message_free (&m_data);
// }

/**
 * new connection, once accepted the process is added to the array_proc
 * structure to be checked periodically for new messages
 */
void handle_new_connection (struct service *srv, struct ipc_process_array *ap)
{
    struct ipc_process *p = malloc(sizeof(struct ipc_process));
    memset(p, 0, sizeof(struct ipc_process));

    if (ipc_server_accept (srv, p) < 0) {
        handle_error("server_accept < 0");
    } else {
        printf("new connection\n");
    }

    if (ipc_process_add (ap, p) < 0) {
        handle_error("ipc_process_add < 0");
    }
}

void handle_new_msg (struct channels *chans
        , struct ipc_process_array *ap, struct ipc_process_array *proc_to_read)
{
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
    int i;
    for (i = 0; i < proc_to_read->size; i++) {
        // printf ("loop handle_new_msg\n");
        if (ipc_server_read (proc_to_read->tab_proc[i], &m) < 0) {
            handle_error("server_read < 0");
        }

        mprint_hexa ("msg received: ", (unsigned char *) m.val, m.valsize);

        // close the process then delete it from the process array
        if (m.type == MSG_TYPE_CLOSE) {
            struct ipc_process *p = proc_to_read->tab_proc[i];

            printf ("proc %d disconnecting\n", p->proc_fd);

            // TODO: to test, unsubscribe when closing
            pubsubd_channels_unsubscribe_everywhere (chans, p);

            // close the connection to the process
            if (ipc_server_close_proc (p) < 0)
                handle_error( "server_close_proc < 0");


            // remove the process from the processes list
            if (ipc_process_del (ap, p) < 0)
                handle_error( "ipc_process_del < 0");
            if (ipc_process_del (proc_to_read, p) < 0)
                handle_err( "handle_new_msg", "ipc_process_del < 0");

            ipc_message_free (&m);

            // free process
            free (p);

            i--;
            continue;
        }

        struct pubsub_msg m_data;
        memset (&m_data, 0, sizeof (struct pubsub_msg));

        pubsub_message_unserialize (&m_data, m.val, m.valsize);

        if (m_data.type == PUBSUB_MSG_TYPE_SUB) {
            printf ("proc %d subscribing to %s\n"
                    , proc_to_read->tab_proc[i]->proc_fd
                    , m_data.chan);
            pubsubd_channels_subscribe (chans
                    , m_data.chan, proc_to_read->tab_proc[i]);
        }

        if (m_data.type == PUBSUB_MSG_TYPE_UNSUB) {
            printf ("proc %d unsubscribing to %s\n"
                    , proc_to_read->tab_proc[i]->proc_fd
                    , m_data.chan);
            pubsubd_channels_unsubscribe (chans
                    , m_data.chan, proc_to_read->tab_proc[i]);
        }

        if (m_data.type == PUBSUB_MSG_TYPE_PUB) {
            printf ("proc %d publishing to %s\n"
                    , proc_to_read->tab_proc[i]->proc_fd
                    , m_data.chan);
            struct channel *chan = pubsubd_channel_search (chans, m_data.chan);
            if (chan == NULL) {
                handle_err ("handle_new_msg", "publish on nonexistent channel");
                ipc_message_free (&m);
                return ;
            }
            pubsubd_send (chan->subs, &m_data);
        }

        pubsub_message_free (&m_data);
        ipc_message_free (&m);
    }
}

/*
 * main loop
 *
 * accept new application connections
 * read a message and send it back
 * close a connection if MSG_TYPE_CLOSE received
 */

void pubsubd_main_loop (struct service *srv, struct channels *chans)
{
    int i, ret = 0; 

    struct ipc_process_array ap;
    memset(&ap, 0, sizeof(struct ipc_process_array));

    struct ipc_process_array proc_to_read;
    memset(&proc_to_read, 0, sizeof(struct ipc_process_array));

    while(1) {
        ret = ipc_server_select (&ap, srv, &proc_to_read);

        if (ret == CONNECTION) {
            handle_new_connection (srv, &ap);
        } else if (ret == APPLICATION) {
            handle_new_msg (chans, &ap, &proc_to_read);
        } else { // both new connection and new msg from at least one client
            handle_new_connection (srv, &ap);
            handle_new_msg (chans, &ap, &proc_to_read);
        }
        ipc_process_array_free (&proc_to_read);
    }

    for (i = 0; i < ap.size; i++) {
        if (ipc_server_close_proc (ap.tab_proc[i]) < 0) {
            handle_error( "server_close_proc < 0");
        }
    }

    pubsubd_channels_del_all (chans);
}

