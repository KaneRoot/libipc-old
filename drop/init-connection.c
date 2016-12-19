#include "../lib/communication.h"

#define SERVICE         "windowing"

void
ohshit(int rvalue, const char* str) {
    fprintf(stderr, "%s\n", str);
    exit(rvalue);
}

int main(int argc, char * argv[], char *env[])
{
    struct service srv;
    memset (&srv, 0, sizeof (struct service));
    srv_init (argc, argv, env, &srv, SERVICE, NULL);
    printf ("Listening on %s.\n", srv.spath);

    // creates the service named pipe, that listens to client applications
    if (srv_create (&srv))
        ohshit(1, "service_create error");

    /*
     *  PROCESS
     */

    struct process p;
    memset (&p, 0, sizeof (struct process));

    int index = 0; // first time we communication with the service
    int version = 1;

    printf ("app creation\n");
    if (app_create (&p, index, version)) // called by the application
        ohshit (1, "app_create");

    /*
     * some exchanges between App and S
     * specific code, talks between applications
     * then App wants to end the communication
     */

    printf ("destroying app\n");
    // the application will shut down, and remove the application named pipes
    if (app_destroy (&p))
        ohshit (1, "app_destroy");

    /*
     *  /PROCESS
     */

    // the application will shut down, and remove the service named pipe
    if (srv_close (&srv))
        ohshit (1, "srv_close error");

    return EXIT_SUCCESS;
}