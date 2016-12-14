#include "tcpd.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h> // mkfifo
#include <linux/limits.h>

#define PORT 6000
#define TMPDIR "/tmp/ipc/"
#define NBCLIENT 10
#define SERVICE_TCP "tcpd"
#define LISTEN_BACKLOG 50
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


int init_connection(const info_request *req)
{
    int yes = 1;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    //struct sockaddr_in sin = { 0 };

    if(sock == -1)
    {
        perror("socket()");
        exit(errno);
    }

    /*"address already in use" error message */
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("Server-setsockopt() error lol!");
        exit(1);
    }
    printf("Server-setsockopt() is OK...\n");

    if(bind(sock,(struct sockaddr *) &req->addr, sizeof(req->addr)) == -1)
    {
        perror("bind()");
        exit(errno);
    }

    if(listen(sock, 10) == -1)
    {
        perror("listen()");
        exit(errno);
    }

    return sock;
}

void write_message(int sock, const char *buffer, size_t size_buf)
{
    if(send(sock, buffer, size_buf, 0) < 0)
    {
        perror("send()");
        exit(errno);
    }
}

int read_message(int sock, char *buffer)
{
    return read(sock, buffer, BUFSIZ);
}

void endConnection(int sock) {
    close(sock);
}

void printAddr(struct sockaddr_in *csin) {
    printf("IP Addr : %s\n", inet_ntoa(csin->sin_addr));
    printf("Port : %u\n", ntohs(csin->sin_port));
}

/*
 * Chaque client correspond à un thread service
 * Le 1er message du client indiquera le service voulu et sa version
 * Etablir la connection avec le service
 * Ecouter ensuite sur la socket client et socket unix pour traiter les messages
 */
void * service_thread(void * c_data) {
    client_data *cda = (client_data*) c_data;
    char *service;
    int version;
    int clientSock = cda->sfd;
    int nbMessages = 0;

    //buffer for message
    size_t nbytes = 0;
    char *buffer = malloc (BUFSIZ);
    if (buffer == NULL) {
        perror("malloc()");
        return NULL;
    }
    memset(buffer, 0, BUFSIZ);

    if (read_message(clientSock, buffer) == -1) {
        perror("read_message()");
        return NULL;
    }else {
        parseServiceVersion(buffer, &service, &version);
    }

    /* TODO : service correspond au service que le client veut utiliser 
     ** il faut comparer service à un tableau qui contient les services 
     ** disponibles
     */

    //pid index version
    char * piv = malloc(PATH_MAX);
    memset(piv , 0, PATH_MAX);
    if (piv == NULL) {
        perror("malloc()");
    }
    makePivMessage(&piv, getpid(), cda->index, version);

    struct service srv;
    srv_init (0, NULL, NULL, &srv, service, NULL);
    if (app_srv_connection(&srv, piv, strlen(piv)) == -1) {
        handle_error("app_srv_connection\n");
    }
    free(piv);

    /*struct process p;
    app_create(&p, getpid(), cda->index, version);
    srv_process_print(&p);*/
    //sleep(1);
    //printf("%s\n",p.path_proc );
    /*if (proc_connection(&p) == -1){
        handle_error("proc_connection");
    }*/
    //utilisation du select() pour surveiller la socket du client et fichier in
    fd_set rdfs;

    int max = clientSock > srv.service_fd ? clientSock : srv.service_fd;

    printf("Waitting for new messages...\n" );
    while(1) {
        FD_ZERO(&rdfs);

        //add client's socket
        FD_SET(clientSock, &rdfs);

        //add in file
        FD_SET(srv.service_fd, &rdfs);

        if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
        {
            perror("select()");
            exit(errno);
        }

        if (FD_ISSET(srv.service_fd, &rdfs)){
            nbytes = file_read(srv.service_fd, &buffer);
            if(nbytes < 0) {
                perror("app_read()");
            }
            printf("message from file : %s\n", buffer );
            write_message(clientSock, buffer, nbytes);
            nbMessages--;

        } else if (FD_ISSET(clientSock, &rdfs)) {
            nbytes = read_message(clientSock, buffer);
            if(nbytes > 0 && strncmp(buffer, "exit", 4) != 0) {
                printf("Server : message (%ld bytes) : %s\n", nbytes, buffer);
                if(file_write(srv.service_fd, buffer, nbytes) < 0) {
                    perror("file_write");
                }

                nbMessages++;
            } 
            if (strncmp(buffer, "exit", 4) == 0 && nbMessages == 0){
                //message end to server
                if(file_write(srv.service_fd, "exit", 4) < 0) {
                    perror("file_write");
                }

                printf("------thread %d shutdown----------\n\n", cda->index );

                break;
            }           
        }
    }
    //close the files descriptors
    close(srv.service_fd);
    close(clientSock);
    free(buffer);

    //release the resources
    pthread_detach(pthread_self());
    return NULL;
}

