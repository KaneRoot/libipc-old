#include "../src/ipc.h"
#include "../src/utils.h"
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>

#define NTAB(t)     ((int) (sizeof (t) / sizeof (t)[0]))
#define SECURE_BUFFER_DECLARATION(type,name,size) type name[size]; memset(&name, 0, sizeof(type) * size);
// print error string
#define PRINT_ERR_STR(code)  const char *estr = ipc_errors_get (code); fprintf (stderr, "%s", estr);
// Test Print Quit
#define T_P_Q(f,err,q) { ret = f; if (ret != IPC_ERROR_NONE) { fprintf (stderr, err); PRINT_ERR_STR(ret); exit (q); } }

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
				elif reading on sock_client
					TODO (first draft): print messages
				else
					lolwat shouldn't happen :(
			elif reading on usual socket
				do something
 **/

#define SERVICE_NAME "simpletcp"

#define PRINTERR(ret,msg) {\
	const char * err = ipc_errors_get (ret);\
	fprintf(stderr, "error while %s: %s\n", msg, err);\
}

struct ipc_switching {
	int orig;
	int dest;
};

struct ipc_switchings {
	struct ipc_switching *collection;
	size_t size;
};

struct networkd {
	int cpt;
	struct ipc_connection_info *srv;
	struct ipc_connection_infos *clients;
	struct ipc_switchings * TCP_TO_IPC;
	struct ipc_switchings * IPC_TO_TCP;
};

struct networkd * ctx;


void ipc_switching_add (struct ipc_switchings *is, int orig, int dest)
{
	is->collection = realloc(is->collection, sizeof(struct ipc_switching) * (is->size+1));
	if (is->collection == NULL) {
		printf ("error realloc\n");
		exit (EXIT_FAILURE);
	}

	is->size++;

	is->collection[is->size-1].orig = orig;
	is->collection[is->size-1].dest = dest;
}

int ipc_switching_del (struct ipc_switchings *is, int orig)
{
	for (size_t i = 0; i < is->size; i++) {
		if (is->collection[i].orig == orig) {
			is->collection[i].orig = is->collection[is->size-1].orig;
			int ret = is->collection[i].dest;
			is->collection[i].dest = is->collection[is->size-1].dest;

			size_t s = (is->size - 1) > 0 ? (is->size - 1) : 1;

			is->collection = realloc(is->collection, sizeof(struct ipc_switching) * s);
			if (is->collection == NULL) {
				printf ("error realloc\n");
				exit (EXIT_FAILURE);
			}

			is->size--;
			return ret;
		}
	}

	return -1;
}

int ipc_switching_get (struct ipc_switchings *is, int orig) {
	for (size_t i = 0; i < is->size; i++) {
		if (is->collection[i].orig == orig) {
			return is->collection[i].dest;
		}
	}

	return -1;
}

void ipc_switching_print (struct ipc_switchings *is) {
	printf ("print!\n");
	for (size_t i = 0; i < is->size; i++)
	{
		printf ("client %d - %d\n", is->collection[i].orig, is->collection[i].dest);
	}
}

void tcp_connection (char **env, int fd, char * buf, int len)
{
	printf ("tcp client %d is not already connected to a service\n", fd);
	buf[len] = '\0';

	// for testing purposes
	size_t last_char = strlen ((const char*)buf) -1;
	if (buf[last_char] == '\n') {
		buf[last_char] = '\0';
	}

	printf ("read something: where to connect %s\n", buf);
	printf ("sending ok\n");

	// TODO: tests
	if (send (fd, "OK", 2, 0) <= 0) {
		fprintf (stderr, "error: cannot send message\n");
		perror("send");
		exit (EXIT_FAILURE);
	}


	SECURE_DECLARATION (struct ipc_connection_info, tcp_to_ipc_ci);

	enum ipc_errors ret = 0;
	T_P_Q (ipc_connection (env, &tcp_to_ipc_ci, buf), "cannot connect to the service\n", EXIT_FAILURE);

	ipc_switching_add (ctx->TCP_TO_IPC, fd, tcp_to_ipc_ci.fd);
	ipc_switching_add (ctx->IPC_TO_TCP, tcp_to_ipc_ci.fd, fd);
	ipc_add_fd (ctx->clients, tcp_to_ipc_ci.fd);
}

