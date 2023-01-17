#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libipc.h"

#define SERVICE "pong"
#define SERVICE_LEN 4

int direct_write_then_read(void);
int wait_event(void);

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
		printf ("Deinit context\n");
		ipc_context_deinit (ctx);
		free(ctx);
		return 1;
	}

	printf ("Let's send a message.\n");
	ret = ipc_write (ctx, servicefd, "hello, plz bounce me", 21);

	if (ret != 0) {
		printf ("Cannot write to the service.\n");
		printf ("Deinit context\n");
		ipc_context_deinit (ctx);
		free(ctx);
		return 1;
	}

	char message[10000];
	size_t size = 10000;

	ret = ipc_read_fd (ctx, servicefd, message, &size);

	if (ret != 0) {
		printf ("Cannot read from the service fd: %d.\n", ret);
		printf ("Deinit context\n");
		ipc_context_deinit (ctx);
		free(ctx);
		return 1;
	}

	if (size == 0) {
		printf ("No message returned.\n");
		printf ("Deinit context\n");
		ipc_context_deinit (ctx);
		free(ctx);
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
	memset (message, 0, 1000);
	size_t size = 10000;
	char event_type;
	size_t index = 0;
	int originfd = 0;
	void *ctx = NULL;

	printf ("Init context.\n");
	ret = ipc_context_init (&ctx);

	if (ret != 0) {
		printf ("Cannot init context.\n");
		printf ("Deinit context\n");
		ipc_context_deinit (ctx);
		free(ctx);
		return 1;
	}

	printf ("Connect to a 'pong' service.\n");
	ret = ipc_connect_service (ctx, &servicefd, SERVICE, SERVICE_LEN);

	if (ret != 0) {
		printf ("Cannot connect to a service.\n");
		printf ("Deinit context\n");
		ipc_context_deinit (ctx);
		free(ctx);
		return 1;
	}

	printf ("Let's schedule a message.\n");
	ret = ipc_schedule (ctx, servicefd, "hello, plz bounce me", 21);

	if (ret != 0) {
		printf ("Cannot schedule a message.\n");
		printf ("Deinit context\n");
		ipc_context_deinit (ctx);
		free(ctx);
		return 1;
	}

	printf ("Let's set the timer to one second.\n");
	ipc_context_timer (ctx, 1000);

	printf ("Let's loop over events.\n");
	char should_continue = 1;
	size_t count = 0;
	while(should_continue) {
		size = 10000;
		ret = ipc_wait_event (ctx, &event_type, &index, &originfd, message, &size);
		if (ret != 0) {
			printf ("Error while waiting for an event.\n");
			return 1;
		}

		printf ("EVENT %lu\t", count++);

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
				printf ("Response (size %lu): %s.\n", size, message);
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
