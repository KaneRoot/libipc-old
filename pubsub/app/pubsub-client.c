#include "../../core/ipc.h"

#include "../lib/message.h"
#include "../lib/channels.h"

#include "../lib/pubsub.h"
#include "../lib/pubsubd.h"

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void usage (char **argv) {
    printf ( "usage: %s [chan [pub]]\n", argv[0]);
}

void print_cmd (void) {
    printf ("\033[32m>\033[00m ");
    fflush (stdout);
}

void chan_sub (struct ipc_service *srv, char *chan)
{
    struct pubsub_msg msg;
    memset (&msg, 0, sizeof (struct pubsub_msg));

    // meta data on the message
    msg.type = PUBSUB_MSG_TYPE_SUB;
    msg.chanlen = strlen (chan) + 1;
    msg.chan = malloc (msg.chanlen);
    memset (msg.chan, 0, msg.chanlen);
    strncpy (msg.chan, chan, msg.chanlen);
    msg.chan[strlen (chan)] = '\0';

    pubsub_message_send (srv, &msg);
    printf ("subscribed to %s\n", chan);

    pubsub_message_free (&msg);
}

void main_loop (char **env, int index, int version
        , char *cmd, char *chan)
{
    printf ("connection to pubsubd: index %d version %d "
            "cmd %s chan %s\n"
          , index, version, cmd, chan );

    struct ipc_service srv;
    memset (&srv, 0, sizeof (struct ipc_service));
    int ret = ipc_application_connection (env, &srv, PUBSUBD_SERVICE_NAME);
    if (ret != 0) {
        handle_err ("pubsub_connection", "application_connection != 0");
    }
    printf ("connected\n");

    if (strncmp (cmd, "sub", 3) == 0) {
        chan_sub (&srv, chan);
    }

    printf ("main_loop\n");

    struct pubsub_msg msg;
    memset (&msg, 0, sizeof (struct pubsub_msg));

    // meta data on the message
    msg.type = PUBSUB_MSG_TYPE_PUB;
    msg.chanlen = strlen (chan) + 1;
    msg.chan = malloc (msg.chanlen);
    memset (msg.chan, 0, msg.chanlen);
    strncpy ((char *) msg.chan, chan, msg.chanlen);
    msg.chan[strlen (chan)] = '\0';

	struct ipc_event event;
	memset (&event, 0, sizeof (struct ipc_event));

	struct ipc_services services;
	memset (&services, 0, sizeof (struct ipc_services));
	ipc_service_add (&services, &srv);

	int should_continue = 1;

    while (should_continue) {
		print_cmd ();
		ret = ipc_application_peek_event (&services, &event);

		if (ret != 0) {
			handle_error("ipc_application_peek_event != 0");
			exit (EXIT_FAILURE);
		}

		switch (event.type) {
			case IPC_EVENT_TYPE_STDIN:
				{
					struct ipc_message *m = event.m;
					if ( m->length == 0 || strncmp (m->payload, "exit", 4) == 0) {
						// TODO: disconnection

						ipc_message_empty (m);
						free (m);

						should_continue = 0;
						break;
					}

					// get the curent payload, change it to be compatible with the application
					// then send it
					print_cmd ();

					// TODO: remove \n
					msg.datalen = m->length + 1;
					msg.data = malloc (msg.datalen);
					memset (msg.data, 0, msg.datalen);
					strncpy (msg.data, m->payload, msg.datalen);
					msg.data[msg.datalen] = '\0';

					pubsub_message_send (&srv, &msg);
					free (msg.data);
					msg.data = NULL;
					msg.datalen = 0;
				}
				break;
			case IPC_EVENT_TYPE_MESSAGE:
				{
					struct ipc_message *m = event.m;
					printf ("msg recv: %.*s", m->length, m->payload);
					// TODO: correctly print the message
				};
				break;
			case IPC_EVENT_TYPE_DISCONNECTION:
				{
					printf ("server disconnected: quitting...\n");

					// just remove srv from services, it's already closed
					ipc_services_free (&services);

					exit (EXIT_SUCCESS);
				};
			case IPC_EVENT_TYPE_NOT_SET:
			case IPC_EVENT_TYPE_CONNECTION:
			case IPC_EVENT_TYPE_ERROR:
			default :
				fprintf (stderr, "should not happen, event type %d\n", event.type);
		}
    }

    // free everything
    pubsub_message_free (&msg);

    printf ("disconnection...\n");
	ipc_services_free (&services);
	if (ipc_application_close (&srv) < 0) {
		handle_err("main", "application_close < 0");
		exit (EXIT_FAILURE);
	}
}

int main(int argc, char **argv, char **env)
{
    char *cmd = "sub";
    char *chan = "chan1";

    if (argc == 2 && strncmp("-h", argv[1], 2) == 0) {
        usage (argv);
        exit (0);
    }

    if (argc >= 2) {
        chan = argv[1];
    }

    if (argc >= 3) {
        cmd = argv[2];
    }

    int index = 0;
    // don't care about the version
    int version = 0;

    main_loop (env, index, version, cmd, chan);

    return EXIT_SUCCESS;
}
