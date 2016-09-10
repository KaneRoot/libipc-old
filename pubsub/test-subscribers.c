#include "../lib/pubsubd.h"
#include <stdlib.h>
#include <string.h>

void
ohshit(int rvalue, const char* str) {
    fprintf(stderr, "%s\n", str);
    exit(rvalue);
}

void usage (char **argv) {
    fprintf (stderr, "usage: %s path\n", argv[0]);
    exit (1);
}

// // element of the list
// // channel : chan name + chan name length + a list of applications
// struct channel {
//     char *chan;
//     size_t chanlen;
//     struct app_list_head *alh;
//     LIST_ENTRY(channel) entries;
// };

// struct process {
//     pid_t pid;
//     unsigned int version;
//     unsigned int index;
//     char path_in [PATH_MAX];
//     char path_out [PATH_MAX];
// };

void fill_process (struct process *p)
{
    p->pid = 10;
    p->version = 1;
    p->index = 1;
    memcpy (p->path_in, "pathin", strlen ("pathin") +1);
    memcpy (p->path_out, "pathout", strlen ("pathout") +1);
}

// enum app_list_elm_action {PUBSUB_QUIT = 1, PUBSUB_PUB, PUBSUB_SUB, PUBSUB_BOTH};
// struct app_list_elm {
//     struct process *p;
//     enum app_list_elm_action action;
//     LIST_ENTRY(app_list_elm) entries;
// };

void fill_app_list_elm (struct app_list_elm *ale)
{
    ale->p = malloc (sizeof (struct process));
    fill_process (ale->p);
    ale->action = PUBSUB_PUB;
}

int main(void)
{
    struct app_list_head alh;
    memset (&alh, 0, sizeof (struct app_list_head));

    struct app_list_elm ale;
    memset (&ale, 0, sizeof (struct app_list_elm));

    fill_app_list_elm (&ale);

    struct app_list_head *chans = &alh;
    pubsubd_subscriber_init (&chans);
    printf ("1 chan, 0 process\n");
    pubsubd_subscriber_print (chans);

    pubsubd_subscriber_add (&alh, &ale);
    printf ("1 chan, 1 process\n");
    pubsubd_subscriber_print (chans);

    pubsubd_subscriber_del_all (&alh);
    printf ("0 chan, 0 process\n");
    pubsubd_subscriber_print (chans);

    free (ale.p);

    return EXIT_SUCCESS;
}

