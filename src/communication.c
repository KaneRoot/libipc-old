#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include <stdio.h>
#include <errno.h>		// error numbers

#include <stdlib.h>
#include <string.h>

#include "ipc.h"
#include "utils.h"

// print structures
#include "message.h"

#include <fcntl.h>

int fd_is_valid(int fd)
{
	return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}


struct ipc_error ipc_server_init (struct ipc_ctx *ctx, const char *sname)
{
	T_R ((sname == NULL), IPC_ERROR_SERVER_INIT__NO_SERVER_NAME_PARAM);

	// Declaration and instanciation of the new connection (ipc_connection_info + pollfd).
	SECURE_DECLARATION (struct ipc_connection_info, srv);
	srv.type = IPC_CONNECTION_TYPE_SERVER;
	SECURE_DECLARATION(struct pollfd, pollfd);
	pollfd.events = POLLIN;

	// Get the service path.
	SECURE_BUFFER_DECLARATION (char, buf, PATH_MAX);
	TEST_IPC_RR (service_path (buf, sname), "cannot get server path");
	size_t s = strlen (buf);
	if (s > PATH_MAX)
		s = PATH_MAX;
	SECURE_BUFFER_HEAP_ALLOCATION_R (srv.spath, s + 1,, IPC_ERROR_SERVER_INIT__MALLOC);
	memcpy (srv.spath, buf, s);
	srv.spath[s] = '\0';	// to be sure

	// Socket initialisation for the service.
	TEST_IPC_RETURN_ON_ERROR (usock_init (&pollfd.fd, srv.spath));

	// Add the server to the listened file descriptors.
	// ipc_add allocate memory then copy the data of srv and pollfd in ctx.
	TEST_IPC_RR (ipc_add (ctx, &srv, &pollfd), "cannot add the server in the context");

	IPC_RETURN_NO_ERROR;
}

// when ipcd is not working properly (or do not retrieve the service): srv->fd = 0
struct ipc_error ipc_contact_ipcd (int *pfd, const char *sname)
{
	T_R ((pfd == NULL),   IPC_ERROR_CONTACT_IPCD__NO_FD_PARAM);
	T_R ((sname == NULL), IPC_ERROR_CONTACT_IPCD__NO_SERVICE_NAME_PARAM);

	// In case there is a problem with ipcd.
	*pfd = 0;

	char *ipcd_var = getenv ("IPC_NETWORK");
	if (ipcd_var == NULL) {
		IPC_RETURN_NO_ERROR;
	}
	// TODO: is there another, more interesting way to do this?
	// currently, IPC_NETWORK is shared with the network service
	// in order to route requests over any chosen protocol stack
	// ex: IPC_NETWORK="audio tor://some.example.com/audio ;pong tls://pong.example.com/pong"

	SECURE_BUFFER_DECLARATION (char, columnthensname, BUFSIZ);
	columnthensname[0] = ';';
	memcpy (columnthensname + 1, sname, strlen (sname));

	if (strncmp (ipcd_var, sname, strlen (sname)) != 0 && strstr (ipcd_var, columnthensname) == NULL) {
		IPC_RETURN_NO_ERROR;
	}

	// Get the service path.
	SECURE_BUFFER_DECLARATION (char, buf, PATH_MAX);
	TEST_IPC_RR (service_path (buf, "network"), "cannot get network service path");

	int ipcd_fd = 0;

	TEST_IPC_RETURN_ON_ERROR (usock_connect (&ipcd_fd, buf));

	SECURE_DECLARATION (struct ipc_message, msg);
	msg.type = MSG_TYPE_NETWORK_LOOKUP;
	msg.user_type = MSG_TYPE_NETWORK_LOOKUP;

	SECURE_BUFFER_DECLARATION (char, content, BUFSIZ);
	snprintf (content, BUFSIZ, "%s;%s", sname, ipcd_var);

