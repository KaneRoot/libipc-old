#include "ipc.h"

struct ipc_error
service_path (char *path, const char *sname)
{
	T_R ((path == NULL), IPC_ERROR_SERVICE_PATH__NO_PATH);
	T_R ((sname == NULL), IPC_ERROR_SERVICE_PATH__NO_SERVICE_NAME);

	memset (path, 0, PATH_MAX);

	char *rundir = getenv ("IPC_RUNDIR");
	if (rundir == NULL)
		rundir = RUNDIR;

	snprintf (path, PATH_MAX - 1, "%s/%s", rundir, sname);

	IPC_RETURN_NO_ERROR;
}

