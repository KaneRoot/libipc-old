#ifndef __COMMUNICATION2_H__
#define __COMMUNICATION2_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cbor.h>

#include "process.h"
#include <unistd.h> // unlink

#include <fcntl.h> // open

#include <errno.h> // error numbers

#define COMMUNICATION_VERSION 1

#define ER_FILE_OPEN                                1
#define ER_FILE_CLOSE                               2
#define ER_FILE_READ                                3
#define ER_FILE_WRITE                               4
#define ER_FILE_WRITE_PARAMS                        5

#define ER_MEM_ALLOC        100
#define ER_PARAMS           101


struct service {
    unsigned int version;
    unsigned int index;
    char spath[PATH_MAX];
};

int srv_init (int argc, char **argv, char **env
        , struct service *srv, const char *sname
        , int (*cb)(int argc, char **argv, char **env
            , struct service *srv, const char *sname));



#endif