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
	SECURE_MALLOC (m.payload, 5, exit(EXIT_FAILURE));
	memcpy (m.payload, "salut", 5);
	m.type = MSG_TYPE_DATA;
	m.user_type = 42;
	m.length = 5;

	ipc_write_fd (ctx->pollfd[0].fd /* only one connection */, &m);

	ipc_message_empty (&m);
}


void read_message (struct ipc_ctx *ctx)
{
#if 0
	SECURE_DECLARATION(struct ipc_event, event);
	SECURE_DECLARATION (struct ipc_message, m);

	int timer = 10000;

	// ctx, index, message
	TEST_IPC_Q(ipc_read (ctx, 0 /* only one server here */, &m), EXIT_FAILURE);

	ipc_wait_event (ctx, &event, &timer);

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

	ipc_ctx_free (ctx);
#else
	SECURE_DECLARATION (struct ipc_message, m);

	TEST_IPC_Q(ipc_read (ctx, 0 /* only one server here */, &m), EXIT_FAILURE);
	printf ("received message: %*.s\n", m.length, m.payload);
	free (m.payload);
#endif
}

int main(void)
{
	SECURE_DECLARATION(struct ipc_ctx, ctx1);
	SECURE_DECLARATION(struct ipc_ctx, ctx2);
	SECURE_DECLARATION(struct ipc_event, event);

	TEST_IPC_Q (ipc_connection (&ctx1, SERVICE_NAME), EXIT_FAILURE);
	TEST_IPC_Q (ipc_connection (&ctx2, SERVICE_NAME), EXIT_FAILURE);

	send_message (&ctx1);
	read_message (&ctx1);

	TEST_IPC_Q (ipc_close_all (&ctx1), EXIT_FAILURE);

	send_message (&ctx2);
	read_message (&ctx2);

	TEST_IPC_Q (ipc_close_all (&ctx2), EXIT_FAILURE);

    return EXIT_SUCCESS;
}
