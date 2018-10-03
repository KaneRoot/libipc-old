#include "../../core/communication.h"
#include "../../core/process.h"
#include "../../core/error.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define PONGD_SERVICE_NAME "pongd"

int cpt = 0;

void handle_new_connection (struct service *srv, struct ipc_process_array *ap)
{
    struct ipc_process *p = malloc(sizeof(struct ipc_process));
    memset(p, 0, sizeof(struct ipc_process));

    if (ipc_server_accept (srv, p) < 0) {
        handle_error("server_accept < 0");
    } else {
        printf("new connection\n");
    }

    if (ipc_process_add (ap, p) < 0) {
        handle_error("ipc_process_add < 0");
    }

    cpt++;
    printf ("%d client(s)\n", cpt);
}

void handle_new_msg (struct ipc_process_array *ap, struct ipc_process_array *proc_to_read)
{
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
    int i;
    for (i = 0; i < proc_to_read->size; i++) {
        // printf ("loop handle_new_msg\n");
        if (ipc_server_read (proc_to_read->tab_proc[i], &m) < 0) {
            handle_error("server_read < 0");
        }

        // close the process then delete it from the process array
        if (m.type == MSG_TYPE_CLOSE) {
            cpt--;
            printf ("disconnection => %d client(s) remaining\n", cpt);

            if (ipc_server_close_proc (proc_to_read->tab_proc[i]) < 0)
                handle_err( "handle_new_msg", "server_close_proc < 0");
            if (ipc_process_del (ap, proc_to_read->tab_proc[i]) < 0)
                handle_err( "handle_new_msg", "ipc_process_del < 0");
            if (ipc_process_del (proc_to_read, proc_to_read->tab_proc[i]) < 0)
                handle_err( "handle_new_msg", "ipc_process_del < 0");
            i--;
            continue;
        }

        printf ("new message : %s", m.val);
        if (ipc_server_write (proc_to_read->tab_proc[i], &m) < 0) {
            handle_err( "handle_new_msg", "server_write < 0");
        }
    }
}

/*
 * main loop
 *
 * accept new application connections
 * read a message and send it back
 * close a connection if MSG_TYPE_CLOSE received
 */

void main_loop (struct service *srv)
{
    int i, ret = 0; 

    struct ipc_process_array ap;
    memset(&ap, 0, sizeof(struct ipc_process_array));

    struct ipc_process_array proc_to_read;
    memset(&proc_to_read, 0, sizeof(struct ipc_process_array));

    while(1) {
        ret = ipc_server_select (&ap, srv, &proc_to_read);
        // printf ("on peut lire ces process:\n");
        // ipc_process_array_print (&proc_to_read);
        // printf ("-- \n\n");

        if (ret == CONNECTION) {
            handle_new_connection (srv, &ap);
        } else if (ret == APPLICATION) {
            handle_new_msg (&ap, &proc_to_read);
        } else { // both new connection and new msg from at least one client
            handle_new_connection (srv, &ap);
            handle_new_msg (&ap, &proc_to_read);
        }
        ipc_process_array_free (&proc_to_read);
    }

    for (i = 0; i < ap.size; i++) {
        if (ipc_server_close_proc (ap.tab_proc[i]) < 0) {
            handle_error( "server_close_proc < 0");
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

    // unlink("/tmp/ipc/pongd-0-0");

    if (ipc_server_init (argc, argv, env, &srv, PONGD_SERVICE_NAME) < 0) {
        handle_error("server_init < 0");
        return EXIT_FAILURE;
    }
    printf ("Listening on %s.\n", srv.spath);

    printf("MAIN: server created\n" );

    // the service will loop until the end of time, a specific message, a signal
    main_loop (&srv);

    // the application will shut down, and remove the service named pipe
    if (ipc_server_close (&srv) < 0) {
        handle_error("server_close < 0");
    }

    return EXIT_SUCCESS;
}