void handle_extra_socket (struct ipc_event event, int sockfd, char **env)
{
	SECURE_DECLARATION (struct sockaddr_in, client);
	socklen_t addrlen = 0;

	printf ("something comes from somewhere: fd %d\n", event.origin->fd);

	// NEW CLIENT
	if (event.origin->fd == sockfd) {
		int sock_fd_client;
		if((sock_fd_client = accept(sockfd, (struct sockaddr *) &client, &addrlen)) == -1) {
			perror("accept");
			close(sockfd);
			exit(EXIT_FAILURE);
		}
		printf ("after accept\n");
		// adding a client
		ipc_add_fd (ctx->clients, sock_fd_client);
		printf ("after ipc_add_fd, TCP client: %d\n", sock_fd_client);
	}
	// CLIENT IS TALKING
	else {
		SECURE_BUFFER_DECLARATION(char, buf, 4096);

		ssize_t len = recv (event.origin->fd, buf, 4096, 0);
		if (len > 0) {

			print_hexa ("RECEIVED", (unsigned char*) buf, len);

			// TODO: check if the message comes from an external IPC
			int fd = ipc_switching_get (ctx->IPC_TO_TCP, event.origin->fd);

			if (fd >= 0) {
				printf ("SWITCH: ipc service %d sent a message for %d\n", event.origin->fd, fd);
				int sent_len = send (fd, buf, len, 0);
				if (sent_len < 0) {
					perror ("write to ipc");
				}
				else if (sent_len != len) {
					fprintf (stderr, "write NOT ENOUGH to tcp client\n");
				}
			}
			else {
				fd = ipc_switching_get (ctx->TCP_TO_IPC, event.origin->fd);
				if (fd >= 0) {
					printf ("SWITCH: client %d sent a message for %d\n", event.origin->fd, fd);
					int sent_len = send (fd, buf, len, 0);
					if (sent_len < 0) {
						perror ("write to ipc");
					}
					else if (sent_len != len) {
						fprintf (stderr, "write NOT ENOUGH to ipc\n");
					}
				}
				else {
					tcp_connection (env, event.origin->fd, buf, len);
				}
			}
		}
		else if (len == 0) {
			// disconnection
			printf ("close connection\n");

			int delfd = ipc_switching_del (ctx->TCP_TO_IPC, event.origin->fd);
			if (delfd >= 0) {
				close (delfd);
				ipc_del_fd (ctx->clients, delfd);
			}

			delfd = ipc_switching_del (ctx->IPC_TO_TCP, event.origin->fd);
			if (delfd >= 0) {
				close (delfd);
				ipc_del_fd (ctx->clients, delfd);
			}

			close (event.origin->fd);
			ipc_del_fd (ctx->clients, event.origin->fd);
		}
	}
}