	msg.length = strlen (content);
	msg.payload = content;

	TEST_IPC_RR (ipc_write_fd (ipcd_fd, &msg), "cannot send a message to ipcd");

	memset (&msg, 0, sizeof(struct ipc_message));

	// ipcd successfully contacted the service or failed.
	// ipcd will tell either OK or NOT OK.
	TEST_IPC_RR (ipc_read_fd (ipcd_fd, &msg), "cannot read the ipcd response");

	// In case ipcd failed.
	if (msg.length != 2) {
		printf ("ipcd failed to contact service: (%d bytes) %s\n"
			, msg.length
			, msg.payload);
		SECURE_DECLARATION(struct ipc_error, ret);
		ret.error_code = IPC_ERROR_CLOSED_RECIPIENT;
		usock_close (ipcd_fd);
		return ret;
	}

	struct ipc_error ret = ipc_receive_fd (ipcd_fd, pfd);
	if (ret.error_code == IPC_ERROR_NONE) {
		usock_close (ipcd_fd);
	}

	return ret;
}

// Create context, contact ipcd, connects to the service.
struct ipc_error ipc_connection_ (struct ipc_ctx *ctx, const char *sname, enum ipc_connection_type type, int *serverfd)
{
	T_R ((ctx == NULL),   IPC_ERROR_CONNECTION__NO_CTX);
	T_R ((sname == NULL), IPC_ERROR_CONNECTION__NO_SERVICE_NAME);

	SECURE_DECLARATION(struct ipc_connection_info, srv);
	srv.type = type;
	SECURE_DECLARATION(struct pollfd, pollfd);
	pollfd.events = POLLIN;

	TEST_IPC_P (ipc_contact_ipcd (&pollfd.fd, sname), "error during ipcd connection");

	// if ipcd did not initiate the connection
	if (pollfd.fd <= 0) {
		// gets the service path
		SECURE_BUFFER_DECLARATION (char, buf, PATH_MAX);
		TEST_IPC_RR (service_path (buf, sname), "cannot get server path");
		TEST_IPC_RETURN_ON_ERROR (usock_connect (&pollfd.fd, buf));
	}

	// Add the server to the listened file descriptors.
	TEST_IPC_RR (ipc_add (ctx, &srv, &pollfd), "cannot add the server in the context");

	if (serverfd != NULL) {
		*serverfd = pollfd.fd;
	}

	IPC_RETURN_NO_ERROR;
}

int ipc_ctx_fd_type (struct ipc_ctx *ctx, int fd, enum ipc_connection_type type)
{
	if (ctx == NULL) {
		return -1;
	}

	for (size_t i = 0; i < ctx->size; i++) {
		if (ctx->pollfd[i].fd == fd) {
			ctx->cinfos[i].type = type;
			return 0;
		}
	}
	return -1;
}

struct ipc_error ipc_connection (struct ipc_ctx *ctx, const char *sname, int *serverfd)
{
	// Data received on the socket = messages, not new clients, and not switched (no callbacks).
	return ipc_connection_ (ctx, sname, IPC_CONNECTION_TYPE_IPC, serverfd);
}

struct ipc_error ipc_connection_switched (struct ipc_ctx *ctx, const char *sname, int clientfd, int *serverfd)
{
	int sfd = 0;
	// Data received are for switched fd (callbacks should be used).
	struct ipc_error ret = ipc_connection_ (ctx
		, sname
		, IPC_CONNECTION_TYPE_SWITCHED
		, &sfd);

	if (ret.error_code != IPC_ERROR_NONE) {
		return ret;
	}

	if (serverfd != NULL) {
		*serverfd = sfd;
	}

	ipc_add_fd_switched (ctx, clientfd);
	// ipc_ctx_fd_type (ctx, clientfd, IPC_CONNECTION_TYPE_SWITCHED);
	ipc_ctx_switching_add (ctx, clientfd, sfd);

	return ret;
}

