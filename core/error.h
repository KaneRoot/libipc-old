#ifndef __ERROR_H__
#define __ERROR_H__

#include "logger.h"

#define handle_error(msg) \
    do { log_error (msg); exit(EXIT_FAILURE); } while (0)

#define handle_err(fun,msg)\
    do { log_error ("%s: file %s line %d %s", fun, __FILE__, __LINE__, msg); } while (0)

#endif
