#include "communication.h"

int service_path (char *buf, const char *sname)
{
    if (buf == NULL) {
        return 1;
    }

    // already done in mkfifo
    if (strlen(TMPDIR) + strlen(sname) > PATH_MAX) {
        return 2;
    }

    bzero (buf, PATH_MAX);
    strncat (buf, TMPDIR, PATH_MAX);
    strncat (buf, sname, PATH_MAX);

    return 0;
}

void process_paths (char *in, char *out, pid_t pid, int index)
{
    bzero (in, PATH_MAX);
    bzero (out, PATH_MAX);
    snprintf(in , PATH_MAX, "%s/%d-%d-in" , TMPDIR, pid, index);
    snprintf(out, PATH_MAX, "%s/%d-%d-out", TMPDIR, pid, index);
}

int service_create (const char *fifopath)
{
    int ret;
    if ((ret = mkfifo (fifopath, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
        return 1;
    }

    return 0;
}

int service_close (const char *fifopath)
{
    if (unlink (fifopath)) {
        return 1;
    }

    return 0;
}

void service_get_new_processes (struct process **proc, int *nproc, int sfifo)
{
    char buf[BUFSIZ];
    bzero (buf, BUFSIZ);
    read (sfifo, buf, BUFSIZ);

    // TODO
#if 0
    unsigned long long int val = strtoull(s, NULL, 10);

    char *token, *saveptr;
    char *str;
    int i, j;

    // printf("%d fields :: ", f->nbfields);
    // for( i = 0 ; i < f->nbfields ; i++) {
    //     printf("%u, ", f->fields[i]);
    // }
    // printf("\n");

    for (str = s, i = 1; ; str = NULL, i++) {
        token = strtok_r(str, delim, &saveptr);
        if (token == NULL)
            break;

        // check if we need to print the current token humanized
        int found = 0;

        for (j = 0 ; j < f->nbfields && found == 0; j++ ) {
            if( i == f->fields[j] ) {
                print_token_to_human(token);
                found = 1;
            }
        }

        // print the current token not humanized
        if (found == 0) {
            printf("%s ", token);
        }
    }
#endif
}

void gen_process_structure (struct process *p
        , pid_t pid, unsigned int index, unsigned int version)
{
    p->pid = pid;
    p->version = version;
    p->index = index;
}


int process_create (struct process *p, int index)
{
    pid_t pid = getpid();
    char fifopathin[PATH_MAX];
    char fifopathout[PATH_MAX];
    process_paths (fifopathin, fifopathout, pid, index);

    // creates the pipes
    int ret;
    if ((ret = mkfifo (fifopathin, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
        return 1;
    }

    if ((ret = mkfifo (fifopathout, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
        return 1;
    }

    // then creates the structure
    gen_process_structure (p, pid, index, COMMUNICATION_VERSION);

    return 0;
}

int process_destroy (struct process *p)
{
    char fifopathin[PATH_MAX];
    char fifopathout[PATH_MAX];
    process_paths (fifopathin, fifopathout, p->pid, p->index);

    if (unlink (fifopathin)) {
        return 1;
    }

    if (unlink (fifopathout)) {
        return 1;
    }

    return 0;
}


int process_open (struct process *proc)
{
    char fifopathin[PATH_MAX];
    char fifopathout[PATH_MAX];
    process_paths (fifopathin, fifopathout, proc->pid, proc->index);

    proc->in = open (fifopathin, S_IRUSR);
    proc->out = open (fifopathout, S_IRUSR);

    return 0;
}

int process_close (struct process *proc)
{
    if (proc->in != 0) {
        close (proc->in);
        proc->in = 0;
    }

    if (proc->out != 0) {
        close (proc->out);
        proc->out = 0;
    }

    return 0;
}


int process_read (struct process *proc, void * buf, size_t * msize)
{
    return 0;
}

int process_write (struct process *proc, void * buf, size_t msize)
{
    return 0;
}
