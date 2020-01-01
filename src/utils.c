#include "utils.h"
#include "ipc.h"
#include <time.h>
#include <sys/types.h>
#include <stdarg.h>

void error_message_format (char *formatted_message, const char *tag, const char *message, ...)
{
	va_list args;

	time_t now;

	time (&now);

	struct tm *t = localtime (&now);

	SECURE_BUFFER_DECLARATION (char, date, 200);

	snprintf (date, 200, "%d-%02d-%02d_%02d-%02d-%02d"
		, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday
		, t->tm_hour, t->tm_min, t->tm_sec);

	size_t written = 0;
	written = snprintf (formatted_message, BUFSIZ, "%s:%s:", date, tag);

	va_start (args, message);
	written += vsprintf (formatted_message + written, message, args);
	va_end (args);
	snprintf (formatted_message + written, BUFSIZ, "\n");
}

void print_hexa (const char *prefix, uint8_t * payload, size_t size)
{
	if (payload == NULL)
		return;

	SECURE_BUFFER_DECLARATION (char, buffer, BUFSIZ);

	char *cur = buffer, *const end = buffer + sizeof buffer;

	size_t i;
	size_t linenum = 0;
	for (i = 0; i < size; i++) {
		if (i == 0) {
			cur += snprintf (cur, end - cur
				, "\033[32m[%2ld/%2ld]\033[00m \033[36m%s\033[00m (%ld) "
				, linenum + 1, (size / 16) + 1, prefix, size);
			linenum++;
		} else if (!(i % 16)) {
			printf ("%s", buffer);
			memset (buffer, 0, BUFSIZ);
			cur = buffer;
			cur += snprintf (cur, end - cur
				, "\033[32m[%2ld/%2ld]\033[00m \033[36m%s\033[00m (%ld) "
				, linenum + 1, (size / 16) + 1, prefix, size);
			linenum++;
		} else if (!(i % 4)) {
			cur += snprintf (cur, end - cur, "   ");
		}
		cur += snprintf (cur, end - cur, "%2x ", payload[i]);
	}

	if ((i % 16)) {
		printf ("%s", buffer);
	}
}
