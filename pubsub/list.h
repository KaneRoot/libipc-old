
#ifndef LIST_H
#define LIST_H

struct link {
	struct link* next;
	char value[];
};

typedef struct {
	struct link* head;
	struct link* tail;
	size_t element_size;
	size_t length;
} List;

List* list_new(size_t);
void* list_append(List*);
void  list_remove(List*, size_t);
void  list_free(List*);

#endif

