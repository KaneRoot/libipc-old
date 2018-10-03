#include <stdlib.h>
#include <string.h> // strndup

#include "pubsub.h"
#include "pubsubd.h"
#include "../../core/error.h"

#define PUBSUB_SUBSCRIBER_ACTION_STR_PUB    "pub"
#define PUBSUB_SUBSCRIBER_ACTION_STR_SUB    "sub"
#define PUBSUB_SUBSCRIBER_ACTION_STR_BOTH   "both"
#define PUBSUB_SUBSCRIBER_ACTION_STR_QUIT   "quit"

char * pubsub_action_to_str (enum subscriber_action action)
{
    switch (action) {
        case PUBSUB_PUB : return strdup (PUBSUB_SUBSCRIBER_ACTION_STR_PUB);
        case PUBSUB_SUB : return strdup (PUBSUB_SUBSCRIBER_ACTION_STR_SUB);
        case PUBSUB_BOTH : return strdup (PUBSUB_SUBSCRIBER_ACTION_STR_BOTH);
        case PUBSUB_QUIT : return strdup (PUBSUB_SUBSCRIBER_ACTION_STR_QUIT);
    }
    return NULL;
}


#if 0
// tell the service to stop
void pubsub_quit (struct service *srv)
{
    // line fmt : 0 0 0 quit
    char line[BUFSIZ];
    snprintf (line, BUFSIZ, "0 0 0 quit\n");
    ipc_application_server_connection (srv, line, strlen (line));
}
#endif

int pubsub_connection (int argc, char **argv, char **env
        , struct service *srv)
{
    int ret = ipc_application_connection (argc, argv, env
            , srv, PUBSUBD_SERVICE_NAME, NULL, 0);

    if (ret != 0) {
        handle_err ("pubsub_connection", "application_connection != 0");
    }

    return ret;
}

int pubsub_disconnect (struct service *srv)
{
    return ipc_application_close (srv);
}

int pubsub_message_send (struct service *srv, const struct pubsub_msg * m)
{
    size_t msize = 0;
    char * buf = NULL;
    pubsub_message_serialize (m, &buf, &msize);

    struct ipc_message m_data;
    memset (&m_data, 0, sizeof (struct ipc_message));

    // format the connection msg
    if (ipc_message_format_data (&m_data, buf, msize) < 0) {
        handle_err ("pubsub_message_send", "msg_format_data");
        if (buf != NULL)
            free (buf);
        return -1;
    }

    ipc_application_write (srv, &m_data);
    ipc_message_free (&m_data);

    if (buf != NULL)
        free(buf);

    return 0;
}

int pubsub_message_recv (struct service *srv, struct pubsub_msg *m)
{
    if (srv == NULL) {
        handle_err ("pubsub_message_recv", "srv == NULL");
        return -1;
    }

    if (m == NULL) {
        handle_err ("pubsub_message_recv", "m == NULL");
        return -1;
    }

    struct ipc_message m_recv;
    memset (&m_recv, 0, sizeof (struct ipc_message));

    ipc_application_read (srv, &m_recv);
    pubsub_message_unserialize (m, m_recv.val, m_recv.valsize);

    ipc_message_free (&m_recv);

    return 0;
}
