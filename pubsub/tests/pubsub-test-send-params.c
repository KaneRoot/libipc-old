int main() { return 0; }

#if 0
#include "../lib/pubsubd.h"
#include <stdlib.h>
#include <pthread.h>

#define MYMESSAGE "coucou"

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
    printf ("Simulate connection : pid %d index %d version %d "
            "cmd %s chan %s\n"
          , pid, index, version, cmd, chan );

    struct ipc_service srv;
    memset (&srv, 0, sizeof (struct ipc_service));
    ipc_server_init (argc, argv, env, &srv, PUBSUB_SERVICE_NAME, NULL);
    printf ("Writing on %s.\n", srv.spath);

    struct ipc_client p;
    memset (&p, 0, sizeof (struct ipc_client));

    printf ("app creation\n");
    if (application_create (&p, pid, index, version)) // called by the application
        ohshit (1, "application_create");

    printf ("connection\n");
    // send a message to warn the service we want to do something
    // line : pid index version action chan
    pubsub_connection (&srv, &p, PUBSUB_PUB, chan);

    struct pubsub_msg m;
    memset (&m, 0, sizeof (struct pubsub_msg));

    if (strcmp (cmd, "pub") == 0) {
        // first message, "coucou"
        m.type = PUBSUB_TYPE_MESSAGE;
        m.chan = malloc (strlen (chan) + 1);
        memset (m.chan, 0, strlen (chan) + 1);
        m.chan[strlen (chan)] = '\0';
        m.chanlen = strlen (chan);

        m.data = malloc (strlen (MYMESSAGE) + 1);
        memset (m.data, 0, strlen (MYMESSAGE) + 1);
        strncpy ((char *) m.data, MYMESSAGE, strlen (MYMESSAGE) + 1);
        m.datalen = strlen (MYMESSAGE);

        printf ("send message\n");
        pubsub_message_send (&p, &m);
    }
    else {
        pubsub_message_recv (&p, &m);
        pubsubd_message_print (&m);
    }

    // free everything
    pubsubd_message_free (&m);

    printf ("disconnection\n");
    // disconnect from the server
    pubsub_disconnect (&p);

    printf ("destroying app\n");
    // the application will shut down, and remove the application named pipes
    if (application_destroy (&p))
        ohshit (1, "application_destroy");
}

void sim_disconnection (int argc, char **argv, char **env, pid_t pid, int index, int version)
{
    struct ipc_service srv;
    memset (&srv, 0, sizeof (struct ipc_service));
    ipc_server_init (argc, argv, env, &srv, PUBSUB_SERVICE_NAME, NULL);
    printf ("Disconnecting from %s.\n", srv.spath);

    struct ipc_client p;
    memset (&p, 0, sizeof (struct ipc_client));

    // create the fake process
    ipc_server_process_gen (&p, pid, index, version);

    // send a message to disconnect
    // line : pid index version action chan
    pubsub_disconnect (&p);
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
#endif
