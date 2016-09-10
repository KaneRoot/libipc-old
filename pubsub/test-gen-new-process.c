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

int
main(int argc, char **argv)
{
    if (argc != 2) {
        usage (argv);
    }

    char *spath = argv[1];

    printf ("Listening on %s.\n", spath);

    struct channels chans;
    memset (&chans, 0, sizeof (struct channels));

    for (int nb = 1, i = 0 ; nb > 0; i++, nb--) {
        struct app_list_elm ale;
        memset (&ale, 0, sizeof (struct app_list_elm));

        struct channel chan;
        memset (&chan, 0, sizeof (struct channel));
        struct channel *c = &chan;

        pubsubd_get_new_process (spath, &ale, &chans, &c);

        printf ("print the channels, %d chan\n", i);
        printf ("--\n");
        pubsubd_channels_print (&chans);
        printf ("--\n");
        printf ("still %d remaining processes\n", nb);

        pubsubd_app_list_elm_free (&ale);
    }

    pubsubd_channels_del_all (&chans);

    return EXIT_SUCCESS;
}

