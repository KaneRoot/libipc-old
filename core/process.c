#include "process.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// TODO
// tout revoir ici

#if 0
struct process * srv_process_copy (const struct process *p)
{
    if (p == NULL)
        return NULL;

    struct process * copy = malloc (sizeof(struct process));
    memcpy (copy, p, sizeof (struct process));

    return copy;
}

int srv_process_eq (const struct process *p1, const struct process *p2)
{
    return (p1->version == p2->version && p1->index == p2->index);
}

void srv_process_gen (struct process *p
        , unsigned int index, unsigned int version)
{
    p->version = version;
    p->index = index;
}


#endif

int add_proc(struct array_proc *aproc, struct process *p) {
    assert(aproc != NULL);
    assert(p != NULL);
    aproc->size++;
    aproc->tab_proc = realloc(aproc->tab_proc, sizeof(struct process) * aproc->size);
    if (aproc->tab_proc == NULL) {
        return -1;
    }

    aproc->tab_proc[aproc->size - 1] = p;
    return 0;
}

int del_proc(struct array_proc *aproc, struct process *p) {
    assert(aproc != NULL);
    assert(p != NULL);

    int i;
    for (i = 0; i < aproc->size; i++) {
        if (aproc->tab_proc[i] == p) {
            aproc->tab_proc[i] = aproc->tab_proc[aproc->size-1];
            aproc->size--;
            aproc->tab_proc = realloc(aproc->tab_proc, sizeof(struct process) * aproc->size);
            if (aproc->tab_proc == NULL) {
                return -1;
            }
            return 0;
        }
    }

    return -2;
}

void process_print (struct process *p)
{
    if (p != NULL)
        printf ("process %d : index %d, version %d\n", p->proc_fd, p->index, p->version);
}

void array_proc_print( struct array_proc *ap) {
    int i;
    for (i = 0; i < ap->size; i++) {
        printf("%d : ", i);
        process_print(ap->tab_proc[i]);
    }
}


