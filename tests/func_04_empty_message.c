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
	SECURE_MALLOC (m.payload, 1, exit(EXIT_FAILURE));
	memcpy (m.payload, "", 0);
	m.type = MSG_TYPE_DATA;
	m.user_type = 42;
	m.length = 0;

	ipc_write (ci, &m);

	ipc_message_empty (&m);
}


void read_message (struct ipc_connection_info *ci)
{
	SECURE_DECLARATION (struct ipc_message, m);

	ipc_read (ci, &m);
	if (m.length > 0) {
		printf ("received message: %*.s\n", m.length, m.payload);
	}
	else {
		printf ("received empty message as intended : %d bytes\n", m.length);
		if (m.payload == NULL) {
			printf ("message payload is NULL\n");
		}
	}
	free (m.payload);
}

int main(int argc, char * argv[], char **env)
{
	SECURE_DECLARATION(struct ipc_connection_info,srv1);

	connection (env, &srv1);
	send_message (&srv1);
	read_message (&srv1);
	closing (&srv1);

    return EXIT_SUCCESS;
}