struct ipc_error ipc_close_all (struct ipc_ctx *ctx)
{
	T_R ((ctx == NULL), IPC_ERROR_CLOSE_ALL__NO_CTX_PARAM);

	for (size_t i = 0 ; i < ctx->size ; i++) {
		TEST_IPC_P (ipc_close (ctx, i), "cannot close a connection in ipc_close_all");
	}

	IPC_RETURN_NO_ERROR;
}

// Removing all messages for this fd.
void ipc_remove_messages_for_fd (struct ipc_ctx *ctx, int fd)
{
	struct ipc_message *m = NULL;
	size_t looping_count = ctx->tx.size;
	for (size_t i = 0; i < looping_count; i++) {
		m = &ctx->tx.messages[i];
		if (m->fd == fd) {
			// Freeing the message structure.
			ipc_message_empty (m);
			ipc_messages_del (&ctx->tx, i); // remove the message indexed by i
			// Let restart this round
			i--;
			looping_count--;
		}
	}
}

struct ipc_error ipc_close (struct ipc_ctx *ctx, uint32_t index)
{
	T_R ((ctx == NULL), IPC_ERROR_CLOSE__NO_CTX_PARAM);

	SECURE_DECLARATION (struct ipc_error, ret);
	int fd = ctx->pollfd[index].fd;

	// Closing the file descriptor only if it is not an external connection,
	// this should be handled by the libipc user application.
	if (ctx->cinfos[index].type != IPC_CONNECTION_TYPE_EXTERNAL) {
		ret = usock_close (fd);
	}

	// Remove all messages for this fd.
	ipc_remove_messages_for_fd (ctx, fd);

	// Verify that the close was OK.
	if (ret.error_code != IPC_ERROR_NONE) {
		return ret;
	}

	if (ctx->cinfos[index].type == IPC_CONNECTION_TYPE_SERVER) {
		ret = usock_remove (ctx->cinfos[index].spath);
		if (ctx->cinfos[index].spath != NULL) {
			free (ctx->cinfos[index].spath);
			ctx->cinfos[index].spath = NULL;
		}
	}

	return ret;
}

// New connection from a client.
struct ipc_error ipc_accept_add (struct ipc_event *event, struct ipc_ctx *ctx, uint32_t index)
{
	T_R ((ctx == NULL),        IPC_ERROR_HANDLE_NEW_CONNECTION__NO_CINFOS_PARAM);
	T_R ((index >= ctx->size), IPC_ERROR_HANDLE_NEW_CONNECTION__INCONSISTENT_INDEX);

	// Memory reallocation.
	ipc_ctx_new_alloc (ctx);

	int server_fd  = ctx->pollfd[index].fd;
	int *client_fd = &ctx->pollfd[ctx->size -1].fd;

	TEST_IPC_RR (usock_accept (server_fd, client_fd), "cannot accept IPC connection");
	ctx->pollfd[ctx->size -1].events = POLLIN; // Tell to poll(2) to watch for incoming data from this fd.
	ctx->cinfos[ctx->size -1].type = IPC_CONNECTION_TYPE_IPC;

	// Set the event structure.
	uint32_t client_index = ctx->size - 1;
	IPC_EVENT_SET (event, IPC_EVENT_TYPE_CONNECTION, client_index, *client_fd, NULL);

	IPC_RETURN_NO_ERROR;
}

// receive then format in an ipc_message structure
struct ipc_error ipc_read_fd (int32_t fd, struct ipc_message *m)
{
	T_R ((m == NULL), IPC_ERROR_READ__NO_MESSAGE_PARAM);

	size_t msize = IPC_MAX_MESSAGE_SIZE;
	SECURE_BUFFER_DECLARATION (char, buf, msize);
	char *pbuf = buf;

	// On error or closed recipient, the buffer already freed.
	TEST_IPC_RETURN_ON_ERROR (usock_recv (fd, &pbuf, &msize));
	TEST_IPC_RETURN_ON_ERROR (ipc_message_format_read (m, buf, msize));

