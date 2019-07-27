#include "../src/ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVICE_NAME "pong"
#define SECURE_MALLOC(p, s, wat) p = malloc (s); if (p == NULL) { wat; }

void connection (char **env, struct ipc_connection_info *ci)
{
	enum ipc_errors ret = ipc_connection (env, ci, SERVICE_NAME);
	if (ret != IPC_ERROR_NONE) {
		fprintf (stderr, "cannot connect to the server\n");
		exit(EXIT_FAILURE);
	}
}

void closing (struct ipc_connection_info *ci)
{
	enum ipc_errors ret = ipc_close (ci);
	if (ret != IPC_ERROR_NONE) {
		fprintf (stderr, "cannot close server\n");
		exit(EXIT_FAILURE);
	}
}

// test the behavior of the server when the client never read its messages

void send_message (struct ipc_connection_info *ci)
{
	SECURE_DECLARATION (struct ipc_message, m);
	SECURE_MALLOC (m.payload, 5, exit(EXIT_FAILURE));
	memcpy (m.payload, "salut", 5);
	m.type = MSG_TYPE_DATA;
	m.user_type = 42;
	m.length = 5;

	ipc_write (ci, &m);

	ipc_message_empty (&m);
}


void read_message (struct ipc_connection_info *ci)
{
#if 0
	SECURE_DECLARATION(struct ipc_event, event);
	SECURE_DECLARATION(struct ipc_connection_infos, clients);

	ipc_add (&clients, ci);

	ipc_wait_event (&clients, NULL, &event);

	switch (event.type) {
		case IPC_EVENT_TYPE_MESSAGE : {
				printf ("received message: %*.s\n", m.length, ((struct ipc_message*) event.m)->payload);
			}
			break;
		case IPC_EVENT_TYPE_CONNECTION :
		case IPC_EVENT_TYPE_DISCONNECTION : 
		case IPC_EVENT_TYPE_NOT_SET :
		case IPC_EVENT_TYPE_ERROR :
		case IPC_EVENT_TYPE_EXTRA_SOCKET :
		default :
			printf ("not ok - should not happen\n");
			exit (EXIT_FAILURE);
			break;
	}

	ipc_connections_free (&clients);
#else
	SECURE_DECLARATION (struct ipc_message, m);

	ipc_read (ci, &m);
	printf ("received message: %*.s\n", m.length, m.payload);
	free (m.payload);
#endif
}

int main(int argc, char * argv[], char **env)
{
	SECURE_DECLARATION(struct ipc_connection_info,srv1);
	SECURE_DECLARATION(struct ipc_connection_info,srv2);
	SECURE_DECLARATION(struct ipc_event, event);

	connection (env, &srv1);
	connection (env, &srv2);
	send_message (&srv1);
	read_message (&srv1);
	closing (&srv1);
	send_message (&srv2);
	read_message (&srv2);
	closing (&srv2);

    return EXIT_SUCCESS;
}
