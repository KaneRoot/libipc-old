#include "../lib/pubsubd.h"
#include <stdlib.h>

void
ohshit(int rvalue, const char* str) {
    fprintf(stderr, "%s\n", str);
    exit(rvalue);
}

    int
main(int argc, char* argv[])
{
    struct service srv;
    srv_init (&srv, PUBSUB_SERVICE_NAME);
    printf ("Listening on %s.\n", srv.spath);

    // creates the service named pipe, that listens to client applications
    if (srv_create (&srv))
        ohshit(1, "service_create error");

    struct channels chans;
    pubsubd_channels_init (&chans);

    for (;;) {
        struct process proc;
        srv_get_new_process (&proc, &srv);
        process_print (&proc);
    }

    // the application will shut down, and remove the service named pipe
    if (srv_close (&srv))
        ohshit(1, "service_close error");

    return EXIT_SUCCESS;
}

#if 0
void main_loop (const struct service *srv)
{
    int ret;
    struct process proc;

    int cnt = 10;

    while (cnt--) {
        // -1 : error, 0 = no new process, 1 = new process
        ret = srv_get_new_process (&proc, srv);
        if (ret == -1) {
            fprintf (stderr, "error service_get_new_process\n");
            continue;
        } else if (ret == 0) { // that should not happen
            continue;
        }

        // printf ("before print\n");
        process_print (&proc);
        // printf ("after print\n");

        // about the message
        size_t msize = BUFSIZ;
        char buf[BUFSIZ];
        bzero(buf, BUFSIZ);

        // printf ("before read\n");
        if ((ret = srv_read (&proc, &buf, &msize))) {
            fprintf(stdout, "error service_read %d\n", ret);
            continue;
        }
        // printf ("after read\n");
        printf ("read, size %ld : %s\n", msize, buf);

        // printf ("before proc write\n");
        if ((ret = srv_write (&proc, &buf, msize))) {
            fprintf(stdout, "error service_write %d\n", ret);
            continue;
        }

        // printf ("after proc write\n");
        printf ("\033[32mStill \033[31m%d\033[32m applications to serve\n",cnt);
    }
}
#endif
