#include "../../core/communication.h"
#include "../../core/process.h"
#include "../../core/error.h"
#include "../lib/remoted.h"
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>

// to quit them properly if a signal occurs
struct service srv;

void handle_signal (int signalnumber)
{
    // the application will shut down, and remove the service unix socket
    if (srv_close (&srv) < 0) {
        handle_error("srv_close < 0");
    }

    fprintf (stderr, "received a signal %d\n", signalnumber);
    exit (EXIT_SUCCESS);
}

void remoted_init ()
{
    /* TODO
     * handle authorizations
    */
}

int
main(int argc, char **argv, char **env)
{
    memset (&srv, 0, sizeof (struct service));
    srv.index = 0;
    srv.version = 0;

    signal(SIGHUP, handle_signal);
    signal(SIGINT, handle_signal);
    signal(SIGQUIT, handle_signal);

    remoted_init ();

    if (srv_init (argc, argv, env, &srv, REMOTED_SERVICE_NAME) < 0) {
        handle_error("srv_init < 0");
        return EXIT_FAILURE;
    }
    printf ("Listening on %s.\n", srv.spath);

    printf("MAIN: server created\n" );

    // the service will loop until the end of time, a specific message, a signal
    remoted_main_loop (&srv);

    // the application will shut down, and remove the service unix socket
    if (srv_close (&srv) < 0) {
        handle_error("srv_close < 0");
    }

    return EXIT_SUCCESS;
}
