#include <stdio.h>
#include <stdlib.h>

int main(void) {
	int ret = 0;

	printf ("Init context.\n");
	void *ctx = NULL;
	ret = ipc_context_init (&ctx);

	if (ret != 0) {
		printf ("Cannot init context.\n");
		return 1;
	}

	// TODO: do stuff

	printf ("Deinit context\n");
	ipc_context_deinit (ctx);
	printf ("Context deinit.\n");
	free(ctx);
	printf ("Context completely freed.\n");
	return 0;
}
