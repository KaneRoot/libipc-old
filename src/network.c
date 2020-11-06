#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <time.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>

#include "ipc.h"

/**
 * TODO:
 * describe a protocol to get this working into ipcd
 *   asking ipcd for a fd with an URI
 *     URI should contain: who (the service name), where (destination), how (protocol)
 *   ipcd initiates a communication with the requested service
 *   ipcd sends the fd
 * get a ipcd working with this
 */

struct ipc_error ipc_receive_fd (int sock, int *fd)
{
	T_R ((fd == NULL), IPC_ERROR_RECEIVE_FD__NO_PARAM_FD);
	*fd = -1;

	SECURE_DECLARATION (struct msghdr, msg);
	SECURE_BUFFER_DECLARATION (char, c_buffer, 256);

	/* On Mac OS X, the struct iovec is needed, even if it points to minimal data */
	char m_buffer[1];
	struct iovec io = {.iov_base = m_buffer,.iov_len = sizeof (m_buffer) };
	msg.msg_iov = &io;
	msg.msg_iovlen = 1;

	msg.msg_control = c_buffer;
	msg.msg_controllen = sizeof (c_buffer);

	T_PERROR_RIPC ((recvmsg (sock, &msg, 0) <= 0), "recvmsg", IPC_ERROR_RECEIVE_FD__RECVMSG);

	struct cmsghdr *cmsg = CMSG_FIRSTHDR (&msg);

	memmove (fd, CMSG_DATA (cmsg), sizeof (*fd));

	IPC_RETURN_NO_ERROR;
}

struct ipc_error ipc_provide_fd (int sock, int fd)
{
	SECURE_DECLARATION (struct msghdr, msg);
	SECURE_BUFFER_DECLARATION (char, buf, CMSG_SPACE (sizeof (fd)));

	/* On Mac OS X, the struct iovec is needed, even if it points to minimal data */
	struct iovec io = {.iov_base = "",.iov_len = 1 };

	msg.msg_iov = &io;
	msg.msg_iovlen = 1;
	msg.msg_control = buf;
	msg.msg_controllen = sizeof (buf);

