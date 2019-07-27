#include "utils.h"
#include "ipc.h"

void print_hexa (const char *prefix, uint8_t *payload, size_t size)
{
    if (payload == NULL)
        return ;

	SECURE_BUFFER_DECLARATION (char, buffer, BUFSIZ);

	char *cur = buffer, * const end = buffer + sizeof buffer;

    size_t i;
	size_t linenum = 0;
    for(i = 0; i < size; i++) {
		if (i == 0) {
			cur += snprintf(cur, end-cur, "\033[32m[%2ld/%2ld]\033[00m \033[36m%s\033[00m (%ld) ", linenum +1, (size / 16) +1, prefix, size);
			linenum ++;
		}
		else if(! (i % 16)) {
			LOG_DEBUG ("%s", buffer);
			memset (buffer, 0, BUFSIZ);
			cur = buffer;
			cur += snprintf(cur, end-cur, "\033[32m[%2ld/%2ld]\033[00m \033[36m%s\033[00m (%ld) ", linenum +1, (size / 16) +1, prefix, size);
			linenum ++;
		}
		else if (! (i % 4)) {
			cur += snprintf(cur, end-cur, "   ");
		}
		cur += snprintf(cur, end-cur, "%2x ", payload[i]);
    }

	if ((i % 16)) {
		LOG_DEBUG ("%s", buffer);
	}
}
