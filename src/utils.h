#ifndef __IPC_UTIL_H__
#define __IPC_UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

void print_hexa (const char *prefix, uint8_t *payload, size_t size);
void mprint_hexa (char *prefix, uint8_t *buf, size_t length);

#endif
