#include "process.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

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
    return (p1->version == p2->version && p1->index == p2->index
            && p1->proc_fd == p2->proc_fd);
}

void srv_process_gen (struct process *p
        , unsigned int index, unsigned int version)
{
    p->version = version;
    p->index = index;
}

int add_proc (struct array_proc *aproc, struct process *p)
{
    assert(aproc != NULL);
    assert(p != NULL);
    aproc->size++;
    aproc->tab_proc = realloc(aproc->tab_proc
            , sizeof(struct process) * aproc->size);

    if (aproc->tab_proc == NULL) {
        return -1;
    }

    aproc->tab_proc[aproc->size - 1] = p;
    return 0;
}

int del_proc (struct array_proc *aproc, struct process *p)
{
    assert(aproc != NULL);
    assert(p != NULL);

    if (aproc->tab_proc == NULL) {
        return -1;
    }

    int i;
    for (i = 0; i < aproc->size; i++) {
        if (aproc->tab_proc[i] == p) {

            aproc->tab_proc[i] = aproc->tab_proc[aproc->size-1];
            aproc->size--;
            if (aproc->size == 0) {
                array_proc_free (aproc);
            }
            else {
                aproc->tab_proc = realloc(aproc->tab_proc
                        , sizeof(struct process) * aproc->size);

                if (aproc->tab_proc == NULL) {
                    return -2;
                }
            }

            return 0;
        }
    }

    return -3;
}

void process_print (struct process *p)
{
    if (p != NULL)
        printf ("process %d : index %d, version %d\n"
                , p->proc_fd, p->index, p->version);
}

void array_proc_print (struct array_proc *ap)
{
    int i;
    for (i = 0; i < ap->size; i++) {
        printf("%d : ", i);
        process_print(ap->tab_proc[i]);
    }
}

void array_proc_free (struct array_proc *ap)
{
    if (ap->tab_proc != NULL) {
        free (ap->tab_proc);
        ap->tab_proc = NULL;
    }
    ap->size = 0;
}
