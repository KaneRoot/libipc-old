#include "../../core/communication.h"
#include "../../core/msg.h"
#include "../../core/client.h"
#include "../../core/utils.h"
#include "../../core/error.h"
#include "../../core/logger.h"

#include "remoted.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

/**
 * new connection, once accepted the client is added to the array_proc
 * structure to be checked periodically for new messages
 */
void handle_new_connection (struct ipc_service *srv, struct ipc_clients *ap)
{
    struct ipc_client *p = malloc(sizeof(struct ipc_client));
    memset(p, 0, sizeof(struct ipc_client));

    if (ipc_server_accept (srv, p) < 0) {
        handle_error("ipc_server_accept < 0");
    } else {
        log_debug ("remoted, new connection", p->proc_fd);
    }

    if (ipc_client_add (ap, p) < 0) {
        handle_error("ipc_client_add < 0");
    }
}

void handle_new_msg (struct ipc_clients *ap, struct ipc_clients *proc_to_read)
{
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
    int i;
    for (i = 0; i < proc_to_read->size; i++) {
        if (ipc_server_read (proc_to_read->clients[i], &m) < 0) {
            handle_error("ipc_server_read < 0");
        }

        mprint_hexa ("msg received: ", (unsigned char *) m.payload, m.length);

        // close the client then delete it from the client array
        if (m.type == MSG_TYPE_CLOSE) {
            struct ipc_client *p = proc_to_read->clients[i];

            log_debug ("remoted, client %d disconnecting", p->proc_fd);

            // close the connection to the client
            if (ipc_server_close_client (p) < 0)
                handle_error( "ipc_server_close_client < 0");

            // remove the client from the clientes list
            if (ipc_client_del (ap, p) < 0)
                handle_error( "ipc_client_del < 0");
            if (ipc_client_del (proc_to_read, p) < 0)
                handle_err( "handle_new_msg", "ipc_client_del < 0");

            ipc_message_free (&m);

            // free client
            free (p);

            i--;
            continue;
        }

#if 0
        struct pubsub_msg m_data;
        memset (&m_data, 0, sizeof (struct pubsub_msg));

        pubsub_message_unserialize (&m_data, m.payload, m.length);

        if (m_data.type == PUBSUB_MSG_TYPE_SUB) {
            printf ("client %d subscribing to %s\n"
                    , proc_to_read->clients[i]->proc_fd
                    , m_data.chan);
            pubsubd_channels_subscribe (chans
                    , m_data.chan, proc_to_read->clients[i]);
        }

        if (m_data.type == PUBSUB_MSG_TYPE_UNSUB) {
            printf ("client %d unsubscribing to %s\n"
                    , proc_to_read->clients[i]->proc_fd
                    , m_data.chan);
            pubsubd_channels_unsubscribe (chans
                    , m_data.chan, proc_to_read->clients[i]);
        }

        if (m_data.type == PUBSUB_MSG_TYPE_PUB) {
            printf ("client %d publishing to %s\n"
                    , proc_to_read->clients[i]->proc_fd
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
#endif
        ipc_message_free (&m);
    }
}

void remoted_main_loop (struct ipc_service *srv, struct remoted_ctx *ctx)
{
    log_debug ("remoted entering main loop");
    int i, ret = 0; 

    struct ipc_clients ap;
    memset(&ap, 0, sizeof(struct ipc_clients));

    struct ipc_clients proc_to_read;
    memset(&proc_to_read, 0, sizeof(struct ipc_clients));

    while(1) {
        /* TODO: authorizations */
        ret = ipc_server_select (&ap, srv, &proc_to_read);

        if (ret == CONNECTION) {
            handle_new_connection (srv, &ap);
        } else if (ret == APPLICATION) {
            handle_new_msg (&ap, &proc_to_read);
        } else { // both new connection and new msg from at least one client
            handle_new_connection (srv, &ap);
            handle_new_msg (&ap, &proc_to_read);
        }
        ipc_client_array_free (&proc_to_read);
    }

    for (i = 0; i < ap.size; i++) {
        if (ipc_server_close_client (ap.clients[i]) < 0) {
            handle_error( "ipc_server_close_client < 0");
        }
    }
}

void remoted_free_ctx (struct remoted_ctx *ctx)
{
    if (ctx->unix_socket_dir != NULL)
        free (ctx->unix_socket_dir), ctx->unix_socket_dir = NULL;
}
