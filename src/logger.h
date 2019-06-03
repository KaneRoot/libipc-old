#ifndef __IPC_LOGGER_H__
#define __IPC_LOGGER_H__

#include <stdarg.h>

// print log, format: date:tag: message
void log_format (const char* tag, const char* message, va_list args);

#endif
