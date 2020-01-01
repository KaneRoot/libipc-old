#ifndef __IPC_UTIL_H__
#define __IPC_UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

void error_message_format (char *formatted_message, const char *tag, const char *message, ...);
void print_hexa (const char *prefix, uint8_t * payload, size_t size);

#endif