	IPC_RETURN_NO_ERROR;	// propagates ipc_message_format return
}

struct ipc_error ipc_read (const struct ipc_ctx *ctx, uint32_t index, struct ipc_message *m)
{
	return ipc_read_fd (ctx->pollfd[index].fd, m);
}

struct ipc_error ipc_write_fd (int fd, const struct ipc_message *m)
{
	size_t msize = 0;
	SECURE_BUFFER_DECLARATION (char, buf, IPC_MAX_MESSAGE_SIZE);
	char *pbuf = buf;

	ipc_message_format_write (m, &pbuf, &msize);

	size_t nbytes_sent = 0;
	TEST_IPC_RETURN_ON_ERROR (usock_send (fd, buf, msize, &nbytes_sent));

	// what was sent != what should have been sent
	T_R ((nbytes_sent != msize), IPC_ERROR_WRITE_FD__NOT_ENOUGH_DATA);

	IPC_RETURN_NO_ERROR;
}

// Put the message in the list of messages to send.
struct ipc_error ipc_write (struct ipc_ctx *ctx, const struct ipc_message *m)
{
	int found = 0;
	for (size_t i = 0; i < ctx->size; i++) {
		if (ctx->pollfd[i].fd == m->fd) {
			ctx->pollfd[i].events |= POLLOUT;
			found = 1;
		}
	}

	T_R ((found == 0), IPC_ERROR_WRITE__FD_NOT_FOUND);

	// Performs a deep copy of the structure.
	return ipc_messages_add (&ctx->tx, m);
}

/**
 * Allocate memory then add a new connection to the context.
 */
struct ipc_error ipc_add (struct ipc_ctx *ctx, struct ipc_connection_info *p, struct pollfd *pollfd)
{
	T_R ((ctx    == NULL), IPC_ERROR_ADD__NO_PARAM_CLIENTS);
	T_R ((p      == NULL), IPC_ERROR_ADD__NO_PARAM_CLIENT);
	T_R ((pollfd == NULL), IPC_ERROR_ADD__NO_PARAM_POLLFD);

	// Memory reallocation.
	ipc_ctx_new_alloc (ctx);

	T_R ((ctx->size <= 0), IPC_ERROR_ADD__NOT_ENOUGH_MEMORY);

	ctx->cinfos[ctx->size - 1] = *p;
	ctx->pollfd[ctx->size - 1] = *pollfd;

	IPC_RETURN_NO_ERROR;
}

struct ipc_error ipc_del (struct ipc_ctx *ctx, uint32_t index)
{
	T_R ((ctx == NULL),                                IPC_ERROR_DEL__NO_CLIENTS_PARAM);
	T_R ((ctx->cinfos == NULL || ctx->pollfd == NULL), IPC_ERROR_DEL__EMPTY_LIST);
	T_R ((index >= ctx->size), IPC_ERROR_DEL__CANNOT_FIND_CLIENT);

	if (ctx->cinfos[index].spath != NULL) {
		free (ctx->cinfos[index].spath);
		ctx->cinfos[index].spath = NULL;
	}

	ipc_remove_messages_for_fd (ctx, ctx->pollfd[index].fd);

	ctx->size--;

	if (ctx->size == 0) {
		// free ctx->cinfos and ctx->pollfd
		ipc_ctx_free (ctx);
		IPC_RETURN_NO_ERROR;
	}

	// The last element in the array replaces the removed one.
	ctx->cinfos[index] = ctx->cinfos[ctx->size];
	ctx->pollfd[index] = ctx->pollfd[ctx->size];

	// Reallocation of the arrays. TODO: should be optimised someday.
	ctx->cinfos = realloc (ctx->cinfos, sizeof (struct ipc_connection_info) * ctx->size);
	ctx->pollfd = realloc (ctx->pollfd, sizeof (struct pollfd             ) * ctx->size);

