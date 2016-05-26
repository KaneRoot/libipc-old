#include "lib/communication.h"

/*
 * main loop
 *
 * opens the application pipes,
 * reads then writes the same message,
 * then closes the pipes
 */

void main_loop (const char *spath)
{
    int ret;
    int sfifo = open (spath, S_IRUSR); // opens the service named pipe

    struct process *proc;
    int nproc = 0;

    do {
        service_get_new_processes (&proc, &nproc, sfifo);

        // for each process : open, read, write, close
        for (int i = 0 ; i < nproc ; i++) {
            printf ("new connected process : %d, version %d, index %d\n"
                    , proc[i].pid, proc[i].version, proc[i].index);

            if ((ret = process_open (&proc[i]))) {
                fprintf(stdout, "error process_create %d\n", ret);
                exit (1);
            }

            // about the message
            size_t msize;
            char *buf;

            if ((ret = process_read (&proc[i], &buf, &msize))) {
                fprintf(stdout, "error process_read %d\n", ret);
                exit (1);
            }

            if ((ret = process_write (&proc[i], &buf, msize))) {
                fprintf(stdout, "error process_read %d\n", ret);
                exit (1);
            }

            if ((ret = process_close (&proc[i]))) {
                fprintf(stdout, "error process_destroy %d\n", ret);
                exit (1);
            }
        }
    } while (0); // it's a test, we only do it once

    close (sfifo); // closes the service named pipe
}

/*
 * service test
 *
 * 1. creates the named pipe /tmp/<service>, then listens
 * 2. opens the named pipes in & out
 * 3. talks with the (test) program
 * 4. closes the test program named pipes
 * 5. removes the named pipe /tmp/<service>
 */

int main(int argc, char * argv[])
{
    // gets the service path, such as /tmp/<service>
    char spath[PATH_MAX];
    service_path (spath, "windows");

    // creates the service named pipe, that listens to client applications
    int ret;
    if ((ret = service_create (spath))) {
        fprintf(stdout, "error service_create %d\n", ret);
        exit (1);
    }

    // the service will loop until the end of time, a specific message, a signal
    main_loop (spath);

    // the application will shut down, and remove the service named pipe
    if ((ret = service_close ("windows"))) {
        fprintf(stdout, "error service_close %d\n", ret);
        exit (1);
    }

    return EXIT_SUCCESS;
}