void main_loop (int argc, char **argv, char **env)
{
	argc = argc; // FIXME: useless
	int sockfd;

	struct sockaddr_in my_addr;
	socklen_t addrlen;

	// socket factory
	if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	int yes = 1;

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror ("setsockopt");
	}


	// init local addr structure and other params
	my_addr.sin_family      = AF_INET;
	my_addr.sin_port        = htons(atoi(argv[1]));
	my_addr.sin_addr.s_addr = INADDR_ANY;
	addrlen                 = sizeof(struct sockaddr_in);

	// bind addr structure with socket
	if(bind(sockfd, (struct sockaddr *) &my_addr, addrlen) == -1)
	{
		perror("bind");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	// set the socket in passive mode (only used for accept())
	// and set the list size for pending connection
	if(listen(sockfd, 5) == -1)
	{
		perror("listen");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	printf("Waiting for incomming connection\n");


    enum ipc_errors ret = 0; 

	ctx->clients = malloc (sizeof (struct ipc_connection_infos));
	memset(ctx->clients, 0, sizeof(struct ipc_connection_infos));

	struct ipc_event event;
	memset(&event, 0, sizeof (struct ipc_event));
	printf ("adding sockfd to ctx->clients\n");
	ipc_add_fd (ctx->clients, sockfd);


    while(1) {
		// ipc_service_poll_event provides one event at a time
		// warning: event->m is free'ed if not NULL
		// printf ("before wait event\n"); // TODO remove
		ret = ipc_wait_event (ctx->clients, ctx->srv, &event);
		// printf ("after wait event\n"); // TODO remove
		if (ret != IPC_ERROR_NONE && ret != IPC_ERROR_CLOSED_RECIPIENT) {
			PRINTERR(ret,"wait event");

			// the application will shut down, and close the service
			ret = ipc_server_close (ctx->srv);
			if (ret != IPC_ERROR_NONE) {
				PRINTERR(ret,"server close");
			}
			exit (EXIT_FAILURE);
		}

		switch (event.type) {
			case IPC_EVENT_TYPE_EXTRA_SOCKET:
				{
					handle_extra_socket (event, sockfd, env);
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
					free (event.origin);
				};
				break;
			case IPC_EVENT_TYPE_MESSAGE:
			   	{
					struct ipc_message *m = event.m;
					if (m->length > 0) {
						printf ("message received (type %d): %.*s\n", m->type, m->length, m->payload);
					}

					ret = ipc_write (event.origin, m);

					if (ret != IPC_ERROR_NONE) {
						handle_err( "handle_new_msg", "server_write < 0");
						PRINTERR(ret,"server write");
					}
				};
				break;
			case IPC_EVENT_TYPE_ERROR:
			   	{
					fprintf (stderr, "a problem happened with client %d\n"
							, (event.origin)->fd);
				};
				break;
			default :
				{
					fprintf (stderr, "there must be a problem, event not set\n");
				};
		}
    }

	// should never go there
	exit (EXIT_FAILURE);
}


void exit_program(int signal)
{
	printf("Quitting, signal: %d\n", signal);

	// free remaining clients
	for (int i = 0; i < ctx->clients->size ; i++) {
		struct ipc_connection_info *cli = ctx->clients->cinfos[i];
		if (cli != NULL) {
			free (cli);
		}
		ctx->clients->cinfos[i] = NULL;
	}

	ipc_connections_free (ctx->clients);
	free (ctx->clients);


    // the application will shut down, and close the service
	enum ipc_errors ret = ipc_server_close (ctx->srv);
    if (ret != IPC_ERROR_NONE) {
		PRINTERR(ret,"server close");
    }
	free (ctx->srv);

	free (ctx->TCP_TO_IPC->collection);
	free (ctx->TCP_TO_IPC);
	free (ctx->IPC_TO_TCP->collection);
	free (ctx->IPC_TO_TCP);
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
		exit(-1);
	}

	printf ("pid = %d\n", getpid ());

	ctx = malloc (sizeof (struct networkd));
	memset (ctx, 0, sizeof (struct networkd));


	ctx->TCP_TO_IPC = malloc(sizeof(struct ipc_switchings));
	memset (ctx->TCP_TO_IPC, 0, sizeof(struct ipc_switchings));
	ctx->TCP_TO_IPC->collection = malloc(sizeof(struct ipc_switching));
	ctx->IPC_TO_TCP = malloc(sizeof(struct ipc_switchings));
	memset (ctx->IPC_TO_TCP, 0, sizeof(struct ipc_switchings));
	ctx->IPC_TO_TCP->collection = malloc(sizeof(struct ipc_switching));

	ctx->srv = malloc (sizeof (struct ipc_connection_info));
	if (ctx->srv == NULL) {
		exit (EXIT_FAILURE);
	}
    memset (ctx->srv, 0, sizeof (struct ipc_connection_info));
    ctx->srv->type = '\0';
    ctx->srv->index = 0;
    ctx->srv->version = 0;
    ctx->srv->fd = 0;
    ctx->srv->spath = NULL;

	enum ipc_errors ret = ipc_server_init (env, ctx->srv, SERVICE_NAME);
    if (ret != IPC_ERROR_NONE) {
		PRINTERR(ret,"server init");
        return EXIT_FAILURE;
    }
    printf ("Listening on %s.\n", ctx->srv->spath);

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
