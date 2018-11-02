#include "../lib/pubsubd.h"
#include <cbor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PKT_CLOSE                   0
#define PKT_MSG                     1
#define PKT_ERROR                   2

void usage (char **argv) {
    printf ("usage: echo something | msg | %s\n", argv[0]);
}

int32_t main(int32_t argc, char * argv[])
{
    if (argc == 2 && strcmp ("-h", argv[1]) == 0) {
        usage (argv);
        exit (1);
    }

    // read the message from the client
    size_t mlen = 0;
    uint8_t buf[BUFSIZ];
    mlen = read (0, buf, BUFSIZ);

    /* Assuming `buffer` contains `info.st_size` bytes of input data */
    struct cbor_load_result result;
    cbor_item_t * item = cbor_load (buf, mlen, &result);

    /* Pretty-print the result */
    cbor_describe(item, stdout);
    fflush(stdout);

    struct cbor_pair * pair = cbor_map_handle (item);
    cbor_mutable_data *data = cbor_bytestring_handle (pair->value);

    size_t datalen = cbor_bytestring_length (pair->value);
    char *bstr = malloc (datalen +1);
    memset (bstr, 0, datalen +1);
    memcpy (bstr, data, datalen);

    printf ("msg data (%ld bytes): %s\n", datalen, bstr);

    /* Deallocate the result */
    cbor_decref (&item);

    free (bstr);
    return EXIT_SUCCESS;
}
