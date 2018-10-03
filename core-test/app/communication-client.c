#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../core/msg.h"
#include "../../core/error.h"
#include "../../core/communication.h"

#define MSG "coucou"
#define SERVICE_NAME "test"

int main (int argc, char *argv[], char *env[])
{

    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
    struct ipc_service srv;
    memset (&srv, 0, sizeof (struct ipc_service));

    // index and version should be filled
    srv.index = 0;
    srv.version = 0;

    // init service
    if (application_connection (argc, argv, env, &srv, SERVICE_NAME, NULL, 0) < 0) {
        handle_err("main", "server_init < 0");
        return EXIT_FAILURE;
    }

    printf ("msg to send: %s\n", MSG);
    ipc_message_format_data (&m, MSG, strlen(MSG) +1);
    printf ("msg to send in the client: ");
    print_msg (&m);
    if (application_write (&srv, &m) < 0) {
        handle_err("main", "application_write < 0");
        return EXIT_FAILURE;
    }
    ipc_message_free (&m);

    if (application_read (&srv, &m) < 0) {
        handle_err("main", "application_read < 0");
        return EXIT_FAILURE;
    }

    printf ("msg recv: %s\n", m.val);
    ipc_message_free (&m);

    if (application_close (&srv) < 0) {
        handle_err("main", "application_close < 0");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
