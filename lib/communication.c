#include "communication.h"
#include <stdio.h>
#include <time.h>

int file_write (const char *path, const char *buf, size_t msize)
{
    int fd = open (path, O_WRONLY);
    if (fd <= 0) {
        return ER_FILE_OPEN;
    }
    
    int ret = 0;
    ret = write (fd, buf, msize);

    close (fd);

    return ret;
}

int file_read (const char *path, char **buf, size_t *msize)
{
    int fd = open (path, O_RDONLY);
    if (fd <= 0) {
        return ER_FILE_OPEN;
    }

    if (*buf == NULL)
        *buf = malloc (BUFSIZ);

    int ret = 0;
    int ret2 = 0;
    ret = read (fd, *buf, BUFSIZ);
    if (ret <= 0) {
        fprintf (stderr, "err: read %s\n", path);
    }
    else {
        *msize = ret;
    }

    ret2 = close (fd);
    if (ret2 < 0) {
        fprintf (stderr, "err: close [err: %d] %s\n", ret2, path);
        perror ("closing");
    }

    return ret;
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

// TODO remove, replace by file_read
int srv_get_listen_raw (const struct service *srv, char **buf, size_t *msize)
{
    return file_read (srv->spath, buf, msize);
}

int srv_get_new_process (const struct service *srv, struct process *p)
{
    if (srv->spath == NULL) {
        return -1;
    }

    char *buf = NULL;
    size_t msize = 0;
    int ret = file_read (srv->spath, &buf, &msize);
    if (ret <= 0) {
        fprintf (stderr, "err: listening on %s\n", srv->spath);
        exit (1);
    }

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

int srv_read (struct process *p, char ** buf, size_t * msize)
{
    return file_read (p->path_out, buf, msize);
}

int srv_write (struct process *p, char * buf, size_t msize)
{
    return file_write (p->path_in, buf, msize);
}

// APPLICATION

// send the connection string to $TMP/<service>
int app_srv_connection (struct service *srv, const char *connectionstr, size_t msize)
{
    if (srv == NULL) {
        return -1;
    }
 
    return file_write (srv->spath, connectionstr, msize);
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

int app_read (struct process *p, char ** buf, size_t * msize)
{
    return file_read (p->path_in, buf, msize);
}

int app_write (struct process *p, char * buf, size_t msize)
{
    return file_write (p->path_out, buf, msize);
}
