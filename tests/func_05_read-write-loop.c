#define _BSD_SOURCE

#include "../src/ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define SERVICE_NAME "pong"
#define SECURE_MALLOC(p, s, wat) p = malloc (s); if (p == NULL) { wat; }
#define DEFAULT_MSG "coucou"


int main(int argc, char * argv[], char **env)
{
	SECURE_DECLARATION(struct ipc_connection_info,srv);
	SECURE_DECLARATION(struct ipc_event, event);

	SECURE_DECLARATION (struct ipc_message, write_m);
	SECURE_DECLARATION (struct ipc_message, read_m);

	// timing the exchange duration
	struct timeval tval_before, tval_after, tval_result;

	size_t nb_rounds = 1000;
	char *message_str = NULL;

	if (argc > 1 && strncmp(argv[1], "-h", 2) == 0) {
		fprintf (stderr, "usage: %s [nb-rounds [message]]\n", argv[0]);
		fprintf (stderr, "default: nb-rounds = 1000, message='coucou'\n");
		exit(0);
	}

	if (argc > 1) {
		nb_rounds = (size_t) atoi(argv[1]);
	}

	if (argc > 2) {
		message_str = argv[2];
	}

	if (message_str == NULL) {
		// message par defaut
		SECURE_MALLOC (message_str, strlen(DEFAULT_MSG) +1, exit(EXIT_FAILURE));
		memcpy(message_str, DEFAULT_MSG, strlen(DEFAULT_MSG));
	}

	TEST_IPC_Q (ipc_connection (env, &srv, SERVICE_NAME), EXIT_FAILURE);

	SECURE_MALLOC (write_m.payload, strlen(message_str), exit(EXIT_FAILURE));
	memcpy (write_m.payload, message_str, strlen(message_str));
	write_m.type = MSG_TYPE_DATA;
	write_m.user_type = 42;
	write_m.length = strlen(message_str);

	gettimeofday(&tval_before, NULL);
	for (size_t i = 0 ; i < nb_rounds ; i++) {
		ipc_write (&srv, &write_m);

		// reading
		TEST_IPC_Q(ipc_read (&srv, &read_m), EXIT_FAILURE);
		// printf ("received message (%d bytes): %*s\n", read_m.length, read_m.length, read_m.payload);
		// ipc_message_empty (&read_m);

	}
	gettimeofday(&tval_after, NULL);

	timersub(&tval_after, &tval_before, &tval_result);

	printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

	// disconnection
	TEST_IPC_Q (ipc_close (&srv), EXIT_FAILURE);

    return EXIT_SUCCESS;
}