	if (ctx->cinfos == NULL || ctx->pollfd == NULL) {
		IPC_RETURN_ERROR (IPC_ERROR_DEL__EMPTIED_LIST);
	}

	IPC_RETURN_NO_ERROR;
}

struct ipc_error ipc_add_fd_ (struct ipc_ctx *ctx, int fd, enum ipc_connection_type type)
{
	T_R ((ctx == NULL), IPC_ERROR_ADD_FD__NO_PARAM_CINFOS);

	SECURE_DECLARATION (struct ipc_connection_info, cinfo);
	cinfo.type = type;

	SECURE_DECLARATION (struct pollfd, pollfd);
	pollfd.fd = fd;
	pollfd.events = POLLIN;

	return ipc_add (ctx, &cinfo, &pollfd);
}

// add a switched file descriptor to read
struct ipc_error ipc_add_fd_switched (struct ipc_ctx *ctx, int fd)
{
	return ipc_add_fd_ (ctx, fd, IPC_CONNECTION_TYPE_SWITCHED);
}

// add an arbitrary file descriptor to read
struct ipc_error ipc_add_fd (struct ipc_ctx *ctx, int fd)
{
	return ipc_add_fd_ (ctx, fd, IPC_CONNECTION_TYPE_EXTERNAL);
}

// remove a connection from its file descriptor
struct ipc_error ipc_del_fd (struct ipc_ctx *ctx, int fd)
{
	T_R ((ctx == NULL),                                IPC_ERROR_DEL_FD__NO_PARAM_CINFOS);
	T_R ((ctx->cinfos == NULL || ctx->pollfd == NULL), IPC_ERROR_DEL_FD__EMPTY_LIST);

	for (size_t i = 0; i < ctx->size; i++) {
		if (ctx->pollfd[i].fd == fd) {
			return ipc_del (ctx, i);
		}
	}

	IPC_RETURN_ERROR (IPC_ERROR_DEL_FD__CANNOT_FIND_CLIENT);
}

struct ipc_error handle_writing_message (struct ipc_event *event, struct ipc_ctx *ctx, uint32_t index)
{
	int txfd = ctx->pollfd[index].fd;
	int mfd;
	struct ipc_message *m;
	for (size_t i = 0; ctx->tx.size ; i++) {
		m = &ctx->tx.messages[i];
		mfd = m->fd;
		if (txfd == mfd) {
			// In case the writing is compromised, do not return right away,
			// just print the result.
			TEST_IPC_P(ipc_write_fd (txfd, m), "cannot send a message to the client");

			// Freeing the message structure.
			ipc_message_empty (m);
			// Removing the message from the context.
			ipc_messages_del (&ctx->tx, i); // remove the message indexed by i

			break; // The message has been sent
		}
	}

	IPC_EVENT_SET (event, IPC_EVENT_TYPE_TX, index, ctx->pollfd[index].fd, NULL);
	IPC_RETURN_NO_ERROR;
}

struct ipc_error handle_new_message (struct ipc_event *event, struct ipc_ctx *ctx, int index)
{
	SECURE_DECLARATION (struct ipc_error, ret);

	// Listen to what they have to say (disconnection or message)
	// then add a client to `event`, the ipc_event structure.
	struct ipc_message *m = NULL;
	SECURE_BUFFER_HEAP_ALLOCATION_R (m, sizeof (struct ipc_message),, IPC_ERROR_HANDLE_MESSAGE__NOT_ENOUGH_MEMORY);

