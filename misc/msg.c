#include "../lib/pubsubd.h"
#include "cbor.h"
#include <stdlib.h>
#include <string.h>

#define PKT_CLOSE                   0
#define PKT_MSG                     1
#define PKT_ERROR                   2

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

int
main(int argc, char **argv)
{
    if (argc == 2 && strcmp ("-h", argv[1]) == 0) {
        usage (argv);
        exit (1);
    }

    unsigned char buf[BUFSIZ];
    memset (buf, 0, BUFSIZ);

    ssize_t buflen = read (0, buf, BUFSIZ);

    /* Preallocate the map structure */
    cbor_item_t * root = cbor_new_definite_map(1);
    /* Add the content */
    cbor_map_add(root, (struct cbor_pair) {
            .key = cbor_move(cbor_build_uint8(PKT_MSG)),
            .value = cbor_move(cbor_build_bytestring(buf, buflen))
            });
    /* Output: `length` bytes of data in the `buffer` */
    unsigned char * buffer;
    size_t buffer_size, length = cbor_serialize_alloc (root, &buffer, &buffer_size);

    fwrite(buffer, 1, length, stdout);
    free(buffer);

    fflush(stdout);
    cbor_decref(&root);

    return EXIT_SUCCESS;
}
