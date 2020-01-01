#include "../src/fs.h"

void print_usage (char *progname)
{
	printf ("usage: %s path\n", progname);
}

int main (int argc, char **argv)
{
	if (argc == 1) {
		print_usage (argv[0]);
		exit (0);
	}

	if (strncmp (argv[1], "-h", 2) == 0) {
		print_usage (argv[0]);
		exit (0);
	}

	printf ("directory_setup_ (\"%s\")\n", argv[1]);

	struct ipc_error ret = directory_setup_ (argv[1]);
	if (ret.error_code != 0) {
		fprintf (stderr, "an error occured: %s\n", ret.error_message);
		exit (1);
	}

	return 0;
}