	struct cmsghdr *cmsg = CMSG_FIRSTHDR (&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN (sizeof (fd));

	memmove (CMSG_DATA (cmsg), &fd, sizeof (fd));

	msg.msg_controllen = cmsg->cmsg_len;

	T_PERROR_RIPC ((sendmsg (sock, &msg, 0) < 0), "sendmsg", IPC_ERROR_PROVIDE_FD__SENDMSG);

	IPC_RETURN_NO_ERROR;
}

void ipc_ctx_switching_add (struct ipc_ctx *ctx, int orig, int dest)
{
	ipc_switching_add (&ctx->switchdb, orig, dest);
}

void ipc_switching_add (struct ipc_switchings *is, int orig, int dest)
{
	// printf ("ipc_switching_add START: switchdb has %ld entries\n", is->size);

	if (is->collection == NULL) {
		// printf ("switchdb collection is null\n");
		is->collection = malloc (sizeof (struct ipc_switching) * (is->size + 1));
	}
	else {
		// printf ("switchdb collection isn't null\n");
		is->collection = realloc (is->collection, sizeof (struct ipc_switching) * (is->size + 1));
	}

	/** TODO: less brutal approach */
	if (is->collection == NULL) {
		fprintf (stderr, __FILE__ " error realloc line %d", __LINE__);
		exit (EXIT_FAILURE);
	}

	is->size++;

	is->collection[is->size - 1].orig = orig;
	is->collection[is->size - 1].dest = dest;

	is->collection[is->size - 1].orig_in  = NULL;
	is->collection[is->size - 1].dest_in  = NULL;
	is->collection[is->size - 1].orig_out = NULL;
	is->collection[is->size - 1].dest_out = NULL;

	// printf ("ipc_switching_add END: switchdb has %ld entries\n", is->size);
}

int ipc_ctx_switching_del (struct ipc_ctx *ctx, int fd)
{
	return ipc_switching_del (&ctx->switchdb, fd);
}

int ipc_switching_del (struct ipc_switchings *is, int fd)
{
	for (size_t i = 0; i < is->size; i++) {
		if (is->collection[i].orig == fd || is->collection[i].dest == fd) {
			int ret;

			if (fd == is->collection[i].orig) {
				ret = is->collection[i].dest;
			} else {
				ret = is->collection[i].orig;
			}

			is->collection[i].orig = is->collection[is->size - 1].orig;
			is->collection[i].dest = is->collection[is->size - 1].dest;

			if (is->size == 1) {
				free(is->collection);
				is->collection = NULL;
			}
			else {
				is->collection = realloc (is->collection, sizeof (struct ipc_switching) * (is->size-1));
			}

			is->size--;
			return ret;
		}
	}

	return -1;
}

/**
 * 0  = fd is origin
 * 1  = fd is dest
 * -1 = not found
 */
int ipc_switching_get_ (const struct ipc_switchings *is
	, int fd
	, struct ipc_switching **s)
{
	for (size_t i = 0; i < is->size; i++) {
		if (is->collection[i].orig == fd) {
			*s = &is->collection[i];
			return 0;
		} else if (is->collection[i].dest == fd) {
			*s = &is->collection[i];
			return 1;
		}
	}

	return -1;
}

int ipc_switching_get (struct ipc_switchings *is, int fd)
{
	for (size_t i = 0; i < is->size; i++) {
		if (is->collection[i].orig == fd) {
			return is->collection[i].dest;
		} else if (is->collection[i].dest == fd) {
			return is->collection[i].orig;
		}
	}

	return -1;
}

void ipc_switching_free (struct ipc_switchings *is)
{
	if (is == NULL)
		return;

	if (is->collection != NULL) {
		free (is->collection);
		is->collection = NULL;
	}
	is->size = 0;
}

enum ipccb
default_cb_in(int fd, struct ipc_message *m, short int *more_to_read)
{
	*more_to_read = 0;

	size_t msize = IPC_MAX_MESSAGE_SIZE;
	SECURE_BUFFER_DECLARATION (char, buf, msize);
	char *pbuf = buf;

	// By default, usock_read (a wrapper around read(2)) is used.

	{   /** Some macros use "ret" as a variable name, so this is to be sure. */
		struct ipc_error ret = usock_recv (fd, &pbuf, &msize);
		if (ret.error_code != IPC_ERROR_NONE) {
			if (ret.error_code == IPC_ERROR_CLOSED_RECIPIENT) {
				return IPC_CB_FD_CLOSING;
			}
			return IPC_CB_FD_ERROR;
		}
	}

	/** There is a message, send it to the corresponding fd **/
	if (msize > 0) {
		struct ipc_error ret = ipc_message_format_read (m, buf, msize);
		if (ret.error_code != IPC_ERROR_NONE) {
			return IPC_CB_PARSING_ERROR;
		}
		return IPC_CB_NO_ERROR;
	}

	// By default, if msize <= 0 the fd should be closed.
	return IPC_CB_FD_CLOSING;
}

enum ipccb
default_cb_out(int fd, struct ipc_message *m)
{
	size_t msize = 0;
	SECURE_DECLARATION (struct ipc_error, ret);
	SECURE_BUFFER_DECLARATION (char, buf, IPC_MAX_MESSAGE_SIZE);
	char *pbuf = buf;

	ipc_message_format_write (m, &pbuf, &msize);

	size_t nbytes_sent = 0;
	ret = usock_send (fd, buf, msize, &nbytes_sent);

	// On error or if what was sent != what should have been sent.
	if (ret.error_code != IPC_ERROR_NONE || nbytes_sent != msize) {
		return IPC_CB_FD_ERROR;
	}

	return IPC_CB_NO_ERROR;
}

void ipc_switching_callbacks_ (struct ipc_ctx *ctx, int fd
	, enum ipccb (*cb_in )(int fd, struct ipc_message *m, short int *more_to_read))
{
	ipc_switching_callbacks (ctx, fd, cb_in, NULL);
}

void ipc_switching_callbacks (
	  struct ipc_ctx *ctx
	, int fd
	, enum ipccb (*cb_in )(int fd, struct ipc_message *m, short int *more_to_read)
	, enum ipccb (*cb_out)(int fd, struct ipc_message *m))
{
	struct ipc_switching *sw = NULL;
	int is_valid = ipc_switching_get_ (&ctx->switchdb, fd, &sw);
	if (is_valid == -1) {
		return;
	}

	if (sw->orig == fd) {
		sw->orig_in  = cb_in;
		sw->orig_out = cb_out;
	}
	else {
		sw->dest_in  = cb_in;
		sw->dest_out = cb_out;
	}
}

/**
 * fd_switching_read allows to read a message from a switched fd.
 */
struct ipc_error fd_switching_read (struct ipc_event *event, struct ipc_ctx *ctx, int index)
{
	// printf ("fd_switching_read\n");

