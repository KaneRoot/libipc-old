#include "communication.h"
#include <stdio.h>
#include <time.h>

int file_open (FILE **f, const char *path, const char *mode)
{
    printf ("opening %s\n", path);
    if (*f != NULL) {
        printf ("f != NULL : %p\n", (void*) *f);
        if (file_close (*f)) {
            return ER_FILE_CLOSE;
        }
    }
    *f = fopen (path, mode);
    if (*f == NULL) {
        fprintf (stderr, "\033[31mnot opened %s\033[00m\n", path);
        return ER_FILE_OPEN;
    }
    printf ("opened : %ld\n", (long) *f);

    return 0;
}

int file_close (FILE *f)
{
    if (f != 0) {
        printf ("before fclosing\n");
        if (fclose (f)) {
            return ER_FILE_CLOSE;
        }
        printf ("after fclosing\n");
    }
    return 0;
}

int file_read (FILE *f, char **buf, size_t *msize) {
    if (*msize == 0) {
        *msize = BUFSIZ; // default value
    }

    if (*buf == NULL) {
        *buf = malloc (*msize);
        if (*buf == NULL) {
            fprintf (stderr, "err can't allocate enough memory (%ld)\n", *msize);
            int ret = file_close (f);
            if (ret != 0)
                return ret;
        }
    }

    *msize = fread (*buf, *msize, 1, f);
    if (*msize == 0) {
        fprintf (stderr, "err can't read a file\n");
        if (ER_FILE_CLOSE == file_close (f)) {
            fprintf (stderr, "err closing the file\n");
            return ER_FILE_CLOSE;
        }
        return ER_FILE_READ;
    }

    return 0;
}

int file_write (FILE *f, const char *buf, size_t msize)
{
    if (0 == fwrite (buf, msize, 1, f)) {
        fprintf (stderr, "err writing in the file\n");
        if (ER_FILE_CLOSE == file_close (f)) {
            fprintf (stderr, "err closing the file\n");
            return ER_FILE_CLOSE;
        }
        return ER_FILE_WRITE;
    }

    return 0;
}

int srv_init (int argc, char **argv, char **env, struct service *srv, const char *sname, int (*cb)(int argc, char **argv, char **env, struct service *srv, const char *sname))
{
    if (srv == NULL)
        return ER_PARAMS;

    // TODO
    //      use the argc, argv and env parameters
    //      it will be useful to change some parameters transparently
    //      ex: to get resources from other machines, choosing the
    //          remote with environment variables

    argc = argc;
    argv = argv;
    env = env;

    // gets the service path, such as /tmp/<service>
    memset (srv->spath, 0, PATH_MAX);
    strncat (srv->spath, TMPDIR, PATH_MAX -1);
    strncat (srv->spath, sname, PATH_MAX -1);

    srv->version = COMMUNICATION_VERSION;
    srv->index = 0; // TODO

    if (cb != NULL) {
        int ret = (*cb) (argc, argv, env, srv, sname);
        if (ret != 0)
            return ret;
    }

    return 0;
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
    memset (*buf, 0, BUFSIZ);

    FILE * f = NULL;
    if (file_open (&f, srv->spath, "rb")) {
        return ER_FILE_OPEN;
    }

    char *ret = NULL;
    ret = fgets (*buf, BUFSIZ, f);
    if (ret == NULL) {
        return ER_FILE_READ;
    }
    buf[0][BUFSIZ -1] = '\0';

    if (file_close (f)) {
        return ER_FILE_CLOSE;
    }

    *msize = strlen (*buf);

    return 0;
}

int srv_get_new_process (const struct service *srv, struct process *p)
{
    if (srv->spath == NULL) {
        return -1;
    }

    char buf[BUFSIZ];
    memset (buf, 0, BUFSIZ);

    // read the pipe, get a process to work on
    struct timespec ts = { 0 };
    struct timespec ts2 = { 0 };

    FILE * f = NULL;
    if (file_open (&f, srv->spath, "rb")) {
        return ER_FILE_OPEN;
    }

    clock_gettime(CLOCK_REALTIME, &ts);

    char *ret = NULL;
    ret = fgets (buf, BUFSIZ, f);
    if (ret == NULL) {
        if (file_close (f)) {
            return ER_FILE_CLOSE;
        }
        return ER_FILE_READ;
    }

    clock_gettime(CLOCK_REALTIME, &ts2);
    if (file_close (f)) {
        return ER_FILE_CLOSE;
    }

    printf("sec: %ld nsec: %ld\n", ts.tv_sec, ts.tv_nsec);
    printf("sec: %ld nsec: %ld\n", ts2.tv_sec, ts2.tv_nsec);

    printf("diff nsec: %ld\n", ts2.tv_nsec - ts.tv_nsec);

    char *token = NULL, *saveptr = NULL;
    char *str = NULL;
    int i = 0;

    pid_t pid = 0;
    int index = 0;
    int version = 0;

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
    if (file_open (&p->out, p->path_out, "rb")) {
        fprintf (stderr, "\033[31merr: srv_read_cb, file_open\033[00m\n");
        if (ER_FILE_CLOSE == file_close (p->out)) {
            fprintf (stderr, "err closing the file %s\n", p->path_out);
            p->out = NULL;
        }
        return ER_FILE_OPEN;
    }

    if (cb != NULL) {
        int ret = (*cb) (p->out, buf, msize);
        if (ret != 0)
            return ret;
    }
    else {
        int ret = file_read (p->out, buf, msize);
        if (ret != 0) {
            return ret;
        }
    }
    // printf ("DEBUG read, size %ld : %s\n", *msize, *buf);

    if (ER_FILE_CLOSE == file_close (p->out)) {
        fprintf (stderr, "err closing the file %s\n", p->path_out);
        p->out = NULL;
    }
    p->out = NULL;

    return 0;
}

