#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ipc.h"

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(_GNU_SOURCE)
#else
extern char *__progname;
#endif

char * log_get_logfile_dir (char *buf, size_t size)
{
	char * datadir = getenv ("XDG_DATA_HOME");
	if (datadir == NULL) {
		datadir = getenv ("HOME");
		if (datadir == NULL) {
			return buf + snprintf (buf, size, "./ipc/");
		}
		else {
			return buf + snprintf (buf, size, "%s/.local/share/ipc/", datadir);
		}
	}
	return buf + snprintf (buf, size, "%s/ipc/", datadir);
}

void log_get_logfile_name (char *buf, size_t size) {
	char *logfile = getenv ("IPC_LOGFILE");
	if (logfile == NULL) {
		char *buf_after_dir;
		buf_after_dir = log_get_logfile_dir (buf, size);
		pid_t pid = getpid();

#if defined(__APPLE__) || defined(__FreeBSD__)
		const char * appname = getprogname();
#elif defined(_GNU_SOURCE)
		const char * appname = program_invocation_name;
#elif defined(__linux__)
		const char * appname = __progname;
#else
#error "cannot know the application name for this environment"
#endif

		snprintf (buf_after_dir, size - (buf_after_dir - buf), "%s-%d.log", appname, pid);
		return;
	}

	snprintf (buf, size, "%s", logfile);
}

void log_format (const char* tag, const char* message, va_list args) {
    time_t now;

    time(&now);

	struct tm *t = localtime (&now);

	SECURE_BUFFER_DECLARATION (char, date, 200);

	snprintf (date, 200, "%d-%02d-%02d_%02d-%02d-%02d"
			, t->tm_year+1900, t->tm_mon +1, t->tm_mday
			, t->tm_hour, t->tm_min, t->tm_sec);

	SECURE_BUFFER_DECLARATION (char, logfile, BUFSIZ);
	log_get_logfile_name (logfile, BUFSIZ);

	FILE * logfd = fopen (logfile, "a");
	if (logfd == NULL) {
		fprintf (stderr, "something gone horribly wrong: cannot open logfile: %s\n", logfile);
		return;
	}

    fprintf (logfd, "%s:%s:", date, tag);
    vfprintf(logfd, message, args);
    fprintf (logfd, "\n");

	fflush (logfd);

	fclose (logfd);
}

void log_error (const char* message, ...) {
    va_list args;
    va_start(args, message);

    log_format("error", message, args);

    va_end(args);
}

void log_info (const char* message, ...) {
    va_list args;
    va_start(args, message);

    log_format("info", message, args);
    va_end(args);
}

void log_debug (const char* message, ...) {
    va_list args;
    va_start(args, message);

    log_format("debug", message, args);

    va_end(args);
}
