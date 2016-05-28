#include <stdlib.h>

#include <communication.h>

#include "list.h"

typedef struct {
	int test;
} Publisher;

typedef struct {
	int test;
} Subscriber;

const char* service_name = "pubsub";

void
ohshit(int rvalue, const char* str) {
	fprintf(stderr, "%s\n", str);

	exit(rvalue);
}

int
main(int argc, char* argv[])
{
	List* subscribers;
	List* publishers;
	int r;
	char s_path[PATH_MAX];
	int s_pipe;

	(void) argc;
	(void) argv;

	service_path(s_path, service_name);

	printf("Listening on %s.\n", s_path);

	if ((r = service_create(s_path)))
		ohshit(1, "service_create error");

	publishers = list_new(sizeof(Publisher));
	subscribers = list_new(sizeof(Subscriber));

	if (!publishers && !subscribers)
		ohshit(1, "out of memory, already...");

	/* ?!?!?!?!? */
	mkfifo(s_path, S_IRUSR);

	s_pipe = open(s_path, S_IRUSR);

	for (;;) {
		struct process* proc;
		int proc_count, i;

		service_get_new_processes(&proc, &proc_count, s_pipe);

		printf("> %i proc\n", proc_count);

		for (i = 0; i < proc_count; i++) {
			size_t message_size = BUFSIZ;
			char buffer[BUFSIZ];

			process_print(proc + i);

			if ((r = process_read(&proc[i], &buffer, &message_size))) {
				ohshit(1, "process_read error");
			}

			printf(": %s\n", buffer);

			
		}

		service_free_processes(&proc, proc_count);

		break;
	}

	close(s_pipe);

	list_free(publishers);
	list_free(subscribers);

	service_close(s_path);

	return 0;
}

