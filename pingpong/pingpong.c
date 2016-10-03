#include "../lib/communication.h"
#include <pthread.h>

#define PONGD_SERVICE_NAME "pongd"


/* control the file descriptor*/
void * pongd_thread(void * pdata) {
    struct process *proc = (struct process*) pdata;

    // about the message
    size_t msize = 0;
    char *buf = NULL;
    int ret = 0;

    while (1) {
        // printf ("before read\n");
        while (ret <= 0 || msize == 0) {
            if ((ret = srv_read (proc, &buf, &msize)) == -1) {
                fprintf(stdout, "MAIN_LOOP: error service_read %d\n", ret);
            }
        }
        // printf ("after read\n");
        printf ("read, size %ld : %s\n", msize, buf);

        if (strncmp ("exit", buf, 4) != 0) {
            //printf ("before proc write\n");
            if ((ret = srv_write (proc, buf, msize)) == -1) {
                fprintf(stdout, "MAIN_LOOP: error service_write %d\n", ret);
            }
        }else {
            printf("------thread shutdown------------\n");
            free(buf);
            break;
        }
        ret = 0;
        msize = 0;
    }

    return NULL;
}

/*
 * main loop
 *
 * opens the application pipes,
 * reads then writes the same message,
 * then closes the pipes
 */

void main_loop (const struct service *srv)
{
    int ret;
    struct process tab_proc[10];
    //thread 
    pthread_t tab_thread[10]; 
    int i;

    int cnt = 0;

    while (cnt < 10) {

        // -1 : error, 0 = no new process, 1 = new process
        ret = srv_get_new_process (srv, &tab_proc[cnt]);

        if (ret == -1) {
            fprintf (stderr, "MAIN_LOOP: error service_get_new_process\n");
            continue;
        } else if (ret == 0) { // that should not happen
            continue;
        }

        srv_process_print (&tab_proc[cnt]);
        
        printf ("\n-------New thread created---------\n");
        int ret = pthread_create( &tab_thread[cnt], NULL, &pongd_thread, (void *) &tab_proc[cnt]);
        if (ret) {
            perror("pthread_create()");
            exit(errno);
        } else {
            printf("Creation of listen thread \n");
        }

        // printf ("after proc write\n");
        printf ("%d applications to serve\n",cnt);

        cnt++;
    }

    for (i = 0; i < cnt; i++) {
        pthread_join(tab_thread[cnt], NULL);    
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
    srv_init (argc, argv, env, &srv, PONGD_SERVICE_NAME, NULL);
    printf ("Listening on %s.\n", srv.spath);

    // creates the service named pipe, that listens to client applications
    int ret;
    if ((ret = srv_create (&srv))) {
        fprintf(stdout, "error service_create %d\n", ret);
        exit (1);
    }
    printf("MAIN: server created\n" );

    // the service will loop until the end of time, a specific message, a signal
    main_loop (&srv);

    // the application will shut down, and remove the service named pipe
    if ((ret = srv_close (&srv))) {
        fprintf(stdout, "error service_close %d\n", ret);
        exit (1);
    }

    return EXIT_SUCCESS;
}
