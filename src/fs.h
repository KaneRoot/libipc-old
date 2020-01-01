#ifndef __FS_H__
#define __FS_H__

#include "ipc.h"

#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

struct ipc_error mkdir_p_ (const char *path);
struct ipc_error directory_setup_ (const char *dir);

int exists_ (const char *path, struct stat *status);
int is_directory_ (struct stat *status);
int is_writable_ (struct stat *status);
int dirname_ (const char *path, char *dname);

#endif
