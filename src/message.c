#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

#include "message.h"
#include "usocket.h"

#include <assert.h>

void ipc_message_print (const struct ipc_message *m)
{
    assert (m != NULL);
#if defined(IPC_WITH_ERRORS) && IPC_WITH_ERRORS > 2
    printf ("msg: type %d len %d\n", m->type, m->length);
#endif
}


enum ipc_errors ipc_message_new (struct ipc_message **m, ssize_t paylen)
{
	m = malloc (sizeof(struct ipc_message));
	if (m == NULL) {
		return IPC_ERROR_MESSAGE_NEW__NO_MESSAGE_PARAM;
	}

	memset (*m, 0, sizeof (struct ipc_message));

	if (paylen != 0) {
		((struct ipc_message *)m)->payload = malloc (paylen);
		if (((struct ipc_message *)m)->payload == NULL) {
			free (*m);
			return IPC_ERROR_NOT_ENOUGH_MEMORY;
		}
	}

	return IPC_ERROR_NONE;
}

enum ipc_errors ipc_message_format_read (struct ipc_message *m, const char *buf, ssize_t msize)
{
    assert (m != NULL);
    assert (buf != NULL);
    assert (msize <= IPC_MAX_MESSAGE_SIZE);

    if (m == NULL) {
        return IPC_ERROR_MESSAGE_FORMAT_READ__EMPTY_MESSAGE;
	}

    if (buf == NULL) {
        return IPC_ERROR_MESSAGE_FORMAT_READ__EMPTY_BUFFER;
	}

    if (msize > IPC_MAX_MESSAGE_SIZE) {
        return IPC_ERROR_MESSAGE_FORMAT_READ__MESSAGE_SIZE;
	}

    m->type = buf[0];
    memcpy (&m->length, buf+1, sizeof m->length);

    assert (m->length <= IPC_MAX_MESSAGE_SIZE);
#if defined(IPC_WITH_ERRORS) && IPC_WITH_ERRORS > 2
    printf ("type %d, paylen = %u, total = %lu\n", m->type, m->length, msize);
#endif
    assert (m->length == msize - IPC_HEADER_SIZE || m->length == 0);

    if (m->payload != NULL) {
        free (m->payload);
		m->payload = NULL;
	}

    if (m->length > 0) {
        m->payload = malloc (m->length);
        memcpy (m->payload, buf+IPC_HEADER_SIZE, m->length);
    }

    return IPC_ERROR_NONE;
}

enum ipc_errors ipc_message_format_write (const struct ipc_message *m, char **buf, ssize_t *msize)
{
    assert (m != NULL);
    assert (buf != NULL);
    assert (msize != NULL);
    assert (m->length <= IPC_MAX_MESSAGE_SIZE);

    if (m == NULL) {
        return IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_MESSAGE;
	}

    if (buf == NULL) {
        return IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_BUFFER;
	}

    if (msize == NULL) {
        return IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_MSIZE;
	}

    if (*buf == NULL) {
        *buf = malloc (IPC_HEADER_SIZE + m->length);
		memset (*buf, 0, IPC_HEADER_SIZE + m->length);
    }

    char *buffer = *buf;

    buffer[0] = m->type;
    memcpy (buffer + 1, &m->length, sizeof m->length);
	if (m->payload != NULL) {
		memcpy (buffer + IPC_HEADER_SIZE, m->payload, m->length);
	}

    *msize = IPC_HEADER_SIZE + m->length;

#if defined(IPC_WITH_ERRORS) && IPC_WITH_ERRORS > 2
	printf ("sending msg: type %u, size %d, msize %ld\n", m->type, m->length, *msize);
#endif

    return IPC_ERROR_NONE;
}

// IPC_ERROR_CLOSED_RECIPIENT on closed recipient
enum ipc_errors ipc_message_read (int32_t fd, struct ipc_message *m)
{
    assert (m != NULL);

	if (m == NULL) {
		return IPC_ERROR_MESSAGE_READ__NO_MESSAGE_PARAM;
	}

