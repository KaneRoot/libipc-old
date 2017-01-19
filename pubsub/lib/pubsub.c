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
    app_srv_connection (srv, line, strlen (line));
}
#endif

int pubsub_connection (int argc, char **argv, char **env
        , struct service *srv)
{
    int ret = app_connection (argc, argv, env
            , srv, PUBSUBD_SERVICE_NAME, NULL, 0);

    if (ret != 0) {
        handle_err ("pubsub_connection", "app_connection != 0");
    }

    return ret;
}

int pubsub_disconnect (struct service *srv)
{
    return app_close (srv);
}

int pubsub_msg_send (struct service *srv, const struct pubsub_msg * m)
{
    size_t msize = 0;
    char * buf = NULL;
    pubsub_msg_serialize (m, &buf, &msize);

    struct msg m_data;
    memset (&m_data, 0, sizeof (struct msg));

    // format the connection msg
    if (msg_format_data (&m_data, buf, msize) < 0) {
        handle_err ("pubsub_msg_send", "msg_format_data");
        if (buf != NULL)
            free (buf);
        return -1;
    }

    app_write (srv, &m_data);
    msg_free (&m_data);

    if (buf != NULL)
        free(buf);

    return 0;
}

int pubsub_msg_recv (struct service *srv, struct pubsub_msg *m)
{
    if (srv == NULL) {
        handle_err ("pubsub_msg_recv", "srv == NULL");
        return -1;
    }

    if (m == NULL) {
        handle_err ("pubsub_msg_recv", "m == NULL");
        return -1;
    }

    struct msg m_recv;
    memset (&m_recv, 0, sizeof (struct msg));

    app_read (srv, &m_recv);
    pubsub_msg_unserialize (m, m_recv.val, m_recv.valsize);

    msg_free (&m_recv);

    return 0;
}
