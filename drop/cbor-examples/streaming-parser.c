#include "cbor.h"
#include <stdio.h>
#include <string.h>

/*
 *  * Illustrates how one might skim through a map (which is assumed to have
 *   * string keys and values only), looking for the value of a specific key
 *    *
 *     * Use the examples/data/map.cbor input to test this.
 *      */

const char * key = "a secret key";
bool key_found = false;

void find_string(void * _ctx, cbor_data buffer, size_t len)
{
    (void) _ctx;
    if (key_found) {
        printf("Found the value: %*s\n", (int) len, buffer);
        key_found = false;
    } else if (len == strlen(key)) {
        key_found = (memcmp(key, buffer, len) == 0);
    }
}

int main(int argc, char * argv[])
{
    (void) argc;
    (void) argv;

    FILE * f = fopen(argv[1], "rb");
    fseek(f, 0, SEEK_END);
    size_t length = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char * buffer = malloc(length);
    fread(buffer, length, 1, f);

    struct cbor_callbacks callbacks = cbor_empty_callbacks;
    struct cbor_decoder_result decode_result;
    size_t bytes_read = 0;
    callbacks.string = find_string;
    while (bytes_read < length) {
        decode_result = cbor_stream_decode(buffer + bytes_read,
                length - bytes_read,
                &callbacks, NULL);
        bytes_read += decode_result.read;
    }

    fclose(f);
}