	// current talking client
	ret = ipc_read (ctx, index, m);
	if (ret.error_code != IPC_ERROR_NONE && ret.error_code != IPC_ERROR_CLOSED_RECIPIENT) {
		struct ipc_error rvalue = ret;	// store the final return value
		ipc_message_empty (m);
		free (m);

		// if there is a problem, just remove the client
		TEST_IPC_P (ipc_close (ctx, index), "cannot close a connection in handle_message");
		TEST_IPC_P (ipc_del   (ctx, index), "cannot delete a connection in handle_message");

		IPC_EVENT_SET (event, IPC_EVENT_TYPE_ERROR, index, ctx->pollfd[index].fd, NULL);
		return rvalue;
	}

	// disconnection: close the client then delete it from ctx
	if (ret.error_code == IPC_ERROR_CLOSED_RECIPIENT) {

		IPC_EVENT_SET (event, IPC_EVENT_TYPE_DISCONNECTION, index, ctx->pollfd[index].fd, NULL);

		ipc_message_empty (m);
		free (m);

		TEST_IPC_P (ipc_close (ctx, index), "cannot close a connection on closed recipient in handle_message");
		TEST_IPC_P (ipc_del   (ctx, index), "cannot delete a connection on closed recipient in handle_message");

		// warning: do not forget to free the ipc_client structure
		IPC_RETURN_NO_ERROR;
	}

	// The message carries the fd it was received on.
	m->fd = ctx->pollfd[index].fd;
	IPC_EVENT_SET (event, IPC_EVENT_TYPE_MESSAGE, index, ctx->pollfd[index].fd, m);
	IPC_RETURN_NO_ERROR;
}

struct ipc_error
handle_writing_switched_message (struct ipc_event *event, struct ipc_ctx *ctx, uint32_t index)
{
	return fd_switching_write (event, ctx, index);
}

struct ipc_error
handle_switched_message(struct ipc_event *event, struct ipc_ctx *ctx, uint32_t index)
{
	return fd_switching_read (event, ctx, index);
}

/* timer is in ms */
struct ipc_error ipc_wait_event (struct ipc_ctx *ctx, struct ipc_event *event, int *timer)
{
	T_R ((ctx == NULL), IPC_ERROR_WAIT_EVENT__NO_CLIENTS_PARAM);
	T_R ((event  == NULL), IPC_ERROR_WAIT_EVENT__NO_EVENT_PARAM);

	IPC_EVENT_CLEAN (event);

	// By default, everything is alright.
	SECURE_DECLARATION(struct ipc_error, final_return);
	final_return.error_code = IPC_ERROR_NONE;

	int32_t n = 0;

	for (size_t i = 0; i < ctx->size; i++) {
		// We assume that any fd in the list has to be listen to.
		ctx->pollfd[i].events = POLLIN;
	}

	// For each message to send…
	for (size_t i = 0; i < ctx->tx.size; i++) {
		// … verify that its destination is available for message exchange.
		for (size_t y = 0; y < ctx->size; y++) {
			if (ctx->pollfd[y].fd == ctx->tx.messages[i].fd) {
				ctx->pollfd[y].events |= POLLOUT;
			}
		}
	}

	struct timeval tv_1;
	memset (&tv_1, 0, sizeof(struct timeval));

	struct timeval tv_2;
	memset (&tv_2, 0, sizeof(struct timeval));

	gettimeofday(&tv_1, NULL);

	int timer_ = *timer;

	/* In case there is a file descriptor that requires more to read. */
	for (size_t i = 0; i < ctx->size; i++) {
		if (ctx->cinfos[i].more_to_read == 1) {
			// printf ("There is more to read for _at least_ fd %d\n", ctx->pollfd[i].fd);
			timer_ = 0;
			break;
		}
	}

	if ((n = poll(ctx->pollfd, ctx->size, timer_)) < 0) {
		IPC_RETURN_ERROR (IPC_ERROR_WAIT_EVENT__POLL);
	}

	gettimeofday(&tv_2, NULL);

	int nb_sec_ms       = (tv_2.tv_sec  - tv_1.tv_sec) * 1000;
	int nb_usec_ms      = (tv_2.tv_usec - tv_1.tv_usec) / 1000;
	int time_elapsed_ms = (nb_sec_ms + nb_usec_ms);

