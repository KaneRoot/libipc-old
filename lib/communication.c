#include "communication.h"
#include <stdio.h>
#include <time.h>

int file_open (FILE **f, const char *path, const char *mode)
{
    printf ("opening %s\n", path);
    *f = fopen (path, mode);
    if (*f == NULL) {
        fprintf (stderr, "\033[31mnot opened\033[00m\n");
        return -1;
    }
    printf ("opened : %ld\n", (long) *f);

    return 0;
}

int file_close (FILE *f)
{
    if (f != 0) {
        printf ("before fclosing\n");
        fclose (f);
        printf ("after fclosing\n");
    }
    return 0;
}

void srv_init (struct service *srv, const char *sname)
{
    if (srv == NULL)
        return;

    // gets the service path, such as /tmp/<service>
    bzero (srv->spath, PATH_MAX);
    strncat (srv->spath, TMPDIR, PATH_MAX);
    strncat (srv->spath, sname, PATH_MAX);

    srv->version = COMMUNICATION_VERSION;
    srv->index = 0; // TODO
}

// SERVICE

int srv_create (struct service *srv)
{
    int ret;
    if ((ret = mkfifo (srv->spath, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
        switch (errno) {
            case EACCES :
                printf ("file %s : EACCES\n", srv->spath);
                return 1;
            case EEXIST :
                printf ("file %s : EEXIST\n", srv->spath);
                break;
            case ENAMETOOLONG :
                printf ("file %s : ENAMETOOLONG\n", srv->spath);
                return 2;
            case ENOENT :
                printf ("file %s : ENOENT\n", srv->spath);
                return 3;
            case ENOSPC :
                printf ("file %s : ENOSPC\n", srv->spath);
                return 4;
            case ENOTDIR :
                printf ("file %s : ENOTDIR\n", srv->spath);
                return 5;
            case EROFS :
                printf ("file %s : EROFS\n", srv->spath);
                return 6;
            default :
                printf ("err file %s unknown\n", srv->spath);
                return 7;
        }
    }

    return 0;
}

int srv_close (struct service *srv)
{
    if (unlink (srv->spath)) {
        return 1;
    }

    return 0;
}

// only get a raw line from TMPDIR/<service>
int srv_get_listen_raw (const struct service *srv, char **buf, size_t *msize)
{
    *buf = malloc(BUFSIZ);
    bzero (*buf, BUFSIZ);

    FILE * f = fopen (srv->spath, "r");
    fgets (*buf, BUFSIZ, f);
    fclose (f);

    *msize = strlen (*buf);

    return 0;
}

int srv_get_new_process (const struct service *srv, struct process *p)
{
    if (srv->spath == NULL) {
        return -1;
    }

    char buf[BUFSIZ];
    bzero (buf, BUFSIZ);

    // read the pipe, get a process to work on
    struct timespec ts = { 0 };
    struct timespec ts2 = { 0 };

    FILE * f = fopen (srv->spath, "r");
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

    pid_t pid;
    int index;
    int version;

    for (str = buf, i = 1; ; str = NULL, i++) {
        token = strtok_r(str, " ", &saveptr);
        if (token == NULL)
            break;

        if (i == 1) {
            pid = strtoul(token, NULL, 10);
        }
        else if (i == 2) {
            index = strtoul(token, NULL, 10);
        }
        else if (i == 3) {
            version = strtoul(token, NULL, 10);
        }
    }

    srv_process_gen (p, pid, index, version);

    return 0;
}

int srv_read_cb (struct process *p, char ** buf, size_t * msize
        , int (*cb)(FILE *f, char ** buf, size_t * msize))
{
    if (file_open (&p->out, p->path_out, "rb"))
        return 1;

    if (cb != NULL)
        (*cb) (p->out, buf, msize);
    else
        *msize = fread (*buf, 1, *msize, p->out); // FIXME check errors
    // printf ("DEBUG read, size %ld : %s\n", *msize, *buf);

    if (file_close (p->out))
        return 1;

    return 0;
}

int srv_read (struct process *p, char ** buf, size_t * msize)
{
    if (file_open (&p->out, p->path_out, "rb"))
        return 1;

    *msize = fread (*buf, 1, *msize, p->out); // FIXME check errors
    // printf ("DEBUG read, size %ld : %s\n", *msize, buf);

    if (file_close (p->out))
        return 1;

    return 0;
}

int srv_write (struct process *p, void * buf, size_t msize)
{
    if (file_open (&p->in, p->path_in, "wb"))
        return 1;

    fwrite (buf, 1, msize, p->in); // FIXME check errors

    if (file_close (p->in))
        return 1;

    return 0;
}

// APPLICATION

int app_create (struct process *p, int index)
{
    pid_t pid = getpid();

    // then creates the structure
    srv_process_gen (p, pid, index, COMMUNICATION_VERSION);

    // creates the pipes
    int ret;
    if ((ret = mkfifo (p->path_in, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)))
    {
        switch (errno) {
            case EACCES :
                printf ("file %s : EACCES\n", p->path_in);
                return 1;
            case EEXIST :
                printf ("file %s : EEXIST\n", p->path_in);
                break;
            case ENAMETOOLONG :
                printf ("file %s : ENAMETOOLONG\n", p->path_in);
                return 2;
            case ENOENT :
                printf ("file %s : ENOENT\n", p->path_in);
                return 3;
            case ENOSPC :
                printf ("file %s : ENOSPC\n", p->path_in);
                return 4;
            case ENOTDIR :
                printf ("file %s : ENOTDIR\n", p->path_in);
                return 5;
            case EROFS :
                printf ("file %s : EROFS\n", p->path_in);
                return 6;
            default :
                printf ("err file %s unknown\n", p->path_in);
                return 7;
        }
    }

    if ((ret = mkfifo (p->path_out, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
        switch (errno) {
            case EACCES :
                printf ("file %s : EACCES\n", p->path_out);
                return 1;
            case EEXIST :
                printf ("file %s : EEXIST\n", p->path_out);
                break;
            case ENAMETOOLONG :
                printf ("file %s : ENAMETOOLONG\n", p->path_out);
                return 2;
            case ENOENT :
                printf ("file %s : ENOENT\n", p->path_out);
                return 3;
            case ENOSPC :
                printf ("file %s : ENOSPC\n", p->path_out);
                return 4;
            case ENOTDIR :
                printf ("file %s : ENOTDIR\n", p->path_out);
                return 5;
            case EROFS :
                printf ("file %s : EROFS\n", p->path_out);
                return 6;
            default :
                printf ("err file %s unknown\n", p->path_out);
                return 7;
        }
    }

    return 0;
}

int app_destroy (struct process *p)
{
    if (unlink (p->path_in)) {
        return 1;
    }

    if (unlink (p->path_out)) {
        return 1;
    }

    return 0;
}

int app_read (struct process *p, void * buf, size_t * msize)
{
    if (file_open (&p->in, p->path_in, "rb"))
        return 1;

    *msize = fread (buf, 1, *msize, p->in); // FIXME check errors
    // printf ("DEBUG read, size %ld : %s\n", *msize, buf);

    if (file_close (p->in))
        return 1;

    return 0;
}

int app_write (struct process *p, void * buf, size_t msize)
{
    if (file_open (&p->out, p->path_out, "wb"))
        return 1;

    fwrite (buf, 1, msize, p->out); // FIXME check errors

    if (file_close (p->out))
        return 1;

    return 0;
}
