#include "../../core/communication.h"
#include <sys/socket.h>
#include <sys/un.h>
#include "../../core/process.h"
#include <unistd.h>

#define PONGD_SERVICE_NAME "pongd"
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)



/*
 * main loop
 *
 * opens the application pipes,
 * reads then writes the same message,
 * then closes the pipes
 */

void main_loop (struct service *srv)
{

    size_t msize = BUFSIZ;
    char *buf = NULL;
    if ( (buf = malloc (BUFSIZ)) == NULL) {
        handle_error ("malloc");
    }
    memset (buf, 0, BUFSIZ);

    int i,ret, cpt = 0; 

    struct array_proc ap;
    memset(&ap, 0, sizeof(struct array_proc));

    struct process *p2 = malloc(sizeof(struct process));

    while(cpt < 5) {
        ret = srv_select(&ap, srv, &p2);
        
        if (ret == CONNECTION) {
            struct process *p = malloc(sizeof(struct process));
            memset(p, 0, sizeof(struct process));
            if (srv_accept (srv, p) < 0) {
                handle_error("srv_accept < 0");
            }else {
                printf("new connection\n");
            }

            if (add_proc(&ap, p) < 0) {
                handle_error("add_proc < 0");
            }
            cpt++;
        } else {
            if (srv_read(p2, &buf, &msize) < 0) {
                handle_error("srv_read < 0");
            }
            
            if (srv_write (p2, buf, msize) < 0) {
                handle_error("srv_write < 0");
            }
        }
    }

    for (i = 0; i < ap.size; i++) {
        if (srv_close_proc (ap.tab_proc[i]) < 0) {
            handle_error( "srv_close_proc < 0");
        }
    }
}


/*
 * service ping-pong
 *
 * 1. creates the named pipe /tmp/<service>, then listens
 * 2. opens the named pipes in & out
 * 3. talks with the (test) program
 * 4. closes the test program named pipes
 * 5. removes the named pipe /tmp/<service>
 */

int main(int argc, char * argv[], char **env)
{
    struct service srv;
    memset (&srv, 0, sizeof (struct service));
    srv.index = 0;
    srv.version = 0;
    unlink("/tmp/ipc/pongd-0-0");
    if (srv_init (argc, argv, env, &srv, PONGD_SERVICE_NAME) < 0) {
        handle_error("srv_init < 0");
        return EXIT_FAILURE;
    }
    printf ("Listening on %s.\n", srv.spath);

    printf("MAIN: server created\n" );

    // the service will loop until the end of time, a specific message, a signal
    main_loop (&srv);

    // the application will shut down, and remove the service named pipe
    if (srv_close (&srv) < 0) {
        handle_error("srv_close < 0");

    }

    return EXIT_SUCCESS;
}
