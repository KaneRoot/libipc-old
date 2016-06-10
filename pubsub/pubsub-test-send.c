#include "../lib/pubsubd.h"
#include <stdlib.h>
#include <pthread.h>

#define MYMESSAGE "coucou"
#define MYCHAN    "chan1"

void
ohshit(int rvalue, const char* str) {
    fprintf (stderr, "\033[31merr: %s\033[00m\n", str);
    exit (rvalue);
}

    int
main(int argc, char* argv[])
{
    struct service srv;
    memset (&srv, 0, sizeof (struct service));
    srv_init (&srv, PUBSUB_SERVICE_NAME);
    printf ("Writing on %s.\n", srv.spath);

    struct process p;
    memset (&p, 0, sizeof (struct process));
    int index = 1;

    if (app_create (&p, index)) // called by the application
        ohshit (1, "app_create");

    // send a message to warn the service we want to do something
    // line : pid index version action chan
    pubsub_connection (&srv, &p, PUBSUB_PUB, MYCHAN);

    struct pubsub_msg m;
    memset (&m, 0, sizeof (struct pubsub_msg));

    // first message, "coucou"
    m.type = PUBSUB_TYPE_INFO;
    m.chan = malloc (strlen (MYCHAN));
    m.chanlen = strlen (MYCHAN);
    m.data = malloc (strlen (MYMESSAGE));
    m.datalen = strlen (MYMESSAGE);
    pubsub_msg_send (&srv, &p, &m);

    // second message, to disconnect from the server
    m.type = PUBSUB_TYPE_DISCONNECT;
    pubsub_msg_send (&srv, &p, &m);

    // free everything

    pubsubd_msg_free (&m);

    // the application will shut down, and remove the application named pipes
    if (app_destroy (&p))
        ohshit (1, "app_destroy");

    srv_process_free (&p);

    return EXIT_SUCCESS;
}
