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

void usage (char **argv)
{
    printf ( "usage : %s pid index (pub|sub|both|quit) [chan]\n", argv[0]);
}

void sim_connection (int argc, char **argv, char **env, pid_t pid, int index, int version, char *cmd, char *chan)
{

    printf ("Simulate connnection : pid %d index %d version %d "
            "cmd %s chan %s\n"
          , pid, index, version, cmd, chan );

    struct service srv;
    bzero (&srv, sizeof (struct service));
    srv_init (argc, argv, env, &srv, PUBSUB_SERVICE_NAME);
    printf ("Writing on %s.\n", srv.spath);

    struct process p;
    bzero (&p, sizeof (struct process));

    if (app_create (&p, index)) // called by the application
        ohshit (1, "app_create");

    // send a message to warn the service we want to do something
    // line : pid index version action chan
    pubsub_connection (&srv, &p, PUBSUB_PUB, MYCHAN);

    struct pubsub_msg m;
    bzero (&m, sizeof (struct pubsub_msg));

    // first message, "coucou"
    m.type = PUBSUB_TYPE_INFO;
    m.chan = malloc (strlen (MYCHAN));
    m.chanlen = strlen (MYCHAN);
    m.data = malloc (strlen (MYMESSAGE));
    m.datalen = strlen (MYMESSAGE);
    pubsub_msg_send (&p, &m);

    // second message, to disconnect from the server
    m.type = PUBSUB_TYPE_DISCONNECT;
    pubsub_msg_send (&p, &m);

    // free everything

    pubsubd_msg_free (&m);

    // the application will shut down, and remove the application named pipes
    if (app_destroy (&p))
        ohshit (1, "app_destroy");

    srv_process_free (&p);
}

void sim_disconnection (int argc, char **argv, char **env, pid_t pid, int index, int version)
{
    struct service srv;
    bzero (&srv, sizeof (struct service));
    srv_init (&srv, PUBSUB_SERVICE_NAME);
    printf ("Disconnecting from %s.\n", srv.spath);

    struct process p;
    bzero (&p, sizeof (struct process));

    // create the fake process
    srv_process_gen (&p, pid, index, version);

    // send a message to disconnect
    // line : pid index version action chan
    pubsub_disconnect (&srv, &p, PUBSUB_PUB, MYCHAN);

    srv_process_free (&p);
}

    int
main(int argc, char **argv, char **env)
{

    if (argc < 3) {
        usage (argv);
        exit (1);
    }

    pid_t pid = 0;
    pid = atol(argv[1]);

    int index = 0;
    index = atoi (argv[2]);

    // don't care about the version
    int version = COMMUNICATION_VERSION;

    char * cmd = NULL;
    cmd = argv[3];

    if (strcmp(cmd, "quit") != 0) {
        char *chan = NULL;
        chan = argv[4];
        sim_connection (argc, argv, env, pid, index, version, cmd, chan);
    }
    else {
        sim_disconnection (argc, argv, env, pid, index, version);
    }

    return EXIT_SUCCESS;
}
