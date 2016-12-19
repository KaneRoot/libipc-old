#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msg-format.h"

#define handle_err(fun,msg)\
    fprintf (stderr, "%s: file %s line %d %s\n", fun, __FILE__, __LINE__, msg);

// [type]   [len]       [val]
// 3        valsize     constr
int msg_format_con (char *buf, const char *constr, size_t *msgsize)
{
    assert (buf != NULL);
    assert (*msgsize + 3 <= BUFSIZ);

    if (*msgsize + 3 > BUFSIZ) {
        handle_err ("msg_format_con", "msgsize > BUFSIZ");
        return 1;
    }

    buf[0] = MSG_TYPE_CON;
    short final_size = (short) *msgsize;
    memcpy (buf + 1, &final_size, 2);
    memcpy (buf + 3, constr, *msgsize);

    return 0;
}

// [type]   [len]
// 4        0
int msg_format_ack (char *buf)
{
    assert (buf != NULL);
    memset(buf, 0, 3);
    buf[0] = MSG_TYPE_ACK;

    return 0;
}
