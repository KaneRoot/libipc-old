#include "utils.h"

void print_hexa (const char *prefix, unsigned char *payload, size_t size)
{
    if (! payload)
        return ;

    size_t i;
    for(i = 0; i < size; i++)
    {
        if(! (i % 4))
            printf("\n%s (%ld) ", prefix, size);
        printf("%2x ", payload[i]);
    }
    printf("\n");
}


void mprint_hexa (char *prefix, unsigned char *buf, size_t length)
{
    print_hexa (prefix, buf, length);
}
