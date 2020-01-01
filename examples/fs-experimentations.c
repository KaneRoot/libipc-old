#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <dirent.h>

#include <limits.h>		/* PATH_MAX */
#include <sys/stat.h>		/* mkdir(2) */
#include <sys/types.h>
#include <errno.h>

void print_content (const char *dirname)
{
	DIR *dir;
	struct dirent *entry;
	dir = opendir (dirname);
	if (dir == NULL)
		return;
	printf ("%s :\n", dirname);

	while ((entry = readdir (dir)) != NULL) {
		printf ("	%s\n", entry->d_name);
	}
	printf ("\n");

	closedir (dir);
}

int mkdir_p (const char *path)
{
	int dir_rights = 01750;

	// notes:
	// S_ISGID
	//   files and sub-directories are automatically in the same group of the root directory
	// S_ISVTX = "sticky bit"
	//   files created in this dir can only be removed by the owner or the directory owner
	//   so in /tmp even people with writing rights cannot remove other's content

	char _path[BUFSIZ];
	const size_t len = strlen (path);
	char *p;

	errno = 0;

	/* copy the string so it becomes mutable */
	if (len > sizeof (_path) - 1) {
		errno = ENAMETOOLONG;
		return -1;
	}
	strcpy (_path, path);

	/* loop over the string, each time we have a slash we can mkdir */
	for (p = _path + 1; *p; p++) {
		if (*p == '/') {
			/* Temporarily truncate */
			*p = '\0';

			if (mkdir (_path, dir_rights) != 0) {
				if (errno != EEXIST)
					return -1;
			}

			*p = '/';
		}
	}

	if (mkdir (_path, dir_rights) != 0) {
		if (errno != EEXIST)
			return -1;
	}

	return 0;
}

void rights (struct stat *status)
{
	printf (status->st_mode & S_IRUSR ? "r" : "-");
	printf (status->st_mode & S_IWUSR ? "w" : "-");
	printf (status->st_mode & S_IXUSR ? "x" : "-");
	printf (status->st_mode & S_IRGRP ? "r" : "-");
	printf (status->st_mode & S_IWGRP ? "w" : "-");
	printf (status->st_mode & S_IXGRP ? "x" : "-");
	printf (status->st_mode & S_IROTH ? "r" : "-");
	printf (status->st_mode & S_IWOTH ? "w" : "-");
	printf (status->st_mode & S_IXOTH ? "x" : "-");
	printf ("\n");
}

void print_status (struct stat *status)
{
	if (S_ISBLK (status->st_mode))
		printf ("block ");
	else if (S_ISBLK (status->st_mode))
		printf ("ISBLK ");
	else if (S_ISCHR (status->st_mode))
		printf ("ISCHR ");
	else if (S_ISDIR (status->st_mode))
		printf ("ISDIR ");
	else if (S_ISFIFO (status->st_mode))
		printf ("ISFIFO ");
	else if (S_ISLNK (status->st_mode))
		printf ("ISLNK ");
	else if (S_ISREG (status->st_mode))
		printf ("ISREG ");
	else if (S_ISSOCK (status->st_mode))
		printf ("ISSOCK ");
	else
		printf ("CANNOT SEE THE FILE TYPE :(\n");

	rights (status);
}

int test_with_stat (char *filename, struct stat *status)
{
	int ret = stat (filename, status);
	if (ret < 0) {
		printf ("file not found or non accessible\n");
		return 1;
	}

	return 0;
}

int main (int argc, char *argv[], char *env[])
{
	env = env;

	if (argc <= 1) {
		printf ("usage: %s directory\n", argv[0]);
		return 0;
	}

	struct stat status;
	memset (&status, 0, sizeof (struct stat));
	if (test_with_stat (argv[1], &status)) {
		printf ("This is neither a directory nor a file, creating directory\n");
		int ret = mkdir_p (argv[1]);
		if (ret < 0) {
			printf ("There is an error: cannot create the directory %s\n", argv[1]);
			exit (1);
		} else {
			printf ("Directory %s created\n", argv[1]);
		}
		print_content (argv[1]);
	} else {
		print_status (&status);
	}

	return 0;
}
