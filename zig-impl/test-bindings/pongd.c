#include <stdio.h>
#include <stdlib.h>

#include "../libipc.h"

#define SERVICE "pong"
#define SERVICE_LEN 4


int main(void) {
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

	printf ("Create a 'pong' service.\n");
	ret = ipc_service_init (ctx, &servicefd, SERVICE, SERVICE_LEN);

	if (ret != 0) {
		printf ("Cannot create a service.\n");
		return 1;
	}

	printf ("Set the timer to ten seconds.\n");
	ipc_context_timer (ctx, 10000);

	printf ("Loop over events.\n");
	char should_continue = 1;
	unsigned int count = 0;
	unsigned int count_timer = 0;
	while(should_continue) {
		size = 10000;
		ret = ipc_wait_event (ctx, &event_type, &index, &originfd, message, &size);
		if (ret != 0) {
			printf ("Error while waiting for an event.\n");
			return 1;
		}

		if ((enum event_types) event_type != TIMER) {
			printf ("EVENT %u\t", count++);
		}

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
				printf ("New connection.\n");
				break;
			}
		case DISCONNECTION: {
				printf ("User disconnected.\n");
				break;
			}
		case TIMER: {
				printf ("\rTIMER (%d).", count_timer++);
				fflush(stdout);
				break;
			}
		case TX: {
				printf ("Message sent.\n");
				break;
			}
		case MESSAGE: {
				if (size == 0) {
					printf ("Error: no message returned.\n");
					should_continue = 0;
				}
				else {
					message[size] = '\0';
					printf ("Message received: %s.\n", message);
					printf ("Scheduling this message.\n");

					ret = ipc_schedule (ctx, originfd, message, size);
					if (ret != 0) {
						printf ("Cannot schedule a message.\n");
						should_continue = 0;
					}
				}
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
