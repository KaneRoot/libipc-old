#include "communication2.h"
#include <stdio.h>
#include <time.h>
#include <errno.h>

int srv_init (int argc, char **argv, char **env, struct service *srv, const char *sname, int (*cb)(int argc, char **argv, char **env, struct service *srv, const char *sname))
{
    if (srv == NULL)
        return ER_PARAMS;

    // TODO
    //      use the argc, argv and env parameters
    //      it will be useful to change some parameters transparently
    //      ex: to get resources from other machines, choosing the
    //          remote with environment variables

    argc = argc;
    argv = argv;
    env = env;

    // gets the service path, such as /tmp/<service>
    memset (srv->spath, 0, PATH_MAX);
    strncat (srv->spath, TMPDIR, PATH_MAX -1);
    strncat (srv->spath, sname, PATH_MAX -1);

    srv->version = COMMUNICATION_VERSION;
    srv->index = 0; // TODO

    if (cb != NULL) {
        int ret = (*cb) (argc, argv, env, srv, sname);
        if (ret != 0)
            return ret;
    }

    return 0;
}