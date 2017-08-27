#include "../../core/communication.h"
#include "../../core/error.h"
#include "msg.h"
#include "remotec.h"
#include "remoted.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int remotec_connection (int argc, char **argv, char **env, struct service *srv)
{
    int ret = app_connection (argc, argv, env
            , srv, REMOTED_SERVICE_NAME, NULL, 0);

    if (ret != 0) {
        handle_err ("remote remotec_connection", "app_connection != 0");
    }

    return ret;
}

int remotec_disconnection (struct service *srv)
{
    return app_close (srv);
}

int remotec_msg_send (struct service *srv, const struct remoted_msg * m)
{
    size_t msize = 0;
    char * buf = NULL;
    remote_msg_serialize (m, &buf, &msize);

    struct msg m_data;
    memset (&m_data, 0, sizeof (struct msg));

    // format the connection msg
    if (msg_format_data (&m_data, buf, msize) < 0) {
        handle_err ("remotec_msg_send", "msg_format_data");
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

int remotec_msg_recv (struct service *srv, struct remoted_msg *m)
{
    if (srv == NULL) {
        handle_err ("remotec_msg_recv", "srv == NULL");
        return -1;
    }

    if (m == NULL) {
        handle_err ("remotec_msg_recv", "m == NULL");
        return -1;
    }

    struct msg m_recv;
    memset (&m_recv, 0, sizeof (struct msg));

    app_read (srv, &m_recv);
    remote_msg_unserialize (m, m_recv.val, m_recv.valsize);

    msg_free (&m_recv);

    return 0;
}
