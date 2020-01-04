#include <string.h>
#include <assert.h>

// error_message_format
#include "utils.h"
#include "fs.h"

/**
	Private functions finish with an underscore
 */

int exists_ (const char *path, struct stat *status)
{
	assert (status != NULL);
	return stat (path, status) == 0;
}

int is_directory_ (struct stat *status)
{
	assert (status != NULL);
	return S_ISDIR (status->st_mode);
}

int is_writable_ (struct stat *status)
{
	assert (status != NULL);
	// what is my effective uid?
	uid_t uid = geteuid ();
	gid_t gid = getegid ();

	if (status->st_uid == uid) {
		// does the user can write?
		return (status->st_mode & S_IWUSR) != 0;
	}

	if (status->st_gid == gid) {
		// does the group can write?
		return (status->st_mode & S_IWGRP) != 0;
	}

	return (status->st_mode & S_IWOTH) != 0;
}

/** avoiding name clashes */
int dirname_ (const char *path, char *dname)
{
	assert (dname != NULL);

	char *last_slash = strrchr (path, '/');
	int pathlen = last_slash - path;

	snprintf (dname, pathlen + 1, "%s", path);

	return 0;
}

struct ipc_error mkdir_p_ (const char *path)
{
	int dir_rights = 01750;

	// notes:
	// S_ISGID
	//   files and sub-directories are automatically in the same group of the root directory
	// S_ISVTX = "sticky bit"
	//   files created in this dir can only be removed by the owner or the directory owner
	//   so in /tmp even people with writing rights cannot remove other's content

	SECURE_BUFFER_DECLARATION (char, _path, BUFSIZ);
	const size_t len = strlen (path);
	char *p;

	errno = 0;

	/* copy the string so it becomes mutable */
	if (len > sizeof (_path) - 1) {
		errno = ENAMETOOLONG;
		IPC_RETURN_ERROR (IPC_ERROR_MKDIR__NAME_TOO_LONG);
	}
	strcpy (_path, path);

	/* loop over the string, each time we have a slash we can mkdir */
	for (p = _path + 1; *p; p++) {
		if (*p == '/') {
			/**
				truncate the path to conserve the path we want to create only
			*/
			*p = '\0';

			if (mkdir (_path, dir_rights) != 0) {
				if (errno != EEXIST)
					IPC_RETURN_ERROR_FORMAT (IPC_ERROR_MKDIR__CANNOT_CREATE_DIR
						, "mkdir_p_: cannot create the directory %s", _path);
			}

			*p = '/';
		}
	}

	if (mkdir (_path, dir_rights) != 0) {
		// actually, in our code this error shouldn't be possible
		// since it already has been tested
		if (errno != EEXIST)
			IPC_RETURN_ERROR_FORMAT (IPC_ERROR_MKDIR__CANNOT_CREATE_DIR
				, "mkdir: cannot create the directory %s", _path);
	}

	IPC_RETURN_NO_ERROR;
}

/** Paramater is the path of the file we want to create
 * A trailing slash '/' is required if the full path is itself a directory we need to create.
 */
struct ipc_error directory_setup_ (const char *path)
{
	T_R ((path == NULL), IPC_ERROR_DIRECTORY_SETUP__PATH_PARAM);

	SECURE_BUFFER_DECLARATION (char, dir, BUFSIZ);
	dirname_ (path, dir);

	SECURE_DECLARATION (struct stat, status);

	/** If the directory doesn't exists, yet */
	if (!exists_ (dir, &status)) {
		/** create the whole path */
		return mkdir_p_ (dir);
	}

	/** The path exists, is it a directory? */
	if (!is_directory_ (&status)) {
		IPC_RETURN_ERROR_FORMAT (IPC_ERROR_DIR_SETUP__NOT_A_DIRECTORY
			, "directory_setup_: path %s is not a directory", dir);
	}

	/** The path exists and it is a directory. Do we have the rights to write in it? */
	if (!is_writable_ (&status)) {
		IPC_RETURN_ERROR_FORMAT (IPC_ERROR_DIR_SETUP__DIRECTORY_NOT_WRITABLE
			, "directory_setup_: directory %s is not writable", dir);
	}

	IPC_RETURN_NO_ERROR;
}
