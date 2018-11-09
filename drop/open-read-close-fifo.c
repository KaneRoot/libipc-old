#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int32_t main(int32_t argc, char * argv[])
{

    (void) argc;
    (void) argv;

    char *fifopathin = "/tmp/123000-1-in";
    size_t msize = 100;

    char buf[BUFSIZ];
    FILE *in = fopen (fifopathin, "rb");

    printf ("opened\n");

    if ((msize = fread (buf, msize, 1, in))) {
        printf ("error read %ld\n", msize);
        return EXIT_FAILURE;
    }

    printf ("%s\n", buf);

    sleep (10);
    printf ("read end\n");

    fclose (in);

    return EXIT_SUCCESS;
}
