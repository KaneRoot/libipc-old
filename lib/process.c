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

    memset (p->path_in, 0, PATH_MAX);
    memset (p->path_out, 0, PATH_MAX);

    snprintf(p->path_in , PATH_MAX, "%s%d-%d-%d-in" , TMPDIR, pid, index, version);
    snprintf(p->path_out, PATH_MAX, "%s%d-%d-%d-out", TMPDIR, pid, index, version);
    snprintf(p->path_proc, PATH_MAX, "%s%d-%d-%d", TMPDIR, pid, index, version);

}

void srv_process_print (struct process *p)
{
    if (p != NULL)
        printf ("process %d : index %d, version %d\n"
                , p->pid, p->index, p->version);
}
