#include "process.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

struct ipc_client * ipc_server_process_copy (const struct ipc_client *p)
{
    if (p == NULL)
        return NULL;

    struct ipc_client * copy = malloc (sizeof(struct ipc_client));
    memcpy (copy, p, sizeof (struct ipc_client));

    return copy;
}

int ipc_server_process_eq (const struct ipc_client *p1, const struct ipc_client *p2)
{
    return (p1->version == p2->version && p1->index == p2->index
            && p1->proc_fd == p2->proc_fd);
}

void ipc_server_process_gen (struct ipc_client *p
        , unsigned int index, unsigned int version)
{
    p->version = version;
    p->index = index;
}

int ipc_process_add (struct ipc_process_array *aproc, struct ipc_client *p)
{
    assert(aproc != NULL);
    assert(p != NULL);
    aproc->size++;
    aproc->tab_proc = realloc(aproc->tab_proc
            , sizeof(struct ipc_client) * aproc->size);

    if (aproc->tab_proc == NULL) {
        return -1;
    }

    aproc->tab_proc[aproc->size - 1] = p;
    return 0;
}

int ipc_process_del (struct ipc_process_array *aproc, struct ipc_client *p)
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
                ipc_process_array_free (aproc);
            }
            else {
                aproc->tab_proc = realloc(aproc->tab_proc
                        , sizeof(struct ipc_client) * aproc->size);

                if (aproc->tab_proc == NULL) {
                    return -2;
                }
            }

            return 0;
        }
    }

    return -3;
}

void process_print (struct ipc_client *p)
{
    if (p != NULL)
        printf ("process %d : index %d, version %d\n"
                , p->proc_fd, p->index, p->version);
}

void ipc_process_array_print (struct ipc_process_array *ap)
{
    int i;
    for (i = 0; i < ap->size; i++) {
        printf("%d : ", i);
        process_print(ap->tab_proc[i]);
    }
}

void ipc_process_array_free (struct ipc_process_array *ap)
{
    if (ap->tab_proc != NULL) {
        free (ap->tab_proc);
        ap->tab_proc = NULL;
    }
    ap->size = 0;
}
