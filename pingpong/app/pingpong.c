#include "../lib/communication.h"
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>

#define PONGD_SERVICE_NAME "pongd"
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


/* control the file descriptor*/
void * pongd_thread(void * pdata) {
    //struct process *proc = (struct process*) pdata;
    int *sockclient = (int*) pdata;

    // about the message
    char *buf = malloc(BUFSIZ);
    if (buf == NULL)
    {
        handle_error("malloc");
    }
    memset(buf, 0, BUFSIZ);
    int nbytes;

    //init unix socket
    /*int sfd, cfd;
    struct sockaddr_un peer_addr;
    socklen_t peer_addr_size;
    printf("%s\n", proc->path_proc);
    //app_create(proc, proc->pid, proc->index, proc->version);

    sfd = set_listen_socket(proc->path_proc);
    if (sfd == -1){
        handle_error("set_listen_socket");
    }
    peer_addr_size = sizeof(struct sockaddr_un);

    cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
    if (cfd == -1)
        handle_error("accept");
    proc->proc_fd = cfd;*/

    while (1) {
        if ((nbytes = file_read (*sockclient, &buf)) == -1) {
            fprintf(stdout, "MAIN_LOOP: error service_read %d\n", nbytes);
        }
        
        if (nbytes == 0 || strncmp ("exit", buf, 4) == 0){
            printf("------thread shutdown------------\n");
            //close(cfd);
            close(*sockclient);
            free(buf);
            break;
        }else {
            printf ("read, size %d : %s\n", nbytes, buf);
            if ((nbytes = file_write (*sockclient, buf, nbytes)) == -1) {
                fprintf(stdout, "MAIN_LOOP: error service_write %d\n", nbytes);
            }
        }
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

void main_loop (struct service *srv)
{
    int ret;
    struct process tab_proc[10];
    //thread 
    pthread_t tab_thread[10]; 
    int cnt = 0;

    //init socket unix for server
    int sfd;
    struct sockaddr_un peer_addr;
    socklen_t peer_addr_size;

    sfd = set_listen_socket(srv->spath);
    if (sfd == -1){
        handle_error("set_listen_socket");
    }

    /* master file descriptor list */
    fd_set master;
    /* temp file descriptor list for select() */
    fd_set read_fds;

    /* maximum file descriptor number */
    int fdmax;
    /* listening socket descriptor */
    int listener = sfd;
    /* newly accept()ed socket descriptor */
    int newfd;
    /* buffer for client data */
    char *buf = malloc(BUFSIZ);
    if (buf == NULL)
    {
        handle_error("malloc");
    }
    memset(buf, 0, BUFSIZ);

    int nbytes;

    int i;

    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    /* add the listener to the master set */
    FD_SET(listener, &master);
    //FD_SET(sfd, &master);

    /* keep track of the biggest file descriptor */
    fdmax = sfd; /* so far, it's this one*/

    for(;;) {
        /* copy it */
        read_fds = master;
        if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("Server-select() error lol!");
            exit(1);
        }
        //printf("Server-select...OK\n");

        /*run through the existing connections looking for data to be read*/
        for(i = 0; i <= fdmax; i++) {
            if(FD_ISSET(i, &read_fds)) {
                /* we got one... */
                if(i == listener) {
                    /* handle new connections */
                    peer_addr_size = sizeof(struct sockaddr_un);
                    newfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
                    if (newfd == -1) {
                        handle_error("accept");
                    }
                    else
                    {
                        printf("Server-accept() is OK...\n");
                        //FD_SET(newfd, &master); /* add to master set */
                        //if(newfd > fdmax)
                        //{ /* keep track of the maximum */
                            //fdmax = newfd;
                        //}
			nbytes = file_read (newfd, &buf);
                        if ( nbytes == -1) {
			    handle_error("file_read");
			} else {
			    buf[BUFSIZ - 1] = '\0';
			    printf ("msg received (%d) : %s\n", nbytes, buf);
			    if (strncmp ("exit", buf, 4) == 0) {
				break;
			    }

			    // -1 : error, 0 = no new process, 1 = new process
			    ret = srv_get_new_process (buf, &tab_proc[cnt]);

			    if (ret == -1) {
				fprintf (stderr, "MAIN_LOOP: error service_get_new_process\n");
				continue;
			    } 

			    srv_process_print (&tab_proc[cnt]);

			    int ret = pthread_create( &tab_thread[cnt], NULL, &pongd_thread, (void *) &newfd);
			    if (ret) {
				perror("pthread_create()");
				exit(errno);
			    } else {
				printf ("\n-------New thread created---------\n");
			    }

			    printf ("%d applications to serve\n",cnt);
			    cnt++;
			}
		    } 
                /*else {
                    nbytes = file_read (i, &buf);
                    if ( nbytes == -1) {
                        handle_error("file_read");
                    } else if( nbytes == 0) {*/
                        /* close it... */
                        //close(i);
                        /* remove from master set */
                        /*FD_CLR(i, &master);
                    }else {
                        buf[BUFSIZ - 1] = '\0';
                        printf ("msg received (%d) : %s\n", nbytes, buf);
                        if (strncmp ("exit", buf, 4) == 0) {
                            break;
                        }

                        // -1 : error, 0 = no new process, 1 = new process
                        ret = srv_get_new_process (buf, &tab_proc[cnt]);

                        if (ret == -1) {
                            fprintf (stderr, "MAIN_LOOP: error service_get_new_process\n");
                            continue;
                        } 

                        srv_process_print (&tab_proc[cnt]);

                        int ret = pthread_create( &tab_thread[cnt], NULL, &pongd_thread, (void *) &i);
                        if (ret) {
                            perror("pthread_create()");
                            exit(errno);
                        } else {
                            printf ("\n-------New thread created---------\n");
                        }

                        printf ("%d applications to serve\n",cnt);
                        cnt++;
                    }*/
                }

            }
        }
        if (strncmp ("exit", buf, 4) == 0) {
            break;
        }

    }
    
    for (i = 0; i < cnt; i++) {
        pthread_join(tab_thread[i], NULL);    
    }
    free(buf);
    close(sfd);
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
