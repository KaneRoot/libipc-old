#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char * argv[])
{

    char *fifopathin = "/tmp/123000-1-in";
    size_t msize;

    FILE *out = fopen (fifopathin, "wb");

    printf ("opened\n");

    char *buf = "coucou";
    printf ("write %s\n", buf);

    if ((msize = fread (buf, 6, 1, out))) {
        printf ("error read %ld\n", msize);
        return EXIT_FAILURE;
    }

    sleep (10);
    printf ("write end\n");

    fclose (out);

    return EXIT_SUCCESS;
}
