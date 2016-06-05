#include "pubsubd.h"
#include <stdlib.h>

const char* service_name = "pubsub";

void
ohshit(int rvalue, const char* str) {
    fprintf(stderr, "%s\n", str);
    exit(rvalue);
}

// init lists
void pubsubd_channels_init (struct channels *chans) { LIST_INIT(chans); }
void pubsubd_subscriber_init (struct app_list *al) { LIST_INIT(al); } 

int
pubsubd_channels_eq (const struct channel *c1, const struct channel *c2)
{
    return (strncmp (c1->chan, c2->chan, c1->chanlen) == 0);
}
struct channels * pubsubd_channels_copy (struct channels *c);

void
pubsubd_channels_add (struct channels *chans, struct channels *c)
{
    if(!chans || !c)
        return;

    struct process *n = pubsubd_channels_copy (c);
    LIST_INSERT_HEAD(al, n, entries);
}

void
pubsubd_subscriber_del (struct app_list *al, struct process *p)
{
    struct process *todel = srv_subscriber_get (al, p);
    if(todel != NULL) {
        LIST_REMOVE(todel, entries);
        srv_process_free (mfree, todel);
        mfree (todel);
        todel = NULL;
    }
}

void
pubsubd_subscriber_add (struct app_list *al, struct process *p)
{
    if(!al || !p)
        return;

    struct process *n = srv_process_copy (p);
    LIST_INSERT_HEAD(al, n, entries);
}

struct process *
pubsubd_subscriber_get (const struct app_list *al
        , const struct process *p)
{
    struct process *np, *res = NULL;
    LIST_FOREACH(np, al, entries) {
        if(srv_process_eq (np, p)) {
            res = np;
        }
    }
    return res;
}

void
pubsubd_subscriber_del (struct app_list *al, struct process *p)
{
    struct process *todel = srv_subscriber_get (al, p);
    if(todel != NULL) {
        LIST_REMOVE(todel, entries);
        srv_process_free (mfree, todel);
        mfree (todel);
        todel = NULL;
    }
}

void pubsubd_msg_send (struct service *s, struct message * m, struct process *p)
{
}
void pubsubd_msg_recv (struct service *s, struct message * m, struct process *p)
{
}
void pubsub_msg_send (struct service *s, struct message * m)
{
}
void pubsub_msg_recv (struct service *s, struct message * m)
{
}

    int
main(int argc, char* argv[])
{
    // gets the service path, such as /tmp/<service>
    char s_path[PATH_MAX];
    service_path (s_path, service_name);
    printf ("Listening on %s.\n", s_path);

    // creates the service named pipe, that listens to client applications
    if (service_create (s_path))
        ohshit(1, "service_create error");

    struct channels chans;
    pubsubd_channels_init (&chans);

    for (;;) {
        struct process proc;
        int proc_count, i;

        service_get_new_process (&proc, s_path);

        printf("> %i proc\n", proc_count);

        for (i = 0; i < proc_count; i++) {
            size_t message_size = BUFSIZ;
            char buffer[BUFSIZ];

            process_print(proc + i);

            if (process_read (&proc[i], &buffer, &message_size))
                ohshit(1, "process_read error");

            printf(": %s\n", buffer);


        }

        service_free_processes(&proc, proc_count);
    }

    // the application will shut down, and remove the service named pipe
    if (service_close (s_path))
        ohshit(1, "service_close error");

    return EXIT_SUCCESS;
}


/*
 * main loop
 *
 * opens the application pipes,
 * reads then writes the same message,
 * then closes the pipes
 */

void main_loop (const char *spath)
{
    int ret;
    struct process proc;

    int cnt = 10;

    while (cnt--) {
        // -1 : error, 0 = no new process, 1 = new process
        ret = service_get_new_process (&proc, spath);
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
        if ((ret = service_read (&proc, &buf, &msize))) {
            fprintf(stdout, "error service_read %d\n", ret);
            continue;
        }
        // printf ("after read\n");
        printf ("read, size %ld : %s\n", msize, buf);

        // printf ("before proc write\n");
        if ((ret = service_write (&proc, &buf, msize))) {
            fprintf(stdout, "error service_write %d\n", ret);
            continue;
        }
        // printf ("after proc write\n");
        printf ("\033[32mStill \033[31m%d\033[32m applications to serve\n",cnt);
    }
}
