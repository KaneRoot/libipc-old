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

// FIXME only works for a single process
void service_get_new_processes (struct process **proc, int *nproc, int sfifo)
{
    char buf[BUFSIZ];
    bzero (buf, BUFSIZ);
    read (sfifo, buf, BUFSIZ);

    //proc[0] = malloc(sizeof(struct process*) * 1); // FIXME
    proc[0] = malloc(sizeof(struct process) * 1); // FIXME

    // unsigned long long int val = strtoul(s, NULL, 10);

    char *token, *saveptr;
    char *str;
    int i;

    for (str = buf, i = 1; ; str = NULL, i++) {
        token = strtok_r(str, " ", &saveptr);
        if (token == NULL)
            break;

        printf ("token : %s\n", token);

        // do something
        if (i == 1) {
            int index = strchr (token, '-') - token;
            char * buf_pid = strndup (token, index);
            proc[0]->pid = strtoul(buf_pid, NULL, 10);
            proc[0]->index = strtoul(token + index +1, NULL, 10);

            printf ("buf_pid : %s\n", buf_pid);
            printf ("pid : %d\n", proc[0]->pid);
            printf ("index : %d\n", proc[0]->index);
        }
        else if (i == 2) {
            // FIXME
            proc[0]->version = strtoul(token, NULL, 10);
        }

    }

    *nproc = 1;
}

void struct_process_free (struct process * p)
{
    free (p);
}

void service_free_processes (struct process **procs, int nproc)
{
    while (--nproc != -1) {
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


void process_print (struct process *p) {
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
    printf ("opened\n");

    return 0;
}


int process_open (struct process *proc)
{
    char fifopathin[PATH_MAX];
    char fifopathout[PATH_MAX];
    process_paths (fifopathin, fifopathout, proc->pid, proc->index);

    printf ("opening %s\n", fifopathin);
    proc->in = fopen (fifopathin, "rb");
    printf ("opening %s\n", fifopathout);
    proc->out = fopen (fifopathout, "wb");
    printf ("opened\n");

    return 0;
}

int process_close_in (struct process *proc)
{
    printf ("closing in\n");
    if (proc->in != 0) {
        fclose (proc->in);
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
        fprintf(stdout, "error process_create %d\n", ret);
        exit (1);
    }

    *msize = fread (buf, 1, *msize, proc->in); // FIXME check errors

    if ((ret = process_close_in (proc))) {
        fprintf(stdout, "error process_close_in %d\n", ret);
        exit (1);
    }

    return 0;
}

int process_write (struct process *proc, void * buf, size_t msize)
{
    int ret;
    if ((ret = process_open_out (proc))) {
        fprintf(stdout, "error process_create %d\n", ret);
        exit (1);
    }

    fwrite (buf, 1, msize, proc->out); // FIXME check errors

    if ((ret = process_close_out (proc))) {
        fprintf(stdout, "error process_close_out %d\n", ret);
        exit (1);
    }
    return 0;
}

int service_read (struct process *proc, void * buf, size_t * msize)
{
    if ((*msize = fread (buf, 1, *msize, proc->out)))
        return *msize;

    return 0;
}

int service_write (struct process *proc, void * buf, size_t msize)
{
    if ((msize = fwrite (buf, 1, msize, proc->in)))
        return msize;

    return 0;
}
