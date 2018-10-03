#include "../../core/communication.h"
#include "../../core/error.h"
#include "../lib/remoted.h"
#include "../lib/remotec.h"
#include "../lib/msg.h"

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/**
 * remoted test application
 *
 * this application can:
 *   listen, given an URI (including a transport layer and eventually a port)
 *   connect to a remote service through a tunnel
 *      the remote service used for testing is pongd
 *
 *  TODO: this test application is a work in progress
 *  currently, this application will:
 *      connect itself to the remoted service
 *      hang up the connection with the remoted service
 */

void usage (char **argv) {
    printf ( "usage: %s uri service\n", argv[0]);
}

#if 0
void * listener (void *params)
{
    int s = 0;
    s = pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
    if (s != 0) {
        handle_err ("listener", "pthread_setcancelstate != 0");
    }

    struct ipc_service *srv = NULL;
    srv = (struct ipc_service *) params;
    if (srv == NULL) {
        handle_err ("listener", "no service passed");
        return NULL;
    }

    // main loop
    while (1) {
        struct remoted_msg m;
        memset (&m, 0, sizeof (struct remoted_msg));

        remote_message_recv (srv, &m);
        printf ("\r\033[31m>\033[00m %s\n", m.data);
        print_cmd ();

        remote_message_free (&m);
    }

    pthread_exit (NULL);
}
#endif

void main_loop (int argc, char **argv, char **env
        , int index, int version, char *uri, char *service)
{
    printf ("connection to remoted: index %d version %d uri %s service %s\n"
          , index, version, uri, service);

    (void) argc;
    (void) argv;
    (void) env;

    struct ipc_service srv;
    memset (&srv, 0, sizeof (struct ipc_service));

    remotec_connection (argc, argv, env, &srv);
    log_debug ("remotec connected, entering main loop");

    struct remoted_msg msg;
    memset (&msg, 0, sizeof (struct remoted_msg));

#if 0
    // msg loop
    for (;;) {
        char buf[BUFSIZ];
        memset (buf, 0, BUFSIZ);

        /* TODO  */
        msg.datalen = 5; // FIXME: take parameters into account
        msg.data = malloc (msg.datalen);
        memset (msg.data, 0, msg.datalen);
        strncpy ((char *) msg.data, "salut", msg.datalen);

        /* TODO */
        remotec_message_send (&srv, &msg);
        free (msg.data);
        msg.data = NULL;
        msg.datalen = 0;
    }

    // free everything
    remote_message_free (&msg);
#endif

    log_debug ("remotec disconnection...");
    // disconnect from the server
    remotec_disconnection (&srv);
}

int main(int argc, char **argv, char **env)
{
    if (argc != 3) {
        usage (argv);
        exit (0);
    }

    int index = 0;
    int version = 0;

    main_loop (argc, argv, env, index, version, argv[1], argv[2]);

    return EXIT_SUCCESS;
}
