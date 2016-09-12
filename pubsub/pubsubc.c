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

void print_cmd (void) {
    printf ("\033[32m>\033[00m ");
    fflush (stdout);
}

void * listener (void *params)
{
    int s = 0;
    s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (s != 0)
        printf ("pthread_setcancelstate: %d\n", s);

    struct process *p = NULL;
    p = (struct process *) params;
    if (p == NULL) {
        fprintf (stderr, "listener: no process\n");
        return NULL;
    }

    // main loop
    while (1) {
        struct pubsub_msg m;
        memset (&m, 0, sizeof (struct pubsub_msg));

        pubsub_msg_recv (p, &m);
        printf ("\n\033[31m>\033[00m %s\n", m.data);
        print_cmd ();

        // if (m.type == PUBSUB_TYPE_DISCONNECT) { }
        pubsubd_msg_free (&m);
    }

    pthread_exit (NULL);
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

    pthread_t thr;
    memset (&thr, 0, sizeof (pthread_t));

    pthread_create (&thr, NULL, listener, &p);
    pthread_detach (thr);

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
        print_cmd ();
        fflush (stdout);

        size_t mlen = read (0, buf, BUFSIZ);

        if (mlen > 1) {
            mlen--;
        }
        buf[mlen] = '\0';

        // TODO debug
        // printf ("data (%ld): %s\n", mlen, buf);

        if (strncmp(buf, "quit", strlen ("quit")) == 0) {
            break;
        }

        msg.data = malloc (strlen (buf) + 1);
        memset (msg.data, 0, strlen (buf) + 1);
        strncpy ((char *) msg.data, buf, strlen (buf) + 1);
        msg.datalen = strlen (buf);

        // TODO debug
        // printf ("send message\n");
        pubsub_msg_send (&p, &msg);
        free (msg.data);
        msg.data = NULL;
        msg.datalen = 0;
    }

    // free everything
    pubsubd_msg_free (&msg);

    pthread_cancel (thr);
    pthread_join (thr, NULL);

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
