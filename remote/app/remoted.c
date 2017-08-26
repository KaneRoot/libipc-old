#include "../../core/communication.h"
#include "../../core/process.h"
#include "../../core/error.h"
#include "../lib/remoted.h"
#include <stdlib.h>

#include "../../core/logger.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>

// to quit them properly if a signal occurs
struct service srv;

void handle_signal (int signalnumber)
{
    // the application will shut down, and remove the service unix socket
    if (srv_close (&srv) < 0) {
        handle_error("srv_close < 0");
    }

    log_info ("remoted received a signal %d\n", signalnumber);
    exit (EXIT_SUCCESS);
}

/* TODO: handle command line arguments */
void remoted_cmd_args (int argc, char **argv, char **env
        , struct remoted_ctx *ctx)
{
    (void) argc;
    (void) argv;
    (void) env;
    (void) ctx;
}

/* TODO: handle authorizations */
int remoted_auth_conf (struct remoted_ctx *ctx)
{
    (void) ctx;
    return 0;
}


int
main(int argc, char **argv, char **env)
{
    struct remoted_ctx ctx;
    memset (&ctx, 0, sizeof (struct remoted_ctx));

    memset (&srv, 0, sizeof (struct service));
    srv.index = 0;
    srv.version = 0;

    signal(SIGHUP, handle_signal);
    signal(SIGINT, handle_signal);
    signal(SIGQUIT, handle_signal);

    remoted_cmd_args (argc, argv, env, &ctx);

    log_info ("remoted started");
    // load configuration
    if (remoted_auth_conf (&ctx)) {
        log_error ("remoted cannot load configuration");
    }
    else
        log_info ("remoted configuration loaded");

    if (srv_init (argc, argv, env, &srv, REMOTED_SERVICE_NAME) < 0) {
        handle_error("srv_init < 0");
    }
    log_info ("remoted listening on %s", srv.spath);

    // TODO: here comes pledge (openbsd)

    // the service will loop until the end of time, a specific message, a signal
    remoted_main_loop (&srv);

    // the application will shut down, and remove the service unix socket
    if (srv_close (&srv) < 0) {
        handle_error("srv_close < 0");
    }
    log_info ("remoted ended");

    return EXIT_SUCCESS;
}