    char *buf = NULL;
    ssize_t msize = IPC_MAX_MESSAGE_SIZE;

    enum ipc_errors ret = usock_recv (fd, &buf, &msize);
    if (ret != IPC_ERROR_NONE && ret != IPC_ERROR_CLOSED_RECIPIENT) {
		if (buf != NULL)
			free (buf);
        handle_err ("msg_read", "usock_recv");
        return ret;
    }

	// closed recipient, buffer already freed
	if (ret == IPC_ERROR_CLOSED_RECIPIENT) {
		if (buf != NULL)
			free (buf);
		return IPC_ERROR_CLOSED_RECIPIENT;
	}

	ret = ipc_message_format_read (m, buf, msize);
    free (buf);
	return ret; // propagates ipc_message_format return
}

enum ipc_errors ipc_message_write (int32_t fd, const struct ipc_message *m)
{
    assert (m != NULL);

	if (m == NULL) {
		return IPC_ERROR_MESSAGE_WRITE__NO_MESSAGE_PARAM;
	}

    char *buf = NULL;
    ssize_t msize = 0;
    ipc_message_format_write (m, &buf, &msize);

	ssize_t nbytes_sent = 0;
    enum ipc_errors ret = usock_send (fd, buf, msize, &nbytes_sent);
	if (buf != NULL) {
		free (buf);
	}

    if (ret != IPC_ERROR_NONE) {
        handle_err ("msg_write", "usock_send");
        return ret;
    }

	// what was sent != what should have been sent
	if (nbytes_sent != msize) {
        handle_err ("msg_write", "usock_send did not send enough data");
        return IPC_ERROR_MESSAGE_WRITE__NOT_ENOUGH_DATA;
	}

    return IPC_ERROR_NONE;
}

// MSG FORMAT

enum ipc_errors ipc_message_format (struct ipc_message *m, char type, const char *payload, ssize_t length)
{
    assert (m != NULL);
    assert (length <= IPC_MAX_MESSAGE_SIZE);
    assert ((length == 0 && payload == NULL) || (length > 0 && payload != NULL));

	if (m == NULL) {
		return IPC_ERROR_MESSAGE_FORMAT__NO_MESSAGE_PARAM;
	}

	if ((length == 0 && payload != NULL) || (length > 0 && payload == NULL)) {
		return IPC_ERROR_MESSAGE_FORMAT__INCONSISTENT_PARAMS;
	}

    if (length > IPC_MAX_MESSAGE_SIZE) {
        handle_err ("msg_format_con", "msgsize > BUFSIZ");
		printf ("msg to format: %ld B)\n", length);
        return IPC_ERROR_MESSAGE_FORMAT__LENGTH;
    }

    m->type = type;
    m->length = (uint32_t) length;

    if (payload != NULL) {
		if (m->payload != NULL) {
			free (m->payload);
		}

        m->payload = malloc (length);
		if (m->payload == NULL) {
			return IPC_ERROR_NOT_ENOUGH_MEMORY;
		}
		memset (m->payload, 0, length);
	}

	if (payload != NULL) {
		memcpy (m->payload, payload, length);
	}
    return IPC_ERROR_NONE;
}

enum ipc_errors ipc_message_format_data (struct ipc_message *m, const char *payload, ssize_t length)
{
    return ipc_message_format (m, MSG_TYPE_DATA, payload, length);
}

enum ipc_errors ipc_message_format_server_close (struct ipc_message *m)
{
    return ipc_message_format (m, MSG_TYPE_SERVER_CLOSE, NULL, 0);
}

enum ipc_errors ipc_message_empty (struct ipc_message *m)
{
    assert (m != NULL);

    if (m == NULL) {
        return IPC_ERROR_MESSAGE_EMPTY__EMPTY_MESSAGE_LIST;
	}

    if (m->payload != NULL) {
        free (m->payload);
		m->payload = NULL;
	}

    m->length = 0;

    return IPC_ERROR_NONE;
}
