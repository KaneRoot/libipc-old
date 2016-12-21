#ifndef __ERROR_H__
#define __ERROR_H__

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_err(fun,msg)\
    fprintf (stderr, "%s: file %s line %d %s\n", fun, __FILE__, __LINE__, msg);

#endif
