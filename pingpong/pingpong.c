#include "../lib/communication.h"

#define PONGD_SERVICE_NAME "pongd"

/*
 * main loop
 *
 * opens the application pipes,
 * reads then writes the same message,
 * then closes the pipes
 */

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

/*
 * service ping-pong
 *
 * 1. creates the named pipe /tmp/<service>, then listens
 * 2. opens the named pipes in & out
 * 3. talks with the (test) program
 * 4. closes the test program named pipes
 * 5. removes the named pipe /tmp/<service>
 */

int main(int argc, char * argv[])
{
    struct service srv;
    srv_init (&srv, PONGD_SERVICE_NAME);
    printf ("Listening on %s.\n", srv.spath);

    // creates the service named pipe, that listens to client applications
    int ret;
    if ((ret = srv_create (&srv))) {
        fprintf(stdout, "error service_create %d\n", ret);
        exit (1);
    }

    // the service will loop until the end of time, a specific message, a signal
    main_loop (&srv);

    // the application will shut down, and remove the service named pipe
    if ((ret = srv_close (&srv))) {
        fprintf(stdout, "error service_close %d\n", ret);
        exit (1);
    }

    return EXIT_SUCCESS;
}