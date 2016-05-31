#include "communication.h"
#include <stdio.h>
#include <time.h>

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
        switch (errno) {
            case EACCES :
                printf ("file %s : EACCES\n", fifopath);
                return 1;
            case EEXIST :
                printf ("file %s : EEXIST\n", fifopath);
                break;
            case ENAMETOOLONG :
                printf ("file %s : ENAMETOOLONG\n", fifopath);
                return 2;
            case ENOENT :
                printf ("file %s : ENOENT\n", fifopath);
                return 3;
            case ENOSPC :
                printf ("file %s : ENOSPC\n", fifopath);
                return 4;
            case ENOTDIR :
                printf ("file %s : ENOTDIR\n", fifopath);
                return 5;
            case EROFS :
                printf ("file %s : EROFS\n", fifopath);
                return 6;
            default :
                printf ("err file %s unknown\n", fifopath);
                return 7;
        }
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

int service_get_new_process (struct process *proc, const char * spath)
{
    if (spath == NULL) {
        return -1;
    }

    char buf[BUFSIZ];
    bzero (buf, BUFSIZ);

    // read the pipe, get a process to work on
    struct timespec ts = { 0 };
    struct timespec ts2 = { 0 };

    FILE * f = fopen (spath, "r");
    clock_gettime(CLOCK_REALTIME, &ts);
    fgets (buf, BUFSIZ, f);
    clock_gettime(CLOCK_REALTIME, &ts2);
    fclose (f);

    printf("sec: %ld nsec: %ld\n", ts.tv_sec, ts.tv_nsec);
    printf("sec: %ld nsec: %ld\n", ts2.tv_sec, ts2.tv_nsec);

    printf("diff nsec: %ld\n", ts2.tv_nsec - ts.tv_nsec);

    char *token, *saveptr;
    char *str;
    int i;

    for (str = buf, i = 1; ; str = NULL, i++) {
        token = strtok_r(str, " ", &saveptr);
        if (token == NULL)
            break;

        if (i == 1) {
            proc->pid = strtoul(token, NULL, 10);
        }
        else if (i == 2) {
            proc->index = strtoul(token, NULL, 10);
        }
        else if (i == 3) {
            proc->version = strtoul(token, NULL, 10);
        }
    }

    return 1;
}

// FIXME only works for a single process
void service_get_new_processes (struct process ***proc, int *nproc, char * spath)
{
    if (proc == NULL || spath == NULL) {
        return;
    }

    char buf[BUFSIZ];
    bzero (buf, BUFSIZ);

    // read the pipe, get a process to work on
    FILE * f = fopen (spath, "rb");
    fgets (buf, BUFSIZ, f);
    fclose (f);

    char *token, *line, *saveptr, *saveptr2;
    char *str, *str2;
    int i, j;

    *nproc = 0;
    proc[0] = malloc(sizeof(struct process**));

    for (str2 = buf, j = 1; ; str2 = NULL, j++) {
        line = strtok_r(str2, "\n", &saveptr2);
        if (line == NULL)
            break;

        printf ("line : %s\n", line);

        *nproc = *nproc +1;

        proc[0] = realloc(proc[0], sizeof(struct process*) * (*nproc));
        proc[0][*nproc -1] = malloc(sizeof(struct process));

        for (str = line, i = 1; ; str = NULL, i++) {
            token = strtok_r(str, " ", &saveptr);
            if (token == NULL)
                break;

            if (i == 1) {
                proc[0][*nproc -1]->pid = strtoul(token, NULL, 10);
            }
            else if (i == 2) {
                proc[0][*nproc -1]->index = strtoul(token, NULL, 10);
            }
            else if (i == 3) {
                proc[0][*nproc -1]->version = strtoul(token, NULL, 10);
            }

        }
    }
}

void struct_process_free (struct process * p)
{
    free (p);
}

void service_free_processes (struct process **procs, int nproc)
{
    printf ("free processes\n");
    while (nproc--) {
        printf ("free process %d\n", nproc);

        struct_process_free (procs[nproc]);
    }
}

void gen_process_structure (struct process *p
        , pid_t pid, unsigned int index, unsigned int version)
{
    p->pid = pid;
    p->version = version;
    p->index = index;
}


void process_print (struct process *p)
{
    printf ("process %d : index %d\n", p->pid, p->index);
}

