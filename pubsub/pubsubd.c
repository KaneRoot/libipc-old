#include "../lib/pubsubd.h"
#include <stdlib.h>
#include <pthread.h>

#define NB_CLIENTS                                      3

void
ohshit(int rvalue, const char* str) {
    fprintf(stderr, "%s\n", str);
    exit(rvalue);
}

// give this structure to the thread worker function
struct worker_params {
    struct channels *chans;
    struct channel *chan;
    struct app_list_elm *ale;
};

void * pubsubd_worker_thread (void *params)
{
    int s = 0;

    s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (s != 0)
        printf ("pthread_setcancelstate: %d\n", s);

    struct worker_params *wp = (struct worker_params *) params;
    if (wp == NULL) {
        fprintf (stderr, "error pubsubd_worker_thread : params NULL\n");
        return NULL;
    }

    struct channels *chans = wp->chans;
    struct channel *chan = wp->chan;
    struct app_list_elm *ale = wp->ale;

    free (wp);

    // main loop
    while (1) {
        struct pubsub_msg m;
        memset (&m, 0, sizeof (struct pubsub_msg));

        sleep (2); // TODO DEBUG
        pubsubd_msg_recv (ale->p, &m);

        if (m.type == PUBSUB_TYPE_DISCONNECT) {
            printf ("process %d disconnecting...\n", ale->p->pid);
            if ( 0 != pubsubd_subscriber_del (chan->alh, ale)) {
                fprintf (stderr, "err : subscriber not registered\n");
            }
            break;
        }
        else {
            struct channel *ch = pubsubd_channel_search (chans, chan->chan);
            if (ch == NULL) {
                printf ("CHAN NON TROUVE\n");
            }
            else {
                printf ("Je dois dire :\n");
                pubsubd_msg_print (&m);
                printf ("CHAN ET PROCESS À QUI JE DOIS COMMUNIQUER\n");
                pubsubd_channel_print (ch);
                pubsubd_msg_send (ch->alh, &m);
            }
        }
    }
#if 0
#endif

#if 0
    while (1) {
        // each chan has a list of subscribers
        // someone who only push a msg doesn't need to be registered
        // if (wp->ale->action == PUBSUB_BOTH || wp->ale->action == PUBSUB_PUB) {
        if (wp->ale->action == PUBSUB_PUB) {
            // publish a message
            printf ("publish to chan %s\n", wp->chan->chan);

            struct pubsub_msg m;
            memset (&m, 0, sizeof (struct pubsub_msg));

            sleep (5); // TODO DEBUG
            pubsubd_msg_recv (wp->ale->p, &m);

            pubsubd_msg_print (&m);

            if (m.type == PUBSUB_TYPE_DISCONNECT) {
                // TODO remove the application from the subscribers
                if ( 0 != pubsubd_subscriber_del (wp->chan->alh, wp->ale)) {
                    fprintf (stderr, "err : subscriber not registered\n");
                }
                break;
            }
            else {
                struct channel *chan = pubsubd_channel_get (wp->chans, wp->chan);
                pubsubd_msg_send (chan->alh, &m);
            }
        }
        else if (wp->ale->action == PUBSUB_SUB) {
            // subscribe to a channel, no need to loop
            // already subscribed
            // printf ("subscribe to %s\n", wp->chan->chan);
            // pubsubd_subscriber_add (wp->chan->alh, wp->ale);
            break;
        }
        else {
            // unrecognized command, no need to loop
            printf ("\033[31mdo not know what you want to do\033[00m\n");
            printf ("\tale->p : %p\n", (void*) wp->ale->p);
            break;
        }
    }
#endif

    pubsubd_app_list_elm_free (ale);

    pthread_exit (NULL);
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

    pthread_t *thr = NULL;
    thr = malloc (sizeof (pthread_t) * NB_CLIENTS);
    memset (thr, 0, sizeof (pthread_t) * NB_CLIENTS);

    for (int i = 0; i < NB_CLIENTS; i++) {
        // for each new process
        struct app_list_elm ale;
        memset (&ale, 0, sizeof (struct app_list_elm));
        struct channel *chan = NULL;
        pubsubd_get_new_process (srv.spath, &ale, &chans, &chan);
        pubsubd_channels_print (&chans);

        // end the application
        if (ale.action == PUBSUB_QUIT) {
            printf ("Quitting ...\n");

            pubsubd_channels_del_all (&chans);
            srv_close (&srv);
            
            // TODO end the threads
            exit (0);
        }

        // TODO thread to handle multiple clients at a time
        struct worker_params *wp = NULL;
        wp = malloc (sizeof (struct worker_params));
        wp->ale = pubsubd_app_list_elm_copy (&ale);
        wp->chans = &chans;
        wp->chan = chan;

        pthread_create (thr + i, NULL, pubsubd_worker_thread, wp);
        pthread_detach (thr[i]);

        pubsubd_app_list_elm_free (&ale);
    }

    sleep (10);
    // stop threads
    for (int i = 0 ; i < NB_CLIENTS ; i++) {
        pthread_cancel (thr[i]);
        void *ret = NULL;
        pthread_join (thr[i], &ret);
        if (ret != NULL) {
            free (ret);
        }
    }
    free (thr);

    printf ("QUIT\n");
    pubsubd_channels_del_all (&chans);

    // the application will shut down, and remove the service named pipe
    if (srv_close (&srv))
        ohshit (1, "service_close error");

    return EXIT_SUCCESS;
}

#if 0
void main_loop (const struct service *srv)
{
    int ret;
    struct process proc;

    int cnt = 10;

    while (cnt--) {
        // -1 : error, 0 = no new process, 1 = new process
        ret = srv_get_new_process (&proc, srv);
        if (ret == -1) {
            fprintf (stderr, "error service_get_new_process\n");
            continue;
        } else if (ret == 0) { // that should not happen
            continue;
        }

        // printf ("before print\n");
        process_print (&proc);
        // printf ("after print\n");

        // about the message
        size_t msize = BUFSIZ;
        char buf[BUFSIZ];
        bzero(buf, BUFSIZ);

        // printf ("before read\n");
        if ((ret = srv_read (&proc, &buf, &msize))) {
            fprintf(stdout, "error service_read %d\n", ret);
            continue;
        }
        // printf ("after read\n");
        printf ("read, size %ld : %s\n", msize, buf);

        // printf ("before proc write\n");
        if ((ret = srv_write (&proc, &buf, msize))) {
            fprintf(stdout, "error service_write %d\n", ret);
            continue;
        }

        // printf ("after proc write\n");
        printf ("\033[32mStill \033[31m%d\033[32m applications to serve\n",cnt);
    }
}
#endif
