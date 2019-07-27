#include "../src/ipc.h"
#include "../src/utils.h"
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>

#define CLOG_DEBUG(a, ...) LOG_DEBUG (""); LOG_DEBUG("\033[36m" a "\033[00m", #__VA_ARGS__); LOG_DEBUG ("") 

void chomp (char *str, size_t len) {
	if (str[len -1] == '\n') {
		str[len -1] = '\0';
	}
	if (str[len -2] == '\n') {
		str[len -2] = '\0';
	}
}

/*
 *
 * TODO:
 * This program is under heavy developement.
 * Still many things to do.
 * NOT READY FOR PRODUCTION.
 *
	Server side:
		1. sock_tcp = tcp socket
		2. ipc_server_init
		3. ipc_add (ipc_server, sock_tcp)
		4. wait_event
			if reading on extra socket
				if reading on sock_tcp
					sock_client = accept
					ipc_add_fd sock_client 
				elif
					if socket bind to another (client to service or service to client)
						reading on fd ; writing on related fd
					else
						connection from the client:
							1. client sends service name
							2. networkd establishes a connection to the service
							3. ack
				else
					lolwat shouldn't happen :(
			elif reading on usual socket
				do something
 **/

#define SERVICE_NAME "simpletcp"

struct networkd * ctx;

void handle_disconnection (int fd)
{
	// disconnection
	LOG_DEBUG ("CLOSE CONNECTION %d", fd);

	int delfd;

	delfd = ipc_switching_del (ctx->TCP_TO_IPC, fd);
	if (delfd >= 0) {
		LOG_DEBUG ("CLOSE RELATED CONNECTION %d", delfd);
		close (delfd);
		ipc_del_fd (ctx->clients, delfd);
	}

	close (fd);
	ipc_del_fd (ctx->clients, fd);

	printf ("\n\n");
	// printf ("TCP_TO_IPC\n");
	ipc_switching_print (ctx->TCP_TO_IPC);

	printf ("\n\n");
}

void tcp_connection (char **env, int fd)
{
	SECURE_BUFFER_DECLARATION(char, buf, BUFSIZ);

	ssize_t len = recv (fd, buf, BUFSIZ, 0);
	if (len <= 0) {
		handle_disconnection (fd);
		return ;
	}

	buf[len] = '\0';

	// XXX: for testing purposes
	chomp (buf, len);
	LOG_DEBUG ("SHOULD CONNECT TO %s", buf);

	// TODO: tests
	T_PERROR_Q ( (send (fd, "OK", 2, 0) <= 0), "sending a message", EXIT_FAILURE);

	SECURE_DECLARATION (struct ipc_connection_info, tcp_to_ipc_ci);

	TIPC_F_Q (ipc_connection (env, &tcp_to_ipc_ci, buf), ("cannot connect to the service [%s]", buf), EXIT_FAILURE);

	ipc_switching_add (ctx->TCP_TO_IPC, fd, tcp_to_ipc_ci.fd);
	ipc_add_fd (ctx->clients, tcp_to_ipc_ci.fd);

	LOG_DEBUG ("CONNECTION TO SERVICE client %d -> %d", fd, tcp_to_ipc_ci.fd);
}

int accept_new_client (int serverfd)
{
	SECURE_DECLARATION (struct sockaddr_in, client);
	socklen_t addrlen = 0;

	int sock_fd_client;
	T_PERROR_Q (((sock_fd_client = accept(serverfd, (struct sockaddr *) &client, &addrlen)) == -1), "accept new client", EXIT_FAILURE);

	// adding a client
	ipc_add_fd (ctx->clients, sock_fd_client);

	return sock_fd_client;
}

