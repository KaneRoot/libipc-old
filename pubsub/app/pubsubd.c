#include "../../core/ipc.h"
#include "../lib/message.h"
#include "../lib/channels.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>

#define PUBSUBD_SERVICE_NAME "pubsubd"

// to quit them properly if a signal occurs
struct ipc_service srv;
struct channels chans;

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
					// TODO: handle a message
					struct ipc_message *m = event.m;
					print_hexa ("received msg hexa", m->payload, m->length);
					struct ipc_client *cli = event.origin;

					struct pubsub_msg pm;
					memset (&pm, 0, sizeof (struct pubsub_msg));

					pubsub_message_from_message (&pm, m);

					if (pm.type == PUBSUB_MSG_TYPE_SUB) {
						printf ("client %d subscribing to %s\n"
								, cli->proc_fd
								, pm.chan);
						pubsubd_channels_subscribe (chans
								, pm.chan, cli);
					}

					if (pm.type == PUBSUB_MSG_TYPE_UNSUB) {
						printf ("client %d unsubscribing to %s\n", cli->proc_fd, pm.chan);
						pubsubd_channels_unsubscribe (chans, pm.chan, cli);
					}

					if (pm.type == PUBSUB_MSG_TYPE_PUB) {
						// printf ("client %d: publishing to %s: %s\n", cli->proc_fd, pm.chan, pm.data);
						printf ("client %d: ", cli->proc_fd);
						pubsub_message_print (&pm);

						struct channel *chan = pubsubd_channel_search (chans, pm.chan);
						if (chan == NULL) {
							handle_err ("handle_new_msg", "publish on nonexistent channel");
							ipc_message_empty (m);
							continue;
						}
						pubsubd_send (chan->subs, &pm);
					}

					pubsub_message_empty (&pm);
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


void handle_signal (int signalnumber)
{
    // the application will shut down, and remove the service named pipe
    if (ipc_server_close (&srv) < 0) {
        handle_error("ipc_server_close < 0");
    }

    pubsubd_channels_del_all (&chans);

    fprintf (stderr, "received a signal %d\n", signalnumber);
    exit (EXIT_SUCCESS);
}

int
main(int argc, char **argv, char **env)
{
	argc = argc;
	argv = argv;

    memset (&srv, 0, sizeof (struct ipc_service));
    srv.index = 0;
    srv.version = 0;

    signal(SIGHUP, handle_signal);
    signal(SIGINT, handle_signal);
    signal(SIGQUIT, handle_signal);

    memset (&chans, 0, sizeof (struct channels));
    pubsubd_channels_init (&chans);

    if (ipc_server_init (env, &srv, PUBSUBD_SERVICE_NAME) < 0) {
        handle_error("ipc_server_init < 0");
        return EXIT_FAILURE;
    }
    printf ("Listening on %s.\n", srv.spath);

    printf("MAIN: server created\n" );

    // the service will loop until the end of time, a specific message, a signal
    pubsubd_main_loop (&srv, &chans);

    // the application will shut down, and remove the service named pipe
    if (ipc_server_close (&srv) < 0) {
        handle_error("ipc_server_close < 0");
    }

    return EXIT_SUCCESS;
}
