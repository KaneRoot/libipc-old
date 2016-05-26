#include "lib/communication.h"

/*
 * pipes creation and removal test program
 *
 * 1. to create the named pipe /tmp/<service>
 * 2. to create the named pipes in & out
 * 3. to remove the named pipes in & out
 * 4. to remove the named pipe /tmp/<service>
 */

int main(int argc, char * argv[])
{
    // service path (/tmp/<service>)
    char spath[PATH_MAX];
    service_path (spath, "windows");

    int ret;
    if ((ret = service_create (spath))) {
        fprintf(stdout, "error service_create %d\n", ret);
        exit (1);
    }

    struct process proc;
    if ((ret = process_create (&proc, 0))) {
        fprintf(stdout, "error process_create %d\n", ret);
        exit (1);
    }

    /* specific code, talks between applications */

    if ((ret = process_destroy (&proc))) {
        fprintf(stdout, "error process_destroy %d\n", ret);
        exit (1);
    }

    if ((ret = service_close (spath))) {
        fprintf(stdout, "error service_close %d\n", ret);
        exit (1);
    }

    return EXIT_SUCCESS;
}
