#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

#include <arpa/inet.h>

#include "message.h"
#include "usocket.h"

// #define IPC_WITH_ERRORS 3

uint32_t ipc_message_raw_serialize (char *buffer, char type, char user_type, char * message, uint32_t message_size)
{
	uint32_t msize = 6 + message_size;
	buffer[0] = type;

	uint32_t msize_n = htonl (message_size);
	uint32_t index = 1;

	memcpy (buffer + index, &msize_n, sizeof (uint32_t));
	index += sizeof (uint32_t);
	buffer[index] = user_type;
	index += 1;
	memcpy (buffer + index, message, message_size);

	return msize;
}


void ipc_message_print (const struct ipc_message *m)
{
    if (m == NULL) return;

#if defined(IPC_WITH_ERRORS) && IPC_WITH_ERRORS > 2
    LOG_INFO ("msg: type %d len %d\n", m->type, m->length);
#endif
}

enum ipc_errors ipc_message_format_read (struct ipc_message *m, const char *buf, size_t msize)
{
    T_R ((m == NULL), IPC_ERROR_MESSAGE_FORMAT_READ__EMPTY_MESSAGE);
    T_R ((buf == NULL), IPC_ERROR_MESSAGE_FORMAT_READ__EMPTY_BUFFER);
    T_R ((msize > IPC_MAX_MESSAGE_SIZE), IPC_ERROR_MESSAGE_FORMAT_READ__MESSAGE_SIZE);

	// message format:
	//   Type (1 B) | Length (4 B) | UserType (1 B) | Payload (Length B)
    m->type = buf[0];
	uint32_t unformated_size = 0;
    memcpy (&unformated_size, buf+1, sizeof(uint32_t));
	m->length = ntohl (unformated_size);
	m->user_type = buf[1 + sizeof m->length];

    T_R ((m->length > IPC_MAX_MESSAGE_SIZE), IPC_ERROR_MESSAGE_FORMAT_READ__READ_MESSAGE_SIZE);
#if defined(IPC_WITH_ERRORS) && IPC_WITH_ERRORS > 2
    LOG_INFO ("receiving msg: type %d, paylen %u, total %lu", m->type, m->length, msize);
#endif
    T_R ((m->length != msize - IPC_HEADER_SIZE && m->length != 0)
			, IPC_ERROR_MESSAGE_FORMAT_READ__READ_MESSAGE_SIZE);

    if (m->payload != NULL) {
        free (m->payload);
		m->payload = NULL;
	}

    if (m->length > 0) {
        m->payload = malloc (m->length);
        memcpy (m->payload, buf+IPC_HEADER_SIZE, m->length);
    }
	else {
        m->payload = malloc (1);
	}

    return IPC_ERROR_NONE;
}

enum ipc_errors ipc_message_format_write (const struct ipc_message *m, char **buf, size_t *msize)
{
    T_R ((m == NULL), IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_MESSAGE);
    T_R ((buf == NULL), IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_BUFFER);
    T_R ((msize == NULL), IPC_ERROR_MESSAGE_FORMAT_WRITE__EMPTY_MSIZE);
    T_R ((m->length > IPC_MAX_MESSAGE_SIZE), IPC_ERROR_MESSAGE_FORMAT_WRITE__MESSAGE_LENGTH);

    if (*buf == NULL) {
        *buf = malloc (IPC_HEADER_SIZE + m->length);
		memset (*buf, 0, IPC_HEADER_SIZE + m->length);
    }

    char *buffer = *buf;
	uint32_t net_paylen = htonl(m->length);

    buffer[0] = m->type;
    memcpy (buffer + 1, &net_paylen, sizeof(uint32_t));
    buffer[1 + sizeof m->length] = m->user_type;
	if (m->payload != NULL && m->length > 0) {
		memcpy (buffer + IPC_HEADER_SIZE, m->payload, m->length);
	}

    *msize = IPC_HEADER_SIZE + m->length;

#if defined(IPC_WITH_ERRORS) && IPC_WITH_ERRORS > 2
	LOG_INFO ("sending msg: type %u, paylen %u, msize %lu", m->type, m->length, *msize);
#endif

    return IPC_ERROR_NONE;
}

// MSG FORMAT

enum ipc_errors ipc_message_format (struct ipc_message *m
		, char type, char utype, const char *payload, size_t length)
{
	T_R ((m == NULL), IPC_ERROR_MESSAGE_FORMAT__NO_MESSAGE_PARAM);
	T_R ((length > IPC_MAX_MESSAGE_SIZE), IPC_ERROR_MESSAGE_FORMAT__MESSAGE_SIZE);
	T_R (((length == 0 && payload != NULL) || (length > 0 && payload == NULL))
			, IPC_ERROR_MESSAGE_FORMAT__INCONSISTENT_PARAMS);

    m->type = type;
    m->user_type = utype;
    m->length = (uint32_t) length;

    if (payload != NULL) {
		if (m->payload != NULL) {
			free (m->payload);
		}

		SECURE_BUFFER_HEAP_ALLOCATION_R (m->payload, length, , IPC_ERROR_NOT_ENOUGH_MEMORY);
	}

	if (payload != NULL) {
		memcpy (m->payload, payload, length);
	}
    return IPC_ERROR_NONE;
}

enum ipc_errors ipc_message_format_data (struct ipc_message *m, char utype, const char *payload, size_t length)
{
    return ipc_message_format (m, MSG_TYPE_DATA, utype, payload, length);
}

enum ipc_errors ipc_message_format_server_close (struct ipc_message *m)
{
    return ipc_message_format (m, MSG_TYPE_SERVER_CLOSE, 0, NULL, 0);
}

enum ipc_errors ipc_message_empty (struct ipc_message *m)
{
    T_R ((m == NULL), IPC_ERROR_MESSAGE_EMPTY__EMPTY_MESSAGE_LIST);

    if (m->payload != NULL) {
        free (m->payload);
		m->payload = NULL;
	}

    m->length = 0;

    return IPC_ERROR_NONE;
}
