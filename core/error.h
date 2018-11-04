#ifndef __IPC_ERROR_H__
#define __IPC_ERROR_H__

// #define IPC_WITH_ERRORS                               3

#ifdef IPC_WITH_ERRORS
#include "logger.h"
#define handle_error(msg) \
    do { log_error (msg); exit(EXIT_FAILURE); } while (0)

#define handle_err(fun,msg)\
    do { log_error ("%s: file %s line %d %s", fun, __FILE__, __LINE__, msg); } while (0)
#else
#define handle_error(msg)
#define handle_err(fun,msg)
#endif

#endif
