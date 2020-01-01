#include "../src/ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVICE_NAME "pong"
#define SECURE_MALLOC(p, s, wat) p = malloc (s); if (p == NULL) { wat; }

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
	argc = argc;
	argv = argv;

	SECURE_DECLARATION(struct ipc_connection_info,srv1);

	TEST_IPC_Q(ipc_connection (env, &srv1, SERVICE_NAME), EXIT_FAILURE);

	send_message (&srv1);
	read_message (&srv1);

	TEST_IPC_Q(ipc_close (&srv1), EXIT_FAILURE);

    return EXIT_SUCCESS;
}
