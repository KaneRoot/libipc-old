#include "../../core/communication.h"
#include "../../core/client.h"
#include "../../core/error.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define PONGD_SERVICE_NAME "pongd"

int cpt = 0;

void handle_new_connection (struct ipc_service *srv, struct ipc_client_array *ap)
{
    struct ipc_client *p = malloc(sizeof(struct ipc_client));
    memset(p, 0, sizeof(struct ipc_client));

    if (ipc_server_accept (srv, p) < 0) {
        handle_error("server_accept < 0");
    } else {
        printf("new connection\n");
    }

    if (ipc_client_add (ap, p) < 0) {
        handle_error("ipc_client_add < 0");
    }

    cpt++;
    printf ("%d client(s)\n", cpt);
}

void handle_new_msg (struct ipc_client_array *ap, struct ipc_client_array *proc_to_read)
{
    struct ipc_message m;
    memset (&m, 0, sizeof (struct ipc_message));
    int i;
    for (i = 0; i < proc_to_read->size; i++) {
        // printf ("loop handle_new_msg\n");
        if (ipc_server_read (proc_to_read->clients[i], &m) < 0) {
            handle_error("server_read < 0");
        }

        // close the client then delete it from the client array
        if (m.type == MSG_TYPE_CLOSE) {
            cpt--;
            printf ("disconnection => %d client(s) remaining\n", cpt);

            if (ipc_server_close_client (proc_to_read->clients[i]) < 0)
                handle_err( "handle_new_msg", "server_close_client < 0");
            if (ipc_client_del (ap, proc_to_read->clients[i]) < 0)
                handle_err( "handle_new_msg", "ipc_client_del < 0");
            if (ipc_client_del (proc_to_read, proc_to_read->clients[i]) < 0)
                handle_err( "handle_new_msg", "ipc_client_del < 0");
            i--;
            continue;
        }

        printf ("new message : %s", m.payload);
        if (ipc_server_write (proc_to_read->clients[i], &m) < 0) {
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

void main_loop (struct ipc_service *srv)
{
    int i, ret = 0; 

    struct ipc_client_array ap;
    memset(&ap, 0, sizeof(struct ipc_client_array));

    struct ipc_client_array proc_to_read;
    memset(&proc_to_read, 0, sizeof(struct ipc_client_array));

    while(1) {
        ret = ipc_server_select (&ap, srv, &proc_to_read);
        // printf ("on peut lire ces client:\n");
        // ipc_client_array_print (&proc_to_read);
        // printf ("-- \n\n");

        if (ret == CONNECTION) {
            handle_new_connection (srv, &ap);
        } else if (ret == APPLICATION) {
            handle_new_msg (&ap, &proc_to_read);
        } else { // both new connection and new msg from at least one client
            handle_new_connection (srv, &ap);
            handle_new_msg (&ap, &proc_to_read);
        }
        ipc_client_array_free (&proc_to_read);
    }

    for (i = 0; i < ap.size; i++) {
        if (ipc_server_close_client (ap.clients[i]) < 0) {
            handle_error( "server_close_client < 0");
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
    struct ipc_service srv;
    memset (&srv, 0, sizeof (struct ipc_service));
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
