#include "../lib/pubsubd.h"
#include <stdlib.h>

#define TEST_NAME "test-chan-lists"

void
ohshit(int rvalue, const char* str) {
    fprintf(stderr, "%s\n", str);
    exit(rvalue);
}

int
main(int argc, char **argv, char **env)
{
    struct service srv;
    memset (&srv, 0, sizeof (struct service));
    srv_init (argc, argv, env, &srv, TEST_NAME, NULL);
    printf ("Listening on %s.\n", srv.spath);

    // creates the service named pipe, that listens to client applications
    if (srv_create (&srv))
        ohshit(1, "service_create error");

    // init chans list
    struct channels chans;
    memset (&chans, 0, sizeof (struct channels));
    pubsubd_channels_init (&chans);

    // for each new process
    // struct app_list_elm ale1;
    // memset (&ale1, 0, sizeof (struct app_list_elm));

    // warning : this is a local structure, not exactly the same in the prog.
    struct channel chan;
    memset (&chan, 0, sizeof (struct channel));
    pubsubd_channel_new (&chan, "coucou");

    // to emulate
    // pubsubd_get_new_process (&srv, &ale1, &chans, &chan);

    // FIRST CHAN TO BE ADDED
    // search for the chan in channels, add it if not found
    struct channel *new_chan = NULL;
    new_chan = pubsubd_channel_get (&chans, &chan);
    if (new_chan == NULL) {
        new_chan = pubsubd_channels_add (&chans, &chan);
        pubsubd_subscriber_init (&new_chan->alh);
    }
    else {
        ohshit (2, "error : new chan, can't be found in channels yet");
    }
    pubsubd_channel_free (&chan);

    printf ("print the channels, 1 chan\n");
    printf ("--\n");
    pubsubd_channels_print (&chans);
    printf ("--\n");

    // SAME CHAN, SHOULD NOT BE ADDED
    pubsubd_channel_new (&chan, "coucou");
    // search for the chan in channels, add it if not found
    new_chan = pubsubd_channel_get (&chans, &chan);
    if (new_chan == NULL) {
        ohshit (3, "error : same chan, shouldn't be added in channels");
    }
    else {
        printf ("already in the 'channels' structure\n");
    }

    printf ("print the channels, 1 chan\n");
    printf ("--\n");
    pubsubd_channels_print (&chans);
    printf ("--\n");

    // NEW CHAN, SHOULD BE ADDED
    pubsubd_channel_new (&chan, "salut");
    // search for the chan in channels, add it if not found
    new_chan = pubsubd_channel_get (&chans, &chan);
    if (new_chan == NULL) {
        new_chan = pubsubd_channels_add (&chans, &chan);
        pubsubd_subscriber_init (&new_chan->alh);
    }
    else {
        ohshit (4, "error : new chan, should be added in channels");
    }
    pubsubd_channel_free (&chan);

    printf ("print the channels, 2 chans\n");
    printf ("--\n");
    pubsubd_channels_print (&chans);
    printf ("--\n");

    // end the application
    pubsubd_channels_del_all (&chans);

    printf ("\nshould be empty now\n");
    printf ("--\n");
    pubsubd_channels_print (&chans);
    printf ("--\n");

    // the application will shut down, and remove the service named pipe
    if (srv_close (&srv))
        ohshit (1, "service_close error");

    return EXIT_SUCCESS;
}
