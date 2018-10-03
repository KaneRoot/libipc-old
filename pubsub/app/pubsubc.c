// int main(void) { return 0; }

// TODO: select on service + input instead of threads

#include "../../core/error.h"
#include "../lib/pubsub.h"
#include "../lib/pubsubd.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void usage (char **argv) {
    printf ( "usage: %s [chan [pub]]\n", argv[0]);
}

void print_cmd (void) {
    printf ("\033[32m>\033[00m ");
    fflush (stdout);
}

void * listener (void *params)
{
    int s = 0;
    s = pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
    if (s != 0) {
        handle_err ("listener", "pthread_setcancelstate != 0");
    }

    struct service *srv = NULL;
    srv = (struct service *) params;
    if (srv == NULL) {
        handle_err ("listener", "no service passed");
        return NULL;
    }

    // main loop
    while (1) {
        struct pubsub_msg m;
        memset (&m, 0, sizeof (struct pubsub_msg));

        pubsub_message_recv (srv, &m);
        printf ("\r\033[31m>\033[00m %s\n", m.data);
        print_cmd ();

        pubsub_message_free (&m);
    }

    pthread_exit (NULL);
}

void chan_sub (struct service *srv, char *chan)
{
    struct pubsub_msg msg;
    memset (&msg, 0, sizeof (struct pubsub_msg));

    // meta data on the message
    msg.type = PUBSUB_MSG_TYPE_SUB;
    msg.chanlen = strlen (chan) + 1;
    msg.chan = malloc (msg.chanlen);
    memset (msg.chan, 0, msg.chanlen);
    strncpy (msg.chan, chan, msg.chanlen);
    msg.chan[strlen (chan)] = '\0';

    pubsub_message_send (srv, &msg);
    printf ("subscribed to %s\n", chan);

    pubsub_message_free (&msg);
}

void main_loop (int argc, char **argv, char **env
        , int index, int version
        , char *cmd, char *chan)
{
    printf ("connection to pubsubd: index %d version %d "
            "cmd %s chan %s\n"
          , index, version, cmd, chan );

    struct service srv;
    memset (&srv, 0, sizeof (struct service));
    pubsub_connection (argc, argv, env, &srv);
    printf ("connected\n");

    if (strncmp (cmd, "sub", 3) == 0) {
        chan_sub (&srv, chan);
    }

    pthread_t thr;
    memset (&thr, 0, sizeof (pthread_t));

    pthread_create (&thr, NULL, listener, &srv);
    pthread_detach (thr);

    printf ("main_loop\n");

    struct pubsub_msg msg;
    memset (&msg, 0, sizeof (struct pubsub_msg));

    // meta data on the message
    msg.type = PUBSUB_MSG_TYPE_PUB;
    msg.chanlen = strlen (chan) + 1;
    msg.chan = malloc (msg.chanlen);
    memset (msg.chan, 0, msg.chanlen);
    strncpy ((char *) msg.chan, chan, msg.chanlen);
    msg.chan[strlen (chan)] = '\0';

    // msg loop
    for (;;) {
        char buf[BUFSIZ];
        memset (buf, 0, BUFSIZ);
        print_cmd ();
        fflush (stdout);

        size_t mlen = read (0, buf, BUFSIZ);

        // remove \n
        if (mlen > 1) {
            mlen--;
        }
        buf[mlen] = '\0';

        if (strncmp(buf, "quit", strlen ("quit")) == 0) {
            break;
        }

        msg.datalen = strlen (buf) + 1;
        msg.data = malloc (msg.datalen);
        memset (msg.data, 0, msg.datalen);
        strncpy ((char *) msg.data, buf, msg.datalen);
        msg.data[strlen(buf)] = '\0';

        pubsub_message_send (&srv, &msg);
        free (msg.data);
        msg.data = NULL;
        msg.datalen = 0;
    }

    // free everything
    pubsub_message_free (&msg);

    pthread_cancel (thr);
    pthread_join (thr, NULL);

    printf ("disconnection...\n");
    // disconnect from the server
    pubsub_disconnect (&srv);
}

int main(int argc, char **argv, char **env)
{
    char *cmd = "sub";
    char *chan = "chan1";

    if (argc == 2 && strncmp("-h", argv[1], 2) == 0) {
        usage (argv);
        exit (0);
    }

    if (argc >= 2) {
        chan = argv[1];
    }

    if (argc >= 3) {
        cmd = argv[2];
    }

    int index = 0;
    // don't care about the version
    int version = 0;

    main_loop (argc, argv, env, index, version, cmd, chan);

    return EXIT_SUCCESS;
}
