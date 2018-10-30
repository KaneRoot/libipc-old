#include <stdio.h>
#include <stdlib.h>
#include <string.h> // strndup, strncpy

#include "message.h"

void pubsub_message_set_data (struct pubsub_msg *pm, char *data, size_t len)
{
    pm->datalen = len;
	if (pm->data != NULL) {
		free (pm->data);
	}
    pm->data = malloc (len + 1);
    memset (pm->data, 0, len);
    strncpy (pm->data, data, len);
    pm->data[len] = '\0';
}

void pubsub_message_set_chan (struct pubsub_msg *pm, char *chan, size_t len)
{
    pm->chanlen = len;
	if (pm->chan != NULL) {
		free (pm->chan);
	}
    pm->chan = malloc (len + 1);
    memset (pm->chan, 0, len);
    strncpy (pm->chan, chan, len);
    pm->chan[len] = '\0';
}

void pubsub_message_from_message (struct pubsub_msg *pm, struct ipc_message *m)
{
    size_t offset = 0;

	pubsub_message_empty (pm); // just in case

    pm->type = m->type;

    // chan
    memcpy (&pm->chanlen, m->payload + offset, sizeof (size_t));
    if (pm->chanlen > BUFSIZ) {
        handle_err ("pubsub_message_from_message", "chanlen > BUFSIZ");
        return;
    }
	offset += sizeof (size_t);
	if (pm->chanlen > 0) {
		pubsub_message_set_chan (pm, m->payload + offset, pm->chanlen);
		offset += pm->chanlen;
	}

	// data
	memcpy (&pm->datalen, m->payload + offset, sizeof (size_t));
	if (pm->datalen > BUFSIZ) {
		handle_err ("pubsub_message_from_message", "chanlen > BUFSIZ");
		return;
	}
	offset += sizeof (size_t);
	if (pm->datalen > 0) {
		pubsub_message_set_data (pm, m->payload + offset, pm->datalen);
		offset += pm->datalen;
	}
}

void pubsub_message_to_message (const struct pubsub_msg *msg, struct ipc_message *m)
{
    if (msg == NULL) {
        handle_err ("pubsub_message_to_message", "msg == NULL");
        return;
    }

	if (m == NULL) {
		handle_err ("pubsub_message_to_message", "data == NULL");
		return;
	}

	ipc_message_empty (m); // just in case

    size_t buflen = 2 * sizeof (size_t) + msg->chanlen + msg->datalen;

    if (buflen > BUFSIZ) {
        handle_err ("pubsub_message_serialize", "chanlen + datalen too high");
        return;
    }

    char *buf = malloc (buflen);
    memset (buf, 0, buflen);

    size_t offset = 0;

	m->type = msg->type;

    // chan
    memcpy (buf + offset, &msg->chanlen, sizeof (size_t));
    offset += sizeof (size_t);
    memcpy (buf + offset, msg->chan, msg->chanlen);
    offset += msg->chanlen;

    // data
    memcpy (buf + offset, &msg->datalen, sizeof (size_t));
    offset += sizeof (size_t);
    memcpy (buf + offset, msg->data, msg->datalen);
    offset += msg->datalen;

    m->payload = buf;
    m->length  = buflen;
}

void pubsub_message_empty (struct pubsub_msg *msg)
{
    if (msg == NULL) {
        handle_err ("pubsub_message_empty", "msg == NULL");
        return;
    }

    if (msg->chan != NULL) {
        free (msg->chan);
        msg->chan = NULL;
    }

    if (msg->data != NULL) {
        free (msg->data);
        msg->data = NULL;
    }
}

void pubsub_message_print (const struct pubsub_msg *pm)
{
    if (pm == NULL) {
        handle_err ("pubsub_message_print", "pm == NULL");
        return;
    }

	if (pm->chanlen > 0 && pm->datalen > 0) {
		printf ("msg: type: %u, chan: %s (%lu bytes), data: %s (%lu bytes)\n"
				, pm->type, pm->chan, pm->chanlen, pm->data, pm->datalen);
	} else if (pm->chanlen > 0) {
		printf ("msg: type: %u, chan: %s (%lu bytes), and no data\n"
				, pm->type, pm->chan, pm->chanlen);
	} else if (pm->datalen > 0) {
		printf ("msg: type: %u, no chan, data: %s (%lu bytes)\n"
				, pm->type, pm->data, pm->datalen);
	}
}

int pubsub_message_send (struct ipc_service *srv, const struct pubsub_msg * pm)
{
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
	pubsub_message_to_message (pm, &m);
    ipc_application_write (srv, &m);
    ipc_message_empty (&m);

    return 0;
}

char * pubsub_action_to_str (enum subscriber_action action)
{
    switch (action) {
        case PUBSUB_PUB : return strdup (PUBSUB_SUBSCRIBER_ACTION_STR_PUB);
        case PUBSUB_SUB : return strdup (PUBSUB_SUBSCRIBER_ACTION_STR_SUB);
        default : return strdup ("undocumented action");
    }
    return NULL;
}
