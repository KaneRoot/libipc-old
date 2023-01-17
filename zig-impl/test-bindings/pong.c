#include <stdio.h>
#include <stdlib.h>

#define SERVICE "pong"
#define SERVICE_LEN 4

enum event_types {
	  ERROR = 0         // A problem occured.
	, EXTERNAL = 1      // Message received from a non IPC socket.
	, SWITCH_RX = 2     // Message received from a switched FD.
	, SWITCH_TX = 3     // Message sent to a switched fd.
	, CONNECTION = 4    // New user.
	, DISCONNECTION = 5 // User disconnected.
	, MESSAGE = 6       // New message.
	, TIMER = 7         // Timeout in the poll(2) function.
	, TX = 8            // Message sent.
};


int main(void) {
	direct_write_then_read();
	wait_event();
}

int direct_write_then_read(void) {
	int ret = 0;
	int servicefd = 0;

	printf ("Init context.\n");
	void *ctx = NULL;
	ret = ipc_context_init (&ctx);

	if (ret != 0) {
		printf ("Cannot init context.\n");
		return 1;
	}

	printf ("Connect to a 'pong' service.\n");
	ret = ipc_connect_service (ctx, &servicefd, SERVICE, SERVICE_LEN);

	if (ret != 0) {
		printf ("Cannot connect to a service.\n");
		return 1;
	}

	printf ("Let's send a message.\n");
	ret = ipc_write (ctx, servicefd, "hello, plz bounce me", 21);

	if (ret != 0) {
		printf ("Cannot write to the service.\n");
		return 1;
	}

	char message[10000];
	unsigned int size = 10000;

	ret = ipc_read_fd (ctx, servicefd, message, &size);

	if (ret != 0) {
		printf ("Cannot read from the service fd: %d.\n", ret);
		return 1;
	}

	if (size == 0) {
		printf ("No message returned.\n");
		return 1;
	}

	message[size] = '\0';
	printf ("Response: %s.\n", message);

	printf ("Deinit context\n");
	ipc_context_deinit (ctx);

	free(ctx);

	printf ("Context freed.\n");
	return 0;
}

int wait_event(void) {
	int ret = 0;
	int servicefd = 0;
	char message[10000];
	unsigned int size = 10000;
	char event_type;
	unsigned int index = 0;
	int originfd = 0;
	void *ctx = NULL;

	printf ("Init context.\n");
	ret = ipc_context_init (&ctx);

	if (ret != 0) {
		printf ("Cannot init context.\n");
		return 1;
	}

	printf ("Connect to a 'pong' service.\n");
	ret = ipc_connect_service (ctx, &servicefd, SERVICE, SERVICE_LEN);

	if (ret != 0) {
		printf ("Cannot connect to a service.\n");
		return 1;
	}

	printf ("Let's schedule a message.\n");
	ret = ipc_schedule (ctx, servicefd, "hello, plz bounce me", 21);

	if (ret != 0) {
		printf ("Cannot schedule a message.\n");
		return 1;
	}

	printf ("Let's loop over events.\n");
	char should_continue = 1;
	unsigned int count = 0;
	while(should_continue) {
		size = 10000;
		ret = ipc_wait_event (ctx, &event_type, &index, &originfd, message, &size);
		if (ret != 0) {
			printf ("Error while waiting for an event.\n");
			return 1;
		}

		printf ("EVENT %u\t", count++);

		switch ((enum event_types) event_type) {
		case ERROR: {
				printf ("Error.\n");
				return 1;
			}
		case EXTERNAL: {
				printf ("External (shouldn't happen).\n");
				return 1;
			}
		case SWITCH_RX: {
				printf ("Switch RX (shouldn't happen).\n");
				return 1;
			}
		case SWITCH_TX: {
				printf ("Switch TX (shouldn't happen).\n");
				return 1;
			}
		case CONNECTION: {
				printf ("Connection (shouldn't happen).\n");
				return 1;
			}
		case DISCONNECTION: {
				printf ("Disconnection (shouldn't happen).\n");
				return 1;
			}
		case TIMER: {
				printf ("TIMER.\n");
				break;
			}
		case TX: {
				printf ("A message has been sent.\n");
				break;
			}
		case MESSAGE: {
				if (size == 0) {
					printf ("No message returned.\n");
					return 1;
				}

				message[size] = '\0';
				printf ("Response: %s.\n", message);
				// We received the response, quitting.
				should_continue = 0;
				break;
			}
		}
	}

	printf ("Deinit context\n");
	ipc_context_deinit (ctx);

	free(ctx);

	printf ("Context freed.\n");
	return 0;
}
