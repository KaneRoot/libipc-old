#include "msg.h"
#include "error.h"
#include "usocket.h"

#include <assert.h>

int msg_format_read (struct msg *m, const char *buf, size_t msize)
{
    assert (m != NULL);
    assert (buf != NULL);
    assert (msize >= 3);

    assert (msize <= BUFSIZ - 3);

    if (m == NULL)
        return -1;

    m->type = buf[0];
    memcpy (&m->valsize, buf+1, 2);

    assert (m->valsize <= BUFSIZ -3);

    if (m->val != NULL)
        free (m->val), m->val = NULL;

    if (m->val == NULL) {
        m->val = malloc (m->valsize);
        memcpy (m->val, buf+3, m->valsize);
    }

    return 0;
}

int msg_format_write (struct msg *m, char **buf, size_t *msize)
{
    assert (m != NULL);
    assert (buf != NULL);
    assert (msize != NULL);
    assert (m->valsize <= BUFSIZ -3);

    if (m == NULL)
        return -1;

    if (buf == NULL)
        return -2;

    if (msize == NULL)
        return -3;

    char *buffer = *buf;

    buffer[0] = m->type;
    memcpy (buffer + 1, &m->valsize, 2);
    memcpy (buffer + 3, &m->val, (m->valsize <= BUFSIZ) ? m->valsize : BUFSIZ);

    return 0;
}

int msg_read (int fd, struct msg *m)
{
    assert (m != NULL);

    char *buf = NULL;
    size_t msize = 0;

    int ret = usock_recv (fd, &buf, &msize);
    if (ret < 0) {
        handle_err ("msg_read", "usock_recv");
        return ret;
    }

    if (msg_format_read (m, buf, msize) < 0) {
        return -1;
    }
    free (buf);

    return 0;
}

int msg_write (int fd, const struct msg *m)
{
    assert (m != NULL);

    char *buf = NULL;
    size_t msize = 0;
    msg_format_write (m, &buf, &msize);

    int ret = usock_send (fd, buf, msize);
    if (ret < 0) {
        handle_err ("msg_write", "usock_send");
        return ret;
    }

    if (buf != NULL)
        free (buf);

    return 0;
}

// MSG FORMAT

int msg_format (struct msg *m, char type, const char *val, size_t valsize)
{
    assert (m != NULL);
    assert (valsize + 3 <= BUFSIZ);
    assert (valsize == 0 && val == NULL || valsize > 0 && val != NULL);

    if (valsize + 3 > BUFSIZ) {
        handle_err ("msg_format_con", "msgsize > BUFSIZ");
        return -1;
    }

    m->type = type;
    m->valsize = (short) valsize;
    memcpy (m->val, val, valsize);
    return 0;
}

int msg_format_con (struct msg *m, const char *val, size_t valsize)
{
    return msg_format (m, MSG_TYPE_CON, val, valsize);
}

int msg_format_data (struct msg *m, const char *val, size_t valsize)
{
    return msg_format (m, MSG_TYPE_DATA, val, valsize);
}

int msg_format_ack (struct msg *m, const char *val, size_t valsize)
{
    return msg_format (m, MSG_TYPE_ACK, val, valsize);
}

int msg_free (struct msg *m)
{
    assert (m != NULL);

    if (m == NULL)
        return -1;

    if (m->val != NULL)
        free (m->val);

    return 0;
}
