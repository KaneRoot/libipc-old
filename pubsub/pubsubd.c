#include "../lib/pubsubd.h"
#include <stdlib.h>

struct workers *my_workers;

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
    srv_init (argc, argv, env, &srv, PUBSUB_SERVICE_NAME, NULL);
    printf ("Listening on %s.\n", srv.spath);

    // creates the service named pipe, that listens to client applications
    if (srv_create (&srv))
        ohshit(1, "service_create error");

    // init chans list
    struct channels chans;
    memset (&chans, 0, sizeof (struct channels));
    pubsubd_channels_init (&chans);

    my_workers = malloc (sizeof (struct workers));
    memset (my_workers, 0, sizeof (struct workers));
    pubsubd_workers_init (my_workers);

    while (1) {
        // for each new process
        struct app_list_elm ale;
        memset (&ale, 0, sizeof (struct app_list_elm));
        struct channel *chan = NULL;
        pubsubd_get_new_process (srv.spath, &ale, &chans, &chan);
        pubsubd_channels_print (&chans);

        // end the application
        if (ale.action == PUBSUB_QUIT) {
            pubsubd_app_list_elm_free (&ale);
            break;
        }

        // thread to handle multiple clients at a time
        struct worker *w = NULL;
        w = malloc (sizeof (struct worker));
        w->thr = malloc (sizeof (pthread_t));
        memset (w->thr, 0, sizeof (pthread_t));
        w->ale = pubsubd_app_list_elm_copy (&ale);
        w->chans = &chans;
        w->my_workers = my_workers;
        w->chan = chan;
        struct worker *wtmp = pubsubd_workers_add (my_workers, w);
        pubsubd_worker_free (w);
        free (w);
        w = wtmp;

        pthread_create (w->thr, NULL, pubsubd_worker_thread, w);
        pthread_detach (*w->thr);

        pubsubd_app_list_elm_free (&ale);
    }

    printf ("Quitting ...\n");

    pubsubd_workers_stop (my_workers);
    pubsubd_channels_del_all (&chans);
    pubsubd_workers_del_all (my_workers);

    free (my_workers);

    // the application will shut down, and remove the service named pipe
    if (srv_close (&srv))
        ohshit (1, "srv_close error");

    return EXIT_SUCCESS;
}
