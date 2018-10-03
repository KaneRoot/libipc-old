#ifndef __IPC_UTIL_H__
#define __IPC_UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_hexa (const char *prefix, unsigned char *val, size_t size);
void mprint_hexa (char *prefix, unsigned char *buf, size_t length);

#endif