	// If the socket is associated to another one for ipcd:
	// read and write automatically and provide a new IPC_EVENT_TYPE indicating the switch.
	T_R ((ctx->switchdb.size == 0), IPC_ERROR_FD_SWITCHING__NO_FD_RECORD);

	int talkingfd = ctx->pollfd[index].fd;
	int dest_fd = -1;
	struct ipc_switching *sw = NULL;
	struct ipc_message m;
	memset(&m, 0, sizeof (struct ipc_message));

	enum ipccb r;
	int is_valid = 0;
	short int more_to_read = 0;

	is_valid = ipc_switching_get_ (&ctx->switchdb, talkingfd, &sw);

	T_R ((is_valid == -1), IPC_ERROR_FD_SWITCHING__NO_FD_RECORD);

	if (sw->orig == talkingfd) {
		dest_fd = sw->dest;
		if (sw->orig_in == NULL) {
			r = default_cb_in (talkingfd, &m, &more_to_read);
		}
		else {
			r = (*sw->orig_in)(talkingfd, &m, &more_to_read);
		}
	}
	else {
		dest_fd = sw->orig;
		if (sw->dest_in == NULL) {
			r = default_cb_in (talkingfd, &m, &more_to_read);
		}
		else {
			r = (*sw->dest_in)(talkingfd, &m, &more_to_read);
		}
	}

	ctx->cinfos[index].more_to_read = more_to_read;

	// Message reception OK: reading the message and put it in the list of messages to send.
	if (r == IPC_CB_NO_ERROR) {
		// In case of message reception:
		// 1. put the message in the list to be sent
		m.fd = dest_fd;
		ipc_write (ctx, &m);
		// 2. delete the message (a deep copy has been made)
		ipc_message_empty (&m);
		// 3. set event IPC_EVENT_TYPE_SWITCH, inform ipcd of a successful reception.
		IPC_EVENT_SET (event, IPC_EVENT_TYPE_SWITCH, index, ctx->pollfd[index].fd, NULL);
		// 4. IPC_RETURN_NO_ERROR
		IPC_RETURN_NO_ERROR;
	}

	// Message reception OK: no message to transfer.
	// This is applied to protocol-specific messages, for example when the client
	// has to communicate with the proxy, not the service.
	if (r == IPC_CB_IGNORE) {
		// printf ("IGNORING REQUEST\n");
		// In case of message reception:
		// 1. set event IPC_EVENT_TYPE_SWITCH, inform ipcd of a successful reception.
		IPC_EVENT_SET (event, IPC_EVENT_TYPE_SWITCH, index, ctx->pollfd[index].fd, NULL);
		// 2. IPC_RETURN_NO_ERROR
		IPC_RETURN_NO_ERROR;
	}

	/**
	 * NOTE: In any other case, the fd is, or should be closed.
	 */

	// 1. remove both fd from switchdb
	// Client and servers should be closed by the libipc user application.
	// close (sw->dest);
	// close (talkingfd);

