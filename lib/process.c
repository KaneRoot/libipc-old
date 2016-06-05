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
    return (p1->pid == p2->pid && p1->version == p2->version
            && p1->index == p2->index);
}

void srv_process_gen (struct process *p
        , pid_t pid, unsigned int index, unsigned int version)
{
    p->pid = pid;
    p->version = version;
    p->index = index;

    bzero (p->path_in, PATH_MAX);
    bzero (p->path_out, PATH_MAX);
    snprintf(p->path_in , PATH_MAX, "%s/%d-%d-in" , TMPDIR, pid, index);
    snprintf(p->path_out, PATH_MAX, "%s/%d-%d-out", TMPDIR, pid, index);
}

void srv_process_free (struct process * p)
{
    if (! p)
        return;
    free (p);
}

void process_print (struct process *p)
{
    printf ("process %d : index %d\n", p->pid, p->index);
}
