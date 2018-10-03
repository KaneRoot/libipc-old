int main() { return 0; }

#if 0
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
main(int argc, char **argv, char **env)
{
    struct service srv;
    memset (&srv, 0, sizeof (struct service));
    ipc_server_init (argc, argv, env, &srv, PUBSUB_SERVICE_NAME, NULL);
    printf ("Writing on %s.\n", srv.spath);

    struct process p;
    memset (&p, 0, sizeof (struct process));
    int index = 1;

    pid_t pid = getpid();

    if (application_create (&p, pid, index, COMMUNICATION_VERSION))
        ohshit (1, "application_create");

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
    pubsub_message_send (&p, &m);

    // second message, to disconnect from the server
    m.type = PUBSUB_TYPE_DISCONNECT;
    pubsub_message_send (&p, &m);

    // free everything

    pubsubd_message_free (&m);

    // the application will shut down, and remove the application named pipes
    if (application_destroy (&p))
        ohshit (1, "application_destroy");

    return EXIT_SUCCESS;
}
#endif
