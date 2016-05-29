#include <stdlib.h>

#include "list.h"

List*
list_new(size_t element_size)
{
	List* l = malloc(sizeof(*l));

	l->element_size = element_size;
	l->head = NULL;
	l->tail = NULL;
	l->length = 0;

	return l;
}

void*
list_append(List* l)
{
	struct link* link = malloc(sizeof(*link) + l->element_size);

	link->next = l->tail;
	l->tail = link;

	if (!l->head)
		l->tail = link;

	l->length++;

	return (void*) link->value;
}

void
list_free(List* l)
{
	struct link* next;
	struct link* link;

	for (link = l->head; link; link = next) {
		next = link->next;

		free(link);
	}

	free(l);
}