	// Handle memory fuckery, 'cause low level programming is fun.
	if (time_elapsed_ms >= *timer) {
		*timer = 0;
	}
	else {
		*timer -= time_elapsed_ms;
	}

	// Timeout.
	if (n == 0 && timer_ != 0) {
		IPC_EVENT_SET (event, IPC_EVENT_TYPE_TIMER, 0, 0, NULL);
		IPC_RETURN_NO_ERROR;
	}

	for (size_t i = 0; i < ctx->size; i++) {

		// Whatever happens, we have the fd and the index in event.
		event->index  = i;
		event->origin = ctx->pollfd[i].fd;

		// Something to read or connection.
		if (ctx->pollfd[i].revents & POLLIN || ctx->cinfos[i].more_to_read == 1) {

			// Avoiding loops.
			ctx->cinfos[i].more_to_read = 0;

			// In case there is something to read for the server socket: new client.
			if (ctx->cinfos[i].type == IPC_CONNECTION_TYPE_SERVER) {
				final_return = ipc_accept_add (event, ctx, i);
				goto wait_event_exit;
			}

			// fd is switched: using callbacks for IO operations.
			if (ctx->cinfos[i].type == IPC_CONNECTION_TYPE_SWITCHED) {
				final_return = handle_switched_message (event, ctx, i);
				goto wait_event_exit;
			}

			// No treatment of the socket if external socket: the libipc user should handle IO operations.
			if (ctx->cinfos[i].type == IPC_CONNECTION_TYPE_EXTERNAL) {
				IPC_EVENT_SET (event, IPC_EVENT_TYPE_EXTRA_SOCKET, i, ctx->pollfd[i].fd, NULL);
				// Default: return no error.
				goto wait_event_exit;
			}

			final_return = handle_new_message (event, ctx, i);
			goto wait_event_exit;
		}

		// Something can be sent.
		if (ctx->pollfd[i].revents & POLLOUT) {
			ctx->pollfd[i].events &= ~POLLOUT;

			// fd is switched: using callbacks for IO operations.
			if (ctx->cinfos[i].type == IPC_CONNECTION_TYPE_SWITCHED) {
				final_return = handle_writing_switched_message (event, ctx, i);
				goto wait_event_exit;
			}

			final_return = handle_writing_message (event, ctx, i);
			goto wait_event_exit;
		}

		// Disconnection.
		if (ctx->pollfd[i].revents & POLLHUP) {
			/** IPC_EVENT_SET: event, type, index, fd, message */
			IPC_EVENT_SET (event, IPC_EVENT_TYPE_DISCONNECTION, i, ctx->pollfd[i].fd, NULL);
			final_return = ipc_close (ctx, i);
			goto wait_event_exit;
		}

		if (ctx->pollfd[i].revents & POLLERR) {
			printf ("POLLERR: PROBLEM WITH fd %d\n", ctx->pollfd[i].fd);
			IPC_EVENT_SET (event, IPC_EVENT_TYPE_ERROR, i, ctx->pollfd[i].fd, NULL);
			goto wait_event_exit;
		}

		if (ctx->pollfd[i].revents & POLLNVAL) {
			printf ("POLLNVAL: INVALID fd %d\n", ctx->pollfd[i].fd);
			IPC_EVENT_SET (event, IPC_EVENT_TYPE_ERROR, i, ctx->pollfd[i].fd, NULL);
			goto wait_event_exit;
		}

	} /** for loop: end of the message handling */

	printf ("END OF THE LOOP WITHOUT GOTO!\n");

wait_event_exit:

	/** TODO: tests on event, it has to be filled. */
	if (event->type == 0) {
		printf ("EVENT TYPE NOT FILLED! code: %d, error_message: %s\n"
			, final_return.error_code
			, final_return.error_message);
	}

	return final_return;
}
