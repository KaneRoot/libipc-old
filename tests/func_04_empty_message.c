#include "../src/ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVICE_NAME "pong"
#define SECURE_MALLOC(p, s, wat) p = malloc (s); if (p == NULL) { wat; }

// test the behavior of the server when the client never read its messages

void send_message (struct ipc_ctx *ctx)
{
	SECURE_DECLARATION (struct ipc_message, m);
	SECURE_MALLOC (m.payload, 1, exit(EXIT_FAILURE));
	memcpy (m.payload, "", 0);
	m.type = MSG_TYPE_DATA;
	m.user_type = 42;
	m.length = 0;
	m.fd = ctx->pollfd[0].fd;

	// ipc_write_fd = write now, without waiting the fd to become available
	ipc_write_fd (ctx->pollfd[0].fd, &m);

	ipc_message_empty (&m);
}


void read_message (struct ipc_ctx *ctx)
{
	SECURE_DECLARATION (struct ipc_message, m);

	ipc_read (ctx, 0 /* there is only one valid index */, &m);
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

int main(void)
{
	SECURE_DECLARATION(struct ipc_ctx, ctx);

	TEST_IPC_Q(ipc_connection (&ctx, SERVICE_NAME), EXIT_FAILURE);

	send_message (&ctx);
	read_message (&ctx);

	TEST_IPC_Q(ipc_close_all (&ctx), EXIT_FAILURE);

    return EXIT_SUCCESS;
}
