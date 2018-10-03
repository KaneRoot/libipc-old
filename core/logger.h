#ifndef __IPC_LOGGER_H__
#define __IPC_LOGGER_H__

#define LOG

#include <stdarg.h>

void log_error (const char* message, ...);
void log_info (const char* message, ...);
void log_debug (const char* message, ...);

// please use previous functions
void log_format (const char* tag, const char* message, va_list args);

#endif