int process_create (struct process *p, int index)
{
    pid_t pid = getpid();
    char fifopathin[PATH_MAX];
    char fifopathout[PATH_MAX];
    process_paths (fifopathin, fifopathout, pid, index);

    // creates the pipes
    int ret;
    if ((ret = mkfifo (fifopathin, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)))
    {
        switch (errno) {
            case EACCES :
                printf ("file %s : EACCES\n", fifopathin);
                return 1;
            case EEXIST :
                printf ("file %s : EEXIST\n", fifopathin);
                break;
            case ENAMETOOLONG :
                printf ("file %s : ENAMETOOLONG\n", fifopathin);
                return 2;
            case ENOENT :
                printf ("file %s : ENOENT\n", fifopathin);
                return 3;
            case ENOSPC :
                printf ("file %s : ENOSPC\n", fifopathin);
                return 4;
            case ENOTDIR :
                printf ("file %s : ENOTDIR\n", fifopathin);
                return 5;
            case EROFS :
                printf ("file %s : EROFS\n", fifopathin);
                return 6;
            default :
                printf ("err file %s unknown\n", fifopathin);
                return 7;
        }
    }

    if ((ret = mkfifo (fifopathout, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
        switch (errno) {
            case EACCES :
                printf ("file %s : EACCES\n", fifopathout);
                return 1;
            case EEXIST :
                printf ("file %s : EEXIST\n", fifopathout);
                break;
            case ENAMETOOLONG :
                printf ("file %s : ENAMETOOLONG\n", fifopathout);
                return 2;
            case ENOENT :
                printf ("file %s : ENOENT\n", fifopathout);
                return 3;
            case ENOSPC :
                printf ("file %s : ENOSPC\n", fifopathout);
                return 4;
            case ENOTDIR :
                printf ("file %s : ENOTDIR\n", fifopathout);
                return 5;
            case EROFS :
                printf ("file %s : EROFS\n", fifopathout);
                return 6;
            default :
                printf ("err file %s unknown\n", fifopathout);
                return 7;
        }
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

int process_open_in (struct process *proc)
{
    char fifopathin[PATH_MAX];
    char fifopathout[PATH_MAX];
    process_paths (fifopathin, fifopathout, proc->pid, proc->index);

    printf ("opening in %s\n", fifopathin);
    proc->in = fopen (fifopathin, "rb");
    if (proc->in == NULL) {
        fprintf (stderr, "\033[31mnot opened\033[00m\n");
        return -1;
    }
    printf ("opened : %d\n", proc->in);

    return 0;
}

int service_proc_open_in (struct process *proc)
{
    char fifopathin[PATH_MAX];
    char fifopathout[PATH_MAX];
    process_paths (fifopathin, fifopathout, proc->pid, proc->index);

    printf ("opening in %s\n", fifopathin);
    proc->in = fopen (fifopathin, "wb");
    if (proc->in == NULL) {
        fprintf (stderr, "\033[31mnot opened\033[00m\n");
        return -1;
    }
    printf ("opened : %d\n", proc->in);

    return 0;
}

int service_proc_open_out (struct process *proc)
{
    char fifopathin[PATH_MAX];
    char fifopathout[PATH_MAX];
    process_paths (fifopathin, fifopathout, proc->pid, proc->index);

    printf ("opening out %s\n", fifopathout);
    proc->out = fopen (fifopathout, "rb");
    if (proc->out == NULL) {
        fprintf (stderr, "\033[31mnot opened\033[00m\n");
        return -1;
    }
    printf ("opened\n");

    return 0;
}

int process_open_out (struct process *proc)
{
    char fifopathin[PATH_MAX];
    char fifopathout[PATH_MAX];
    process_paths (fifopathin, fifopathout, proc->pid, proc->index);

    printf ("opening out %s\n", fifopathout);
    proc->out = fopen (fifopathout, "wb");
    if (proc->out == NULL) {
        fprintf (stderr, "\033[31mnot opened\033[00m\n");
        return -1;
    }
    printf ("opened\n");

    return 0;
}

int process_close_in (struct process *proc)
{
    printf ("closing in\n");
    if (proc->in != 0) {
        printf ("before fclose in\n");
        fclose (proc->in);
        printf ("after fclose in\n");
        proc->in = 0;
    }
    return 0;
}

int process_close_out (struct process *proc)
{
    printf ("closing out\n");
    if (proc->out != 0) {
        fclose (proc->out);
        proc->out = 0;
    }
    return 0;
}


int process_read (struct process *proc, void * buf, size_t * msize)
{
    int ret;
    if ((ret = process_open_in (proc))) {
        return 1;
    }

    *msize = fread (buf, 1, *msize, proc->in); // FIXME check errors
    // printf ("DEBUG read, size %ld : %s\n", *msize, buf);

    if ((ret = process_close_in (proc))) {
        return 1;
    }

    return 0;
}

int process_write (struct process *proc, void * buf, size_t msize)
{
    int ret;
    if ((ret = process_open_out (proc))) {
        return 1;
    }

    fwrite (buf, 1, msize, proc->out); // FIXME check errors

    if ((ret = process_close_out (proc))) {
        return 1;
    }
    return 0;
}

int service_read (struct process *proc, void * buf, size_t * msize)
{
    int ret;
    if ((ret = service_proc_open_out (proc))) {
        return 1;
    }

    *msize = fread (buf, 1, *msize, proc->out); // FIXME check errors
    // printf ("DEBUG read, size %ld : %s\n", *msize, buf);

    if ((ret = process_close_out (proc))) {
        return 1;
    }

    return 0;
}

int service_write (struct process *proc, void * buf, size_t msize)
{
    int ret;
    if ((ret = service_proc_open_in (proc))) {
        return 1;
    }

    fwrite (buf, 1, msize, proc->in); // FIXME check errors

    if ((ret = process_close_in (proc))) {
        return 1;
    }
    return 0;
}