	ipc_del_fd (ctx, sw->dest);
	ipc_del_fd (ctx, talkingfd);

	ipc_switching_del (&ctx->switchdb, talkingfd);

	// 2. set event (either error or disconnection)
	if (r == IPC_CB_FD_CLOSING) {
		IPC_EVENT_SET (event, IPC_EVENT_TYPE_DISCONNECTION, index, talkingfd, NULL);
	}
	else {
		IPC_EVENT_SET (event, IPC_EVENT_TYPE_ERROR, index, talkingfd, NULL);
	}

	// 3. return IPC_ERROR_CLOSED_RECIPIENT
	IPC_RETURN_ERROR (IPC_ERROR_CLOSED_RECIPIENT);
}

/**
 * fd_switching_write allows to read a message from a switched fd.
 */
struct ipc_error fd_switching_write (struct ipc_event *event, struct ipc_ctx *ctx, int index)
{
	// printf ("fd_switching_write\n");

	// If the socket is associated to another one for ipcd:
	// read and write automatically and provide a new IPC_EVENT_TYPE indicating the switch.
	T_R ((ctx->switchdb.size == 0), IPC_ERROR_FD_SWITCHING__NO_FD_RECORD);

	int output_fd = ctx->pollfd[index].fd;
	struct ipc_switching *sw = NULL;
	struct ipc_message *m = NULL;
	size_t i;

	// search for the next message to send for output_fd fd.
	for (i = 0; ctx->tx.size ; i++) {
		if (ctx->tx.messages[i].fd == output_fd) {
			m = &ctx->tx.messages[i];
			break;
		}
	}

	// In case there is no message for the fd: the error will be catched.

	enum ipccb r;
	int is_valid = 0;

	is_valid = ipc_switching_get_ (&ctx->switchdb, output_fd, &sw);

	T_R ((is_valid == -1), IPC_ERROR_FD_SWITCHING__NO_FD_RECORD);

	if (sw->orig == output_fd) {
		if (sw->orig_in == NULL) {
			r = default_cb_out (output_fd, m);
		}
		else {
			r = (*sw->orig_out)(output_fd, m);
		}
	}
	else {
		if (sw->dest_in == NULL) {
			r = default_cb_out (output_fd, m);
		}
		else {
			r = (*sw->dest_out)(output_fd, m);
		}
	}

	// Whether or not the message has been sent, it should be removed.
	// Freeing the message structure.
	ipc_message_empty (m);
	// Removing the message from the context.
	ipc_messages_del (&ctx->tx, i); // remove the message indexed by i

	// Message reception OK: reading the message and put it in the list of messages to send.
	if (r == IPC_CB_NO_ERROR) {
		// 1. set event IPC_EVENT_TYPE_SWITCH, inform ipcd of a successful reception.
		IPC_EVENT_SET (event, IPC_EVENT_TYPE_TX, index, output_fd, NULL);
		// 2. IPC_RETURN_NO_ERROR
		IPC_RETURN_NO_ERROR;
	}

	/**
	 * NOTE: In any other case, the fd is, or should be closed.
	 */

	// 1. close and remove both fd from switchdb
	int delfd = ipc_switching_del (&ctx->switchdb, output_fd);
	if (delfd >= 0) {
		close (delfd);
		ipc_del_fd (ctx, delfd);
	}
	close (output_fd);
	ipc_del_fd (ctx, output_fd);

	// 2. set event (either error or disconnection)
	if (r == IPC_CB_FD_CLOSING) {
		IPC_EVENT_SET (event, IPC_EVENT_TYPE_DISCONNECTION, index, output_fd, NULL);
	}
	else {
		IPC_EVENT_SET (event, IPC_EVENT_TYPE_ERROR, index, output_fd, NULL);
	}

	// 3. return IPC_ERROR_CLOSED_RECIPIENT
	IPC_RETURN_ERROR (IPC_ERROR_CLOSED_RECIPIENT);
}