void main_loop (int argc, char **argv, char **env)
{
	argc = argc; // FIXME: useless
	int serverfd;

	SECURE_DECLARATION (struct sockaddr_in, my_addr);
	socklen_t addrlen;

	// socket factory
	T_PERROR_R (((serverfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1), "socket", );

	int yes = 1;

	T_PERROR_R ((setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1), "setsockopt", );

	// init local addr structure and other params
	my_addr.sin_family      = AF_INET;
	my_addr.sin_port        = htons(atoi(argv[1]));
	my_addr.sin_addr.s_addr = INADDR_ANY;
	addrlen                 = sizeof(struct sockaddr_in);

	// bind addr structure with socket
	T_PERROR_R ((bind(serverfd, (struct sockaddr *) &my_addr, addrlen) == -1), "bind", );

	// set the socket in passive mode (only used for accept())
	// and set the list size for pending connection
	T_PERROR_R ((listen(serverfd, 5) == -1), "listen", );

	printf("Waiting for incomming connection\n");

	SECURE_BUFFER_HEAP_ALLOCATION_Q (ctx->clients, sizeof (struct ipc_connection_infos), , EXIT_FAILURE);
	SECURE_DECLARATION (struct ipc_event, event);

	printf ("adding serverfd to ctx->clients\n");
	ipc_add_fd (ctx->clients, serverfd);


    while(1) {
		// ipc_wait_event provides one event at a time
		// warning: event->m is free'ed if not NULL
		TIPC_T_P_I_R (
				  /* function to test */ ipc_wait_event_networkd (ctx->clients, ctx->srv, &event, ctx->TCP_TO_IPC)
				, /* error condition */  ret != IPC_ERROR_NONE && ret != IPC_ERROR_CLOSED_RECIPIENT
				, /* to say on error */  "wait event"
				, /* to do on error */   LOG_ERROR ("\033[31m error happened\033[00m"); TIPC_P (ipc_server_close (ctx->srv), "server close")
				, /* return function */  return );

		switch (event.type) {
			case IPC_EVENT_TYPE_SWITCH:
				{
					printf ("switch happened\n");
				}
				break;

			case IPC_EVENT_TYPE_EXTRA_SOCKET:
				{
					// NEW CLIENT
					if (event.origin->fd == serverfd) {
						int sock_fd_client = accept_new_client (serverfd);
						ctx->cpt++;
						printf ("TCP connection: %d clients connected\n", ctx->cpt);
						printf ("new TCP client has the fd %d\n", sock_fd_client);
					}
					// CLIENT IS TALKING
					else {
						tcp_connection (env, event.origin->fd);
					}
				}
				break;

			case IPC_EVENT_TYPE_CONNECTION:
				{
					ctx->cpt++;
					printf ("connection: %d clients connected\n", ctx->cpt);
					printf ("new client has the fd %d\n", (event.origin)->fd);
				};
				break;

			case IPC_EVENT_TYPE_DISCONNECTION:
				{
					ctx->cpt--;
					printf ("disconnection: %d clients remaining\n", ctx->cpt);

					// free the ipc_client structure
					// if (event.origin != NULL)
					// 	free (event.origin);
				};
				break;
			case IPC_EVENT_TYPE_MESSAGE:
			   	{
					struct ipc_message *m = event.m;
					if (m->length > 0) {
						printf ("message received (type %d): %.*s\n", m->type, m->length, m->payload);
					}

					TIPC_P (ipc_write (event.origin, m), "server write");
				};
				break;
			case IPC_EVENT_TYPE_ERROR:
				fprintf (stderr, "a problem happened with client %d\n", (event.origin)->fd);
				break;
			default :
				fprintf (stderr, "there must be a problem, event not set\n");
		}
    }

	// should never go there
	exit (EXIT_FAILURE);
}


void exit_program(int signal)
{
	printf("Quitting, signal: %d\n", signal);

	// free remaining clients
	for (size_t i = 0; i < ctx->clients->size ; i++) {
		struct ipc_connection_info *cli = ctx->clients->cinfos[i];
		if (cli != NULL) {
			free (cli);
		}
		ctx->clients->cinfos[i] = NULL;
	}

	ipc_connections_free (ctx->clients);

    // the application will shut down, and close the service
	TIPC_P (ipc_server_close (ctx->srv), "server close");

	// free, free everything!
	free (ctx->clients);
	free (ctx->srv);
	free (ctx->TCP_TO_IPC->collection);
	free (ctx->TCP_TO_IPC);
	free (ctx);

	exit(EXIT_SUCCESS);
}

/*
 * service ping-pong: send back everything sent by the clients
 * stop the program on SIG{TERM,INT,ALRM,USR{1,2},HUP} signals
 */

int main(int argc, char * argv[], char **env)
{
	// check the number of args on command line
	if(argc != 2)
	{
		printf("USAGE: %s port_num\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	printf ("pid = %d\n", getpid ());

	SECURE_BUFFER_HEAP_ALLOCATION_Q (ctx, sizeof (struct networkd), , EXIT_FAILURE);
	SECURE_BUFFER_HEAP_ALLOCATION_Q (ctx->TCP_TO_IPC, sizeof(struct ipc_switchings), , EXIT_FAILURE);
	SECURE_BUFFER_HEAP_ALLOCATION_Q (ctx->TCP_TO_IPC->collection, sizeof(struct ipc_switching), , EXIT_FAILURE);
	SECURE_BUFFER_HEAP_ALLOCATION_Q (ctx->srv, sizeof (struct ipc_connection_info), , EXIT_FAILURE);

	TIPC_P_R (ipc_server_init (env, ctx->srv, SERVICE_NAME), "server init", EXIT_FAILURE);
    printf ("Listening on [%s].\n", ctx->srv->spath);

    printf("MAIN: server created\n" );

	signal (SIGHUP, exit_program);
	signal (SIGALRM, exit_program);
	signal (SIGUSR1, exit_program);
	signal (SIGUSR2, exit_program);
	signal (SIGTERM, exit_program);
	signal (SIGINT, exit_program);

    // the service will loop until the end of time, or a signal
    main_loop (argc, argv, env);

	// main_loop should not return
    return EXIT_FAILURE;
}
