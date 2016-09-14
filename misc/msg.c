#include "../lib/pubsubd.h"
#include <stdlib.h>
#include <string.h>

void
ohshit(int rvalue, const char* str) {
    fprintf (stderr, "\033[31merr: %s\033[00m\n", str);
    exit (rvalue);
}

void usage (char **argv)
{
    printf ( "NOT DONE YET\n");
    printf ( "usage: %s [type [param]]...\n", argv[0]);
    printf ( "ex: echo data | %s char_t 1\n", argv[0]);
    printf ( "    This sends a CBOR msg [ 1, \"data\" ]\n");
}

/*
 *  implemented types:
 *      bstr_t (default)
 *      tstr_t
 *      int_t
 *
 *  future types:
 *      nint_t 
 */

int
main(int argc, char **argv)
{
    if (argc == 2 && strcmp ("-h", argv[1]) == 0) {
        usage (argv);
        exit (1);
    }

    return EXIT_SUCCESS;
}