void parseServiceVersion(char * buf, char ** service, int *version) {
    char *token = NULL, *saveptr = NULL;
    char *str = NULL;
    int i = 0;

    for (str = buf, i = 1; ; str = NULL, i++) {
        token = strtok_r(str, " ", &saveptr);
        if (token == NULL)
            break;

        if (i == 1) {
            *service = token;
        }
        else if (i == 2) {
            *version = strtoul(token, NULL, 10);
        }
    }

}

int fifo_create (char * path)
{
    int ret;
    if ((ret = mkfifo (path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) {
        switch (errno) {
            case EACCES :
                printf ("file %s : EACCES\n", path);
                return 1;
            case EEXIST :
                printf ("file %s : EEXIST\n", path);
                break;
            case ENAMETOOLONG :
                printf ("file %s : ENAMETOOLONG\n", path);
                return 2;
            case ENOENT :
                printf ("file %s : ENOENT\n", path);
                return 3;
            case ENOSPC :
                printf ("file %s : ENOSPC\n", path);
                return 4;
            case ENOTDIR :
                printf ("file %s : ENOTDIR\n", path);
                return 5;
            case EROFS :
                printf ("file %s : EROFS\n", path);
                return 6;
            default :
                printf ("err file %s unknown\n", path);
                return 7;
        }
    }

    return ret;
}

void inOutPathCreate(char ** pathname, int index, int version) {
    snprintf(pathname[0] , PATH_MAX, "%s%d-%d-%d-in" , TMPDIR, getpid(), index, version);
    snprintf(pathname[1] , PATH_MAX, "%s%d-%d-%d-out", TMPDIR, getpid(), index, version);
}

void makePivMessage (char ** piv, int pid, int index, int version) {

    snprintf(*piv , PATH_MAX, "%d %d %d" , getpid(), index, version);

}

/*
 * lancer le serveur, ecouter sur une l'adresse et port
 * A chaque nouveau client lance un thread service
 */
void * server_thread(void * reqq) {
    info_request *req = (info_request*) reqq;

    //client 
    client_data tab_client[NBCLIENT];
    pthread_t tab_service_threads[NBCLIENT];
    int actual = 0;
    int i;

    int sock = init_connection(req);
    fd_set rdfs;

    printf("Waitting for new clients :\n" );
    while(1) {
        FD_ZERO(&rdfs);

        /* add STDIN_FILENO */
        FD_SET(STDIN_FILENO, &rdfs);

        //add listener's socket
        FD_SET(sock, &rdfs);

        if(select(sock + 1, &rdfs, NULL, NULL, NULL) == -1)
        {
            perror("select()");
            exit(errno);
        }

        /* something from standard input : i.e keyboard */
        if(FD_ISSET(STDIN_FILENO, &rdfs))
        {
            /* stop process when type on keyboard */
            for (i = 0; i < actual; i++) {
                if (pthread_cancel(tab_service_threads[i]) != 0) {
                    printf("Aucun thread correspond \n");
                }   
            }
            printf("server shutdown\n");
            break; 
        }
        else if (FD_ISSET(sock, &rdfs)){
            //New client
            socklen_t sinsize = sizeof (struct sockaddr_in);
            tab_client[actual].sfd = accept(sock, (struct sockaddr *)&tab_client[actual].c_addr, &sinsize);
            if(tab_client[actual].sfd == -1)
            {
                perror("accept()");
                close(sock);
                exit(errno);
            }

            printf("New client :\n");
            printAddr(&tab_client[actual].c_addr);

            tab_client[actual].index = actual;

            int ret = pthread_create( &tab_service_threads[actual], NULL, &service_thread, (void *) &tab_client[actual]);
            if (ret) {
                perror("pthread_create()");
                endConnection(sock);
                exit(errno);
            } else {
                printf("\n----------Creation of listen thread %d ------------\n", actual);
            }

            actual++;
        }
    }

    for (i = 0; i < actual; i++) {
        pthread_join(tab_service_threads[i], NULL); 
    }

    printf("--------------server thread shutdown--------------- \n");
    endConnection(sock);

    return NULL;
}

/*
*   user can send 2 types of request to server : listen or connect
*   listen = server for a service such as pongd
*   connect = connect to a server
*/
int srv_get_new_request(char *buf, info_request *req) {

    char *token = NULL, *saveptr = NULL;
    char *str = NULL;
    int i = 0;

    //for a "connect" request
    pid_t pid = 0;
    int index = 0;
    int version = 0;

    for (str = buf, i = 1; ; str = NULL, i++) {
        token = strtok_r(str, " ", &saveptr);
        if (token == NULL)
            break;

        if (i == 1) {
            if(strncmp("exit", token, 4) == 0 ) {
                strncpy(req->request, token, 4);
                free(str);
                return 0;
            }
            req->request = token;
        }
        else if (i == 2){ 
            req->addr.sin_addr.s_addr = inet_addr(token);
        }
        else if (i == 3) {
            req->addr.sin_port = htons(strtoul(token, NULL, 10));
        }
        else if (i == 4 && (strcmp("connect", req->request)) == 0){
            pid = strtoul(token, NULL, 10);
        }
        else if (i == 5 && (strcmp("connect", req->request)) == 0) {
            index = strtoul(token, NULL, 10);
        }
        else if (i == 6 && (strcmp("connect", req->request)) == 0) {
            version = strtoul(token, NULL, 10);
        }
        
    }

    req->addr.sin_family = AF_INET;

    if (strcmp("connect", req->request) == 0) {
        srv_process_gen (req->p, pid, index, version);
    }

    return 1;
}

/*
 * client thread est lancé suite à une requete "connect"
 * connecter à une adresse, port le 1er message indiquera le service et le version voulus
 * Se mettre ensuite sur l'écoute de la socket serveur et le fichier client pour traiter les messages
 */
void * client_thread(void *reqq) {
    info_request *req = (info_request*) reqq;
    /* buffer for client data */
    int nbytes;
    char *buffer= malloc(BUFSIZ);
    if (buffer == NULL)
    {
        handle_error("malloc");
    }
    memset(buffer, 0, BUFSIZ);;
    int nbMessages = 0;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)
    {
        perror("socket()");
        exit(errno);
    }

    if(connect(sock,(struct sockaddr *) &req->addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect()");
        exit(errno);
    }

    printf("Connected to server at :\n");
    printAddr(&req->addr);

    write_message(sock, "pongd 5", strlen("pongd 5"));

    /*//init socket unix for server
    int sfd;
    struct sockaddr_un peer_addr;
    socklen_t peer_addr_size;

    sfd = set_listen_socket(req->p->path_proc);
    if (sfd == -1){
        handle_error("set_listen_socket");
    }*/

    /* master file descriptor list */
    fd_set master;
    /* temp file descriptor list for select() */
    fd_set read_fds;

    /* maximum file descriptor number */
    int fdmax;
    /* listening socket descriptor */
    int listener = req->p->proc_fd;
    /* newly accept()ed socket descriptor */
    //int newfd;

    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    /* add the listener to the master set */
    FD_SET(listener, &master);
    FD_SET(sock, &master);

    /* keep track of the biggest file descriptor */
    fdmax = sock > listener ? sock : listener;

    while(1) {
        /* copy it */
        read_fds = master;

        if(select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select()");
            exit(errno);
        }
        printf("select...OK\n");


        // if(FD_ISSET(listener, &read_fds)) {
        //     /* handle new connections */
        //     peer_addr_size = sizeof(struct sockaddr_un);
        //     newfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
        //     if (newfd == -1) {
        //         handle_error("accept");
        //     }
        //     else
        //     {
        //         printf("Server-accept() is OK...\n");
        //         FD_SET(newfd, &master); /* add to master set */
        //         if(newfd > fdmax)
        //         { /* keep track of the maximum */
        //             fdmax = newfd;
        //         }
        //         req->p->proc_fd = newfd;
        //     }
        // }
        /*
         * TODO:
         * Que se passe-t-il si on reçoit un message du serveur avant l'app client?
         * Ou ecrit-on le message ??
         */ 
        /*else*/ if (FD_ISSET(sock, &read_fds)) { 
            int n = read_message(sock, buffer);
            if(n > 0) {
                printf("Client : message from server(%d bytes) : %s\n", n, buffer);
                if(app_write(req->p, buffer, strlen(buffer)) < 0) {
                    perror("file_write");
                }
                nbMessages--;     

            } else if (n == 0){
                //message end from server
                printf("server down\n");
                printf("------thread client shutdown----------\n");

                break;
            }
        }else {
            nbytes = app_read (req->p, &buffer);
            printf("Client : message from app %d : %s\n",nbytes, buffer );
            if ( nbytes == -1) {
                handle_error("file_read");
            } else if( nbytes == 0) {
                /* close it... */
                close(req->p->proc_fd);
                /* remove from master set */
                FD_CLR(req->p->proc_fd, &master);
                break;
            }else {
                //printf("Client : size message %d \n",nbytes );
                write_message(sock, buffer, nbytes);
                if (strncmp(buffer, "exit", 4) != 0) {
                    nbMessages++;
                }
            }
        }   
    }

    printf("------thread client shutdown----------\n");
    close(listener);
    close(sock);
    free(buffer);

    //release the resources
    pthread_detach(pthread_self());
    return NULL;

}

void request_print (const info_request *req) {
    printf("%s \n",req->request);
}

/*
 *Surveiller le fichier tmp/ipc/service
 *Accepter 2 types de requetes :
 * listen : lancer un serveur, ecouter sur un port ie "listen 127.0.0.1 6000" 
 * connect : connecter à une adresse, port ie "connect 127.0.0.1 6000 ${pid} 1 1"
 */
void main_loop (struct service *srv) {
    //request
    info_request tab_req[NBCLIENT];
    int ret;
    int i;
    //pid server
    pthread_t pid_s;
    //pid client
    pthread_t tab_client[NBCLIENT];
    int nbclient = 0;

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

    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    /* add the listener to the master set */
    FD_SET(listener, &master);
    //FD_SET(sfd, &master);

    /* keep track of the biggest file descriptor */
    fdmax = listener; /* so far, it's this one*/

    for(;;) {
        /* copy it */
        read_fds = master;
        if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("Server-select() error lol!");
            exit(1);
        }
        //printf("Server-select...OK\n");

        for (i = 0; i <= fdmax; i++) {
            if(FD_ISSET(i, &read_fds)) {
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
			} else if( nbytes == 0) {
			    /* close it... */
			    close(i);
			    /* remove from master set */
			    FD_CLR(i, &master);
			}else {
			    buf[BUFSIZ - 1] = '\0';
			    printf ("msg received (%d) : %s\n", nbytes, buf);
			    if (strncmp ("exit", buf, 4) == 0) {
				break;
			    }

			    tab_req[nbclient].p = malloc(sizeof(struct process));
			    // -1 : error, 0 = no new process, 1 = new process
			    ret = srv_get_new_request (buf, &tab_req[nbclient]);
			    tab_req[nbclient].p->proc_fd = newfd;
			    if (ret == -1) {
				perror("srv_get_new_request()");
				exit(1);
			    } else if (ret == 0) {
				break;
			    }

			    request_print(&tab_req[nbclient]);

			    if (strcmp("listen", tab_req[nbclient].request) == 0) {
				int ret = pthread_create( &pid_s, NULL, &server_thread, (void *) &tab_req[nbclient]);
				if (ret) {
				    perror("pthread_create()");
				    exit(errno);
				} else {
				    printf("\n----------Creation of server thread ------------\n");
				}
				nbclient++;
			    }else {
				int ret = pthread_create( &tab_client[nbclient], NULL, &client_thread, (void *) &tab_req[nbclient]);
				if (ret) {
				    perror("pthread_create()");
				    exit(errno);
				} else {
				    printf("\n----------Creation of client thread ------------\n");
				}
				nbclient++;

			    }
			}
                    }
                } /*else {
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
                        if (strncmp ("exit_server", buf, 4) == 0) {
                            break;
                        }

                        tab_req[nbclient].p = malloc(sizeof(struct process));
                        // -1 : error, 0 = no new process, 1 = new process
                        ret = srv_get_new_request (buf, &tab_req[nbclient]);
                        tab_req[nbclient].p->proc_fd = i;
                        if (ret == -1) {
                            perror("srv_get_new_request()");
                            exit(1);
                        } else if (ret == 0) {
                            break;
                        }

                        request_print(&tab_req[nbclient]);

                        if (strcmp("listen", tab_req[nbclient].request) == 0) {
                            int ret = pthread_create( &pid_s, NULL, &server_thread, (void *) &tab_req[nbclient]);
                            if (ret) {
                                perror("pthread_create()");
                                exit(errno);
                            } else {
                                printf("\n----------Creation of server thread ------------\n");
                            }
                            nbclient++;
                        }else {
                            int ret = pthread_create( &tab_client[nbclient], NULL, &client_thread, (void *) &tab_req[nbclient]);
                            if (ret) {
                                perror("pthread_create()");
                                exit(errno);
                            } else {
                                printf("\n----------Creation of client thread ------------\n");
                            }
                            nbclient++;

                        }
                    }
                }*/ //i == listener
            } //if FDISSET
        }//boucle for
        if (strncmp ("exit", buf, 4) == 0) {
            break;
        }
    }//boucle while

    if (pthread_cancel(pid_s) != 0) {
        printf("Aucun thread correspond \n");
    }   

    pthread_join(pid_s, NULL);
    
    /*for (i = 0; i < nbclient; i++) {
        pthread_join(tab_client[i], NULL);
    }*/

    for (i = 0; i < nbclient; i++) {
        free(tab_req[i].p);
    }
    free(buf);
    close(sfd);
}

int main(int argc, char * argv[], char **env) {
    struct service srv;
    srv_init (argc, argv, env, &srv, SERVICE_TCP, NULL);
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

    return 0;
}
