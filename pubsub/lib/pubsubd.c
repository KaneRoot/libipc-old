#include "../../core/communication.h"
#include "../../core/message.h"
#include "../../core/client.h"
#include "../../core/utils.h"
#include "../../core/error.h"

#include "pubsubd.h"
#include "channels.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

void pubsubd_send (const struct ipc_clients *clients, const struct pubsub_msg * pubsub_msg)
{
    if (clients == NULL) {
        fprintf (stderr, "pubsubd_send: clients == NULL");
        return;
    }

    if (pubsub_msg == NULL) {
        fprintf (stderr, "pubsubd_send: pubsub_msg == NULL");
        return;
    }

    char *buf = NULL;
    size_t msize = 0;
    pubsub_message_serialize (pubsub_msg, &buf, &msize);

    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
    ipc_message_format_data (&m, buf, msize);

    int i;
    for (i = 0; i < clients->size ; i++) {
        ipc_server_write (clients->clients[i], &m);
    }
    ipc_message_empty (&m);

    if (buf != NULL) {
        free (buf);
    }
}

// void pubsubd_recv (struct ipc_client *p, struct pubsub_msg *m)
// {
//     struct ipc_message m_data;
//     memset (&m_data, 0, sizeof (struct ipc_message));
// 
//     // read the message from the client
//     ipc_server_read (p, &m_data);
// 
//     pubsub_message_unserialize (m, m_data.payload, m_data.length);
// 
//     ipc_message_empty (&m_data);
// }

void pubsubd_main_loop (struct ipc_service *srv, struct channels *chans)
{
    int i, ret = 0; 

    struct ipc_clients clients;
    memset(&clients, 0, sizeof(struct ipc_clients));

    struct ipc_clients proc_to_read;
    memset(&proc_to_read, 0, sizeof(struct ipc_clients));

	struct ipc_event event;
	memset(&event, 0, sizeof (struct ipc_event));
	event.type = IPC_EVENT_TYPE_NOT_SET;

	int cpt = 0;

    while(1) {
		ret = ipc_service_poll_event (&clients, srv, &event);
		if (ret != 0) {
			handle_error("ipc_service_poll_event != 0");
			// the application will shut down, and close the service
			if (ipc_server_close (srv) < 0) {
				handle_error("ipc_server_close < 0");
			}
			exit (EXIT_FAILURE);
		}

		switch (event.type) {
			case IPC_EVENT_TYPE_CONNECTION:
				{
					cpt++;
					struct ipc_client *cli = event.origin;
					printf ("connection of client %d: %d clients connected\n", cli->proc_fd, cpt);
				};
				break;
			case IPC_EVENT_TYPE_DISCONNECTION:
				{
					cpt--;
					struct ipc_client *cli = event.origin;
					printf ("disconnection of client %d: %d clients remaining\n", cli->proc_fd, cpt);

					// TODO: to test, unsubscribe when closing
					pubsubd_channels_unsubscribe_everywhere (chans, cli);

					// free the ipc_client structure
					free (event.origin);
				};
				break;
			case IPC_EVENT_TYPE_MESSAGE:
			   	{
					struct ipc_message *m = event.m;
					struct ipc_client *cli = event.origin;
					if (m->length > 0) {
						printf ("message received (type %d): %.*s\n", m->type, m->length, m->payload);
					}
					// TODO: handle a message
					// handle_new_msg (chans, &clients, &proc_to_read);

					struct pubsub_msg pubsub_msg;
					memset (&pubsub_msg, 0, sizeof (struct pubsub_msg));

					pubsub_message_unserialize (&pubsub_msg, m->payload, m->length);

					if (pubsub_msg.type == PUBSUB_MSG_TYPE_SUB) {
						printf ("client %d subscribing to %s\n"
								, cli->proc_fd
								, pubsub_msg.chan);
						pubsubd_channels_subscribe (chans
								, pubsub_msg.chan, cli);
					}

					if (pubsub_msg.type == PUBSUB_MSG_TYPE_UNSUB) {
						printf ("client %d unsubscribing to %s\n", cli->proc_fd, pubsub_msg.chan);
						pubsubd_channels_unsubscribe (chans, pubsub_msg.chan, cli);
					}

					if (pubsub_msg.type == PUBSUB_MSG_TYPE_PUB) {
						printf ("client %d publishing to %s\n", cli->proc_fd, pubsub_msg.chan);

						struct channel *chan = pubsubd_channel_search (chans, pubsub_msg.chan);
						if (chan == NULL) {
							handle_err ("handle_new_msg", "publish on nonexistent channel");
							ipc_message_empty (m);
							continue;
						}
						pubsubd_send (chan->subs, &pubsub_msg);
					}

					pubsub_message_free (&pubsub_msg);
				};
				break;
			case IPC_EVENT_TYPE_ERROR:
			   	{
					fprintf (stderr, "a problem happened with client %d\n"
							, ((struct ipc_client*) event.origin)->proc_fd);
				};
				break;
			default :
				{
					fprintf (stderr, "there must be a problem, event not set\n");
				};
		}
    }

    for (i = 0; i < clients.size; i++) {
        if (ipc_server_close_client (clients.clients[i]) < 0) {
            handle_error( "server_close_client < 0");
        }
    }

    pubsubd_channels_del_all (chans);
}

