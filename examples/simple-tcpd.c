#include "../src/ipc.h"
// #include "../src/log.h"
#include "../src/utils.h"
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>

#define CLOG_DEBUG(a, ...) LOG_DEBUG (""); LOG_DEBUG("\033[36m" a "\033[00m", #__VA_ARGS__); LOG_DEBUG ("")

void chomp (char *str, size_t len)
{
	if (str[len - 1] == '\n') {
		str[len - 1] = '\0';
	}
	if (str[len - 2] == '\n') {
		str[len - 2] = '\0';
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
							2. ipcd establishes a connection to the service
							3. ack
				else
					lolwat shouldn't happen :(
			elif reading on usual socket
				do something
 **/

#define SERVICE_NAME "simpletcp"

struct ipc_ctx *ctx = NULL;

void handle_disconnection (int fd)
{
	int delfd;

	delfd = ipc_switching_del (&ctx->switchdb, fd);
	if (delfd >= 0) {
		close (delfd);
		ipc_del_fd (ctx, delfd);
	}

	close (fd);
	ipc_del_fd (ctx, fd);

	// printf ("ctx.switchdb\n");
	ipc_switching_print (&ctx->switchdb);
}

void tcp_connection (int fd)
{
	SECURE_BUFFER_DECLARATION (char, buf, BUFSIZ);

	ssize_t len = recv (fd, buf, BUFSIZ, 0);
	if (len <= 0) {
		handle_disconnection (fd);
		return;
	}

	buf[len] = '\0';

	// XXX: for testing purposes
	chomp (buf, len);

	// TODO: tests
	T_PERROR_Q ((send (fd, "OK", 2, 0) <= 0), "sending a message", EXIT_FAILURE);

	printf ("connection to %s\n", buf);
	struct ipc_error ret = ipc_connection_switched (ctx, buf, fd, NULL);
	if (ret.error_code != IPC_ERROR_NONE) {
		fprintf (stderr, "%s\n", ret.error_message);
		exit (EXIT_FAILURE);
	}
}

int accept_new_client (int serverfd)
{
	SECURE_DECLARATION (struct sockaddr_in, client);
	socklen_t addrlen = 0;

	int sock_fd_client;
	T_PERROR_Q (((sock_fd_client =
		accept (serverfd, (struct sockaddr *)&client, &addrlen)) == -1), "accept new client",
		EXIT_FAILURE);

	// adding a client, for now not switched:
	// tcpd should handle the first message (getting the service name)
	ipc_add_fd (ctx, sock_fd_client);

	return sock_fd_client;
}

void main_loop (int argc, char **argv)
{
	argc = argc;		// FIXME: useless
	int serverfd;

	SECURE_DECLARATION (struct sockaddr_in, my_addr);
	socklen_t addrlen;

	// socket factory
	if ((serverfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		perror ("socket");
		return;
	}

	int yes = 1;

	if (setsockopt (serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1) {
		perror ("setsockopt");
		return;
	}
	// init local addr structure and other params
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons (atoi (argv[1]));
	my_addr.sin_addr.s_addr = INADDR_ANY;
	addrlen = sizeof (struct sockaddr_in);

	// bind addr structure with socket
	if (bind (serverfd, (struct sockaddr *)&my_addr, addrlen) == -1) {
		perror ("bind");
		return;
	}
	// set the socket in passive mode (only used for accept())
	// and set the list size for pending connection
	if (listen (serverfd, 5) == -1) {
		perror ("listen");
		return;
	}

	SECURE_DECLARATION (struct ipc_event, event);

	ipc_add_fd (ctx, serverfd);

	int cpt = 0;

	int timer = 10000;
	while (1) {
		// ipc_wait_event provides one event at a time
		// warning: event->m is free'ed if not NULL

		ipc_ctx_print (ctx);
		TEST_IPC_WAIT_EVENT_Q (ipc_wait_event (ctx, &event, &timer), EXIT_FAILURE);

		switch (event.type) {
		case IPC_EVENT_TYPE_TIMER:{
				printf ("timed out!\n");
				timer = 10000;
			}
			break;

		case IPC_EVENT_TYPE_SWITCH:{
				printf ("switch happened, from %d\n", event.origin);
			}
			break;

		case IPC_EVENT_TYPE_EXTRA_SOCKET:
			{
				// NEW CLIENT
				if (event.origin == serverfd) {
					int sock_fd_client = accept_new_client (serverfd);
					cpt++;
					printf ("TCP connection: %d ctx connected\n", cpt);
					printf ("new TCP client has the fd %d\n", sock_fd_client);
				}
				// CLIENT IS TALKING
				else {
					// Test: if the socket already is in the switch, this means we can just switch the packet.
					// Is the socket in the switch db?
					tcp_connection (event.origin);
				}
			}
			break;

		case IPC_EVENT_TYPE_CONNECTION:
			{
				cpt++;
				printf ("connection: %d ctx connected\n", cpt);
				printf ("new client has the fd %d\n", event.origin);
			};
			break;

		case IPC_EVENT_TYPE_DISCONNECTION:
			{
				cpt--;
				printf ("disconnection: %d ctx remaining\n", cpt);
			};
			break;

		case IPC_EVENT_TYPE_MESSAGE:
			{
				struct ipc_message *m = event.m;
				if (m->length > 0) {
					printf ("message received (type %d): %.*s\n", m->type, m->length, m->payload);
				}
				// m->fd = 3;
				TEST_IPC_P (ipc_write (ctx, m), "server write");
			};
			break;

		case IPC_EVENT_TYPE_TX:
			{
				printf ("a message was sent\n");
			}
			break;

		case IPC_EVENT_TYPE_ERROR:
			fprintf (stderr, "a problem happened with client %d\n", event.origin);
			break;

		default:
			fprintf (stderr, "there must be a problem, event not set: %d\n", event.type);
			exit(1);
		}
	}

	// should never go there
	exit (EXIT_FAILURE);
}

void exit_program (int signal)
{
	printf ("Quitting, signal: %d\n", signal);

	// Close then free remaining ctx.
	ipc_close_all (ctx);
	ipc_ctx_free  (ctx);

	// free, free everything!
	free (ctx);

	exit (EXIT_SUCCESS);
}

/*
 * service ping-pong: send back everything sent by the ctx
 * stop the program on SIG{TERM,INT,ALRM,USR{1,2},HUP} signals
 */

int main (int argc, char *argv[])
{
	// check the number of args on command line
	if (argc != 2) {
		printf ("USAGE: %s port_num\n", argv[0]);
		exit (EXIT_FAILURE);
	}

	printf ("pid = %d\n", getpid ());

	ctx = malloc (sizeof (struct ipc_ctx));
	memset(ctx, 0, sizeof (struct ipc_ctx));

	struct ipc_error ret = ipc_server_init (ctx, SERVICE_NAME);
	if (ret.error_code != IPC_ERROR_NONE) {
		fprintf (stderr, "%s\n", ret.error_message);
		return EXIT_FAILURE;
	}
	printf ("Listening on [%s].\n", ctx->cinfos[0].spath);

	printf ("MAIN: server created\n");

	signal (SIGHUP  , exit_program);
	signal (SIGALRM , exit_program);
	signal (SIGUSR1 , exit_program);
	signal (SIGUSR2 , exit_program);
	signal (SIGTERM , exit_program);
	signal (SIGINT  , exit_program);

	// the service will loop until the end of time, or a signal
	main_loop (argc, argv);

	// main_loop should not return
	return EXIT_FAILURE;
}
