#include "process.h"

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
    snprintf(p->path_proc, PATH_MAX, "%s%d-%d", TMPDIR, index, version);
}

void srv_process_print (struct process *p)
{
    if (p != NULL)
        printf ("process %d : index %d, version %d\n", p->index, p->version);
}
