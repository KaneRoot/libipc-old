#include <stdio.h>
#include <stdlib.h>

#define SERVICE "pong"
#define SERVICE_LEN 4

int main(void) {
	int ret = 0;
	int servicefd = 0;

	printf ("Init context.\n");
	void *ctx = NULL;
	ret = ipc_context_init (&ctx);

	if (ret != 0) {
		printf ("Cannot init context.\n");
		return 1;
	}

	printf ("Context initiated.\n");

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

#if 0
	// TODO: loop over ipc_wait
	int event_type, index, originfd = 0;
	unsigned int size = 0;
	char message[10000];
	printf ("Wait for a response.\n", ret);
	ret = ipc_wait (&event_type, &index, &originfd, &size, message);

	if (ret != 0) {
		printf ("Error while waiting for an event.\n");
		return 1;
	}

	if (size == 0) {
		printf ("No message returned.\n");
		return 1;
	}

	message[size] = '\0';
	printf ("Response: %s.\n", message);
#endif

	printf ("Deinit context\n");
	ipc_context_deinit (ctx);

	free(ctx);

	printf ("Context freed.\n");
	return 0;
}
