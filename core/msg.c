#include "msg.h"
#include "error.h"
#include "usocket.h"

#include <assert.h>

void ipc_message_print (const struct ipc_message *m)
{
    assert (m != NULL);
    printf ("msg: type %d len %d\n", m->type, m->length);
}

int ipc_message_format_read (struct ipc_message *m, const char *buf, size_t msize)
{
    assert (m != NULL);
    assert (buf != NULL);
    assert (msize <= BUFSIZ - 3);

    if (m == NULL)
        return -1;

    m->type = buf[0];
    memcpy (&m->length, buf+1, 2);

    assert (m->length <= BUFSIZ - 3);
    printf ("type %d : msize = %ld, length = %d\n", m->type, msize, m->length);
    assert (m->length == msize - 3 || m->length == 0);

    if (m->payload != NULL)
        free (m->payload), m->payload = NULL;

    if (m->payload == NULL && m->length > 0) {
        m->payload = malloc (m->length);
        memcpy (m->payload, buf+3, m->length);
    }

    return 0;
}

int ipc_message_format_write (const struct ipc_message *m, char **buf, size_t *msize)
{
    assert (m != NULL);
    assert (buf != NULL);
    assert (msize != NULL);
    assert (m->length <= BUFSIZ -3);

    if (m == NULL)
        return -1;

    if (buf == NULL)
        return -2;

    if (msize == NULL)
        return -3;

    if (*buf == NULL) {
        *buf = malloc (3 + m->length);
		memset (*buf, 0, 3 + m->length);
    }

    char *buffer = *buf;

    buffer[0] = m->type;
    memcpy (buffer + 1, &m->length, 2);
	if (m->payload != NULL) {
		memcpy (buffer + 3, m->payload, m->length);
	}

    *msize = 3 + m->length;

	printf ("sending msg: type %u, size %d, msize %ld\n", m->type, m->length, *msize);

    return 0;
}

int ipc_message_read (int fd, struct ipc_message *m)
{
    assert (m != NULL);

    char *buf = NULL;
    size_t msize = BUFSIZ;

    int ret = usock_recv (fd, &buf, &msize);
    if (ret < 0) {
        handle_err ("msg_read", "usock_recv");
        return ret;
    }

    if (ipc_message_format_read (m, buf, msize) < 0) {
        return -1;
    }
    free (buf);

    return 0;
}

int ipc_message_write (int fd, const struct ipc_message *m)
{
    assert (m != NULL);

    char *buf = NULL;
    size_t msize = 0;
    ipc_message_format_write (m, &buf, &msize);

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

int ipc_message_format (struct ipc_message *m, char type, const char *payload, size_t length)
{
    assert (m != NULL);
    assert (length + 3 <= BUFSIZ);
    assert ((length == 0 && payload == NULL) || (length > 0 && payload != NULL));

    if (length + 3 > BUFSIZ) {
        handle_err ("msg_format_con", "msgsize > BUFSIZ");
        return -1;
    }

    m->type = type;
    m->length = (short) length;

    if (payload != NULL) {
		if (m->payload != NULL) {
			free (m->payload);
		}
		// FIXME: test malloc
        m->payload = malloc (length);
		memset (m->payload, 0, length);
	}

	if (payload != NULL) {
		memcpy (m->payload, payload, length);
	}
    return 0;
}

int ipc_message_format_con (struct ipc_message *m, const char *payload, size_t length)
{
    return ipc_message_format (m, MSG_TYPE_CON, payload, length);
}

int ipc_message_format_data (struct ipc_message *m, const char *payload, size_t length)
{
    return ipc_message_format (m, MSG_TYPE_DATA, payload, length);
}

int ipc_message_format_ack (struct ipc_message *m, const char *payload, size_t length)
{
    return ipc_message_format (m, MSG_TYPE_ACK, payload, length);
}

int ipc_message_format_server_close (struct ipc_message *m)
{
    return ipc_message_format (m, MSG_TYPE_SERVER_CLOSE, NULL, 0);
}

int ipc_message_free (struct ipc_message *m)
{
    assert (m != NULL);

    if (m == NULL)
        return -1;

    if (m->payload != NULL)
        free (m->payload), m->payload = NULL;

    m->length = 0;

    return 0;
}
