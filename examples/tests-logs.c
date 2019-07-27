#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/ipc.h"

int main(void)
{
	SECURE_BUFFER_DECLARATION (char, buffer, BUFSIZ);

	log_get_logfile_name (buffer, BUFSIZ);

	printf ("log: %s\n", buffer);

    return EXIT_SUCCESS;
}