int srv_read (struct process *p, char ** buf, size_t * msize)
{
    if (ER_FILE_OPEN == file_open (&p->out, p->path_out, "rb")) {
        fprintf (stderr, "err opening the file %s\n", p->path_out);
        return ER_FILE_OPEN;
    }

    int ret = file_read (p->out, buf, msize);
    if (ret != 0) {
        p->out = NULL;
        return ret;
    }

    // printf ("DEBUG read, size %ld : %s\n", *msize, buf);

    if (ER_FILE_CLOSE == file_close (p->out)) {
        fprintf (stderr, "err closing the file %s\n", p->path_out);
        p->out = NULL;
        return ER_FILE_CLOSE;
    }
    p->out = NULL;

    return 0;
}

int srv_write (struct process *p, char * buf, size_t msize)
{
    if (ER_FILE_OPEN == file_open (&p->in, p->path_in, "wb")) {
        fprintf (stderr, "err opening the file %s\n", p->path_in);
        return ER_FILE_OPEN;
    }

    int ret = file_write (p->in, buf, msize);
    if (ret != 0) {
        fprintf (stderr, "err writing in the file %s\n", p->path_in);
        p->in = NULL;
        return ret;
    }

    if (ER_FILE_CLOSE == file_close (p->in)) {
        fprintf (stderr, "err closing the file %s\n", p->path_in);
        p->in = NULL;
        return ER_FILE_CLOSE;
    }
    p->in = NULL;

    return 0;
}

// APPLICATION

// send the connection string to $TMP/<service>
int app_srv_connection (struct service *srv, const char *connectionstr, size_t msize)
{
    if (srv == NULL) {
        return 1;
    }

    FILE * f = NULL;
    if (ER_FILE_OPEN == file_open (&f, srv->spath, "wb")) {
        fprintf (stderr, "err opening the service file %s\n", srv->spath);
        return ER_FILE_OPEN;
    }

    int ret = file_write (f, connectionstr, msize);
    if (ret != 0) {
        fprintf (stderr, "err writing in the service file %s\n", srv->spath);
        return ret;
    }

    if (ER_FILE_CLOSE == file_close (f)) {
        fprintf (stderr, "err closing the file\n");
        return ER_FILE_CLOSE;
    }

    return 0;
}

int app_create (struct process *p, pid_t pid, int index, int version)
{
    if (version == 0) {
        version = COMMUNICATION_VERSION;
    }

    // then creates the structure
    srv_process_gen (p, pid, index, version);

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

int app_read_cb (struct process *p, char ** buf, size_t * msize
        , int (*cb)(FILE *f, char ** buf, size_t * msize))
{
    if (file_open (&p->in, p->path_in, "rb")) {
        fprintf (stderr, "\033[31merr: app_read_cb, file_open\033[00m\n");
        p->in = NULL;
        return 1;
    }

    if (cb != NULL) {
        int ret = (*cb) (p->in, buf, msize);
        if (ret != 0)
            return ret;
    }
    else {
        int ret = file_read (p->in, buf, msize);
        if (ret != 0) {
            p->in = NULL;
            return ret;
        }
    }


    if (ER_FILE_CLOSE == file_close (p->in)) {
        fprintf (stderr, "err closing the file %s\n", p->path_in);
    }
    p->in = NULL;

    return 0;
}

int app_read (struct process *p, char ** buf, size_t * msize)
{
    if (ER_FILE_OPEN == file_open (&p->in, p->path_in, "rb")) {
        fprintf (stderr, "err opening the file %s\n", p->path_in);
        return ER_FILE_OPEN;
    }

    int ret = file_read (p->in, buf, msize);
    if (ret != 0) {
        p->in = NULL;
        return ret;
    }

    if (ER_FILE_CLOSE == file_close (p->in)) {
        fprintf (stderr, "err closing the file %s\n", p->path_in);
        p->in = NULL;
        return ER_FILE_CLOSE;
    }
    p->in = NULL;

    return 0;
}

int app_write (struct process *p, char * buf, size_t msize)
{
    if (buf == NULL) {
        return ER_FILE_WRITE_PARAMS;
    }

    if (ER_FILE_OPEN == file_open (&p->out, p->path_out, "wb")) {
        fprintf (stderr, "err opening the file %s\n", p->path_out);
        return ER_FILE_OPEN;
    }

    int ret = file_write (p->out, buf, msize);
    if (ret != 0) {
        fprintf (stderr, "err writing in the file %s\n", p->path_out);
        p->out = NULL;
        return ret;
    }

    if (ER_FILE_CLOSE == file_close (p->out)) {
        fprintf (stderr, "err closing the file %s\n", p->path_out);
        p->out = NULL;
        return ER_FILE_CLOSE;
    }
    p->out = NULL;

    return 0;
}
