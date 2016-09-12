#include "../lib/pubsubd.h"
#include <stdlib.h>
#include <pthread.h>

void
ohshit(int rvalue, const char* str) {
    fprintf (stderr, "\033[31merr: %s\033[00m\n", str);
    exit (rvalue);
}

void usage (char **argv)
{
    printf ( "usage: %s\n", argv[0]);
}

void main_loop (int argc, char **argv, char **env
        , pid_t pid, int index, int version
        , char *cmd, char *chan)
{
    printf ("connection : pid %d index %d version %d "
            "cmd %s chan %s\n"
          , pid, index, version, cmd, chan );

    struct service srv;
    memset (&srv, 0, sizeof (struct service));
    srv_init (argc, argv, env, &srv, PUBSUB_SERVICE_NAME, NULL);
    printf ("Writing on %s.\n", srv.spath);

    struct process p;
    memset (&p, 0, sizeof (struct process));

    printf ("app creation\n");
    if (app_create (&p, pid, index, version)) // called by the application
        ohshit (1, "app_create");

    printf ("main_loop\n");
    // send a message to warn the service we want to do something
    // line : pid index version action chan
    enum app_list_elm_action action = PUBSUB_BOTH;

    if (strncmp (cmd, "pub", 3) == 0) {
        action = PUBSUB_PUB;
    }
    else if (strncmp (cmd, "sub", 3) == 0) {
        action = PUBSUB_SUB;
    }

    pubsub_connection (&srv, &p, action, chan);

    struct pubsub_msg msg;
    memset (&msg, 0, sizeof (struct pubsub_msg));


    // meta data on the message
    msg.type = PUBSUB_TYPE_MESSAGE;
    msg.chan = malloc (strlen (chan) + 1);
    memset (msg.chan, 0, strlen (chan) + 1);
    strncpy ((char *) msg.chan, chan, strlen (chan));
    msg.chan[strlen (chan)] = '\0';
    msg.chanlen = strlen (chan);

    // msg loop
    for (;;) {
        char buf[BUFSIZ];
        memset (buf, 0, BUFSIZ);
        printf ("msg to send (chan: %s) [quit]: ", msg.chan);
        fflush (stdout);

        size_t mlen = read (0, buf, BUFSIZ);

        printf ("data (%ld): %s\n", mlen, buf);

        if (strncmp(buf, "quit\n", strlen ("quit\n")) == 0) {
            break;
        }

        msg.data = malloc (strlen (buf) + 1);
        memset (msg.data, 0, strlen (buf) + 1);
        strncpy ((char *) msg.data, buf, strlen (buf) + 1);
        msg.datalen = strlen (buf);

        printf ("send message\n");
        pubsub_msg_send (&p, &msg);
        free (msg.data);
        msg.data = NULL;
        msg.datalen = 0;
    }

    // free everything
    pubsubd_msg_free (&msg);

    printf ("disconnection...\n");
    // disconnect from the server
    pubsub_disconnect (&p);

    printf ("destroying app\n");
    // the application will shut down, and remove the application named pipes
    if (app_destroy (&p))
        ohshit (1, "app_destroy");
}

int main(int argc, char **argv, char **env)
{

    if (argc != 1) {
        usage (argv);
        exit (1);
    }

    pid_t pid = getpid();
    int index = 0;
    // don't care about the version
    int version = COMMUNICATION_VERSION;
    char *cmd = "both";
    char *chan = "chan1";

    main_loop (argc, argv, env, pid, index, version, cmd, chan);

    return EXIT_SUCCESS;
}
