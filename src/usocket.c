#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "usocket.h"
#include "utils.h"

// exists_, basename_, is_directory_, have_rights_
#include "fs.h"

// #define IPC_DEBUG 3

struct ipc_error usock_send (const int32_t fd, const char *buf, size_t len, size_t * sent)
{
	ssize_t ret = 0;
	ret = send (fd, buf, len, MSG_NOSIGNAL);
	T_R ((ret <= 0), IPC_ERROR_USOCK_SEND);
	*sent = ret;
	IPC_RETURN_NO_ERROR;
}

// *len is changed to the total message size read (header + payload)
struct ipc_error usock_recv (const int32_t fd, char **buf, size_t * len)
{
	T_R ((buf == NULL), IPC_ERROR_USOCK_RECV__NO_BUFFER);
	T_R ((len == NULL), IPC_ERROR_USOCK_RECV__NO_LENGTH);

	// printf("USOCKET: listen to %d (up to %lu bytes)\n", fd, *len);
	int32_t ret_recv = 0;

	if (*len == 0)
		*len = IPC_MAX_MESSAGE_SIZE;

	// msize_read: size of the message (without the header).
	uint32_t msize = 0;

	// msize_read: size sum of the packets received.
	uint32_t msize_read = 0;

	do {
		/**
		 * recv:
		 * ret >  0: message receveid
		 * ret == 0: fd is closing
		 * ret <  0: error
		 */
		ret_recv = recv (fd, *buf, *len, 0);

		if (ret_recv > 0) {
#ifdef IPC_DEBUG
			print_hexa ("msg recv", (uint8_t *) * buf, ret_recv);
			fflush (stdout);
#endif
			if (msize == 0) {
				memcpy (&msize, *buf + 1, sizeof msize);
				msize = ntohl (msize);
			}
			// else {
			// 	printf ("USOCKET: We received a message in (at least) two packets (receveid %u bytes).\n", msize_read);
			// }

			if (msize > IPC_MAX_MESSAGE_SIZE) {
#ifdef IPC_DEBUG
				print_hexa ("msg recv", (uint8_t *) * buf, ret_recv);
				fflush (stdout);
#endif
			}

			// Do not allow messages with a longer size than expected.
			T_R ((msize > IPC_MAX_MESSAGE_SIZE), IPC_ERROR_USOCK_RECV__MESSAGE_SIZE);

			msize_read += ret_recv;
		} else if (ret_recv < 0) {
			*len = 0;

			switch (errno) {

				// The receive buffer pointer(s) point outside the process's address space.
				ERROR_CASE (EFAULT, "usock_recv", "critical error: use of unallocated memory, quitting...");

				// Invalid argument passed.
				ERROR_CASE (EINVAL, "usock_recv", "critical error: invalid arguments to read(2), quitting...");

				// Could not allocate memory for recvmsg().
				ERROR_CASE (ENOMEM, "usock_recv", "critical error: cannot allocate memory, quitting...");

				// The argument sockfd is an invalid descriptor.
				ERROR_CASE (EBADF, "usock_recv", "critical error: invalid descriptor, quitting...");

				// The file descriptor sockfd does not refer to a socket.
				ERROR_CASE (ENOTSOCK, "usock_recv", "critical error: fd is not a socket, quitting...");

				// The socket is associated with a connection-oriented protocol and has not
				// been connected (see connect(2) and accept(2)).
				ERROR_CASE (ENOTCONN, "usock_recv", "critical error: read(2) on a non connected socket, quitting...");

				ERROR_CASE (EAGAIN, "usock_recv", "ERROR WHILE RECEIVING: EAGAIN / EWOULDBLOCK");

				// A remote host refused to allow the network connection
				// (typically because it is not running the requested service).
				ERROR_CASE (ECONNREFUSED, "usock_recv", "ERROR WHILE RECEIVING: ECONNREFUSED");

				// The receive was interrupted by delivery of a signal before
				// any data were available; see signal(7).
				ERROR_CASE (EINTR, "usock_recv", "unsupported error");

			default:
				IPC_RETURN_ERROR_FORMAT (IPC_ERROR_USOCK_RECV__UNRECOGNIZED_ERROR
					, "%s"
					, "usock_recv: unrecognized error after recv(2)");
			}

			IPC_RETURN_ERROR_FORMAT (IPC_ERROR_USOCK_RECV
				, "%s"
				, "usock_recv: recv < 0, is the message size malformed?");
		}

		// if (msize > msize_read) {
		// 	printf ("USOCKET: loop again for %d (read %u/%u)\n", fd, msize_read, msize);
		// }

		// In case msize still is 0, recv didn't worked as expected.
	} while (msize > 0 && msize > msize_read - IPC_HEADER_SIZE);

	// printf("USOCKET: end of the loop for client %d -- %u bytes read\n", fd, msize_read);
	*len = msize_read;

	// none bytes received, indicates a closed recipient
	if (ret_recv == 0) {
		*len = 0;
		IPC_RETURN_ERROR (IPC_ERROR_CLOSED_RECIPIENT);
	}

	IPC_RETURN_NO_ERROR;
}

struct ipc_error usock_connect (int32_t * fd, const char *path)
{
	T_R ((fd == NULL), IPC_ERROR_USOCK_CONNECT__WRONG_FILE_DESCRIPTOR);
	T_R ((path == NULL), IPC_ERROR_USOCK_CONNECT__EMPTY_PATH);

	SECURE_DECLARATION (struct sockaddr_un, my_addr);
	my_addr.sun_family = AF_UNIX;

	int32_t sfd;
	socklen_t peer_addr_size = sizeof (struct sockaddr_un);

	T_PERROR_RIPC (((sfd = socket (AF_UNIX, SOCK_SEQPACKET, 0)) == -1), "socket", IPC_ERROR_USOCK_CONNECT__SOCKET);
	strncpy (my_addr.sun_path, path, (strlen (path) < PATH_MAX) ? strlen (path) : PATH_MAX);

	TEST_IPC_RETURN_ON_ERROR(directory_setup_ (path));

	if (connect (sfd, (struct sockaddr *)&my_addr, peer_addr_size) == -1) {
		char *str_error = strerror (errno);
		SECURE_BUFFER_DECLARATION(char, error_message, BUFSIZ);
		snprintf (error_message, BUFSIZ
			, "unix socket connection to the path %s not possible:%s"
			, path, str_error);

		IPC_RETURN_ERROR_FORMAT (IPC_ERROR_USOCK_CONNECT__CONNECT, "%s", error_message);
	}

	*fd = sfd;

	IPC_RETURN_NO_ERROR;
}

struct ipc_error usock_init (int32_t * fd, const char *path)
{
	T_R ((fd == NULL), IPC_ERROR_USOCK_INIT__EMPTY_FILE_DESCRIPTOR);
	T_R ((path == NULL), IPC_ERROR_USOCK_INIT__EMPTY_PATH);

	SECURE_DECLARATION (struct sockaddr_un, my_addr);
	my_addr.sun_family = AF_UNIX;
	strncpy (my_addr.sun_path, path, strlen (path));

	int32_t sfd;
	socklen_t peer_addr_size;

	TEST_IPC_RETURN_ON_ERROR(directory_setup_ (path));

	T_PERROR_RIPC (((sfd = socket (AF_UNIX, SOCK_SEQPACKET, 0)) == -1)
		, "socket", IPC_ERROR_USOCK_INIT__WRONG_FILE_DESCRIPTOR);

	// delete the unix socket if already created
	// ignore otherwise
	usock_remove (path);

	peer_addr_size = sizeof (struct sockaddr_un);

	T_PERROR_RIPC ((bind (sfd, (struct sockaddr *)&my_addr, peer_addr_size) == -1)
		, "bind", IPC_ERROR_USOCK_INIT__BIND);
	T_PERROR_RIPC ((listen (sfd, LISTEN_BACKLOG) == -1), "listen", IPC_ERROR_USOCK_INIT__LISTEN);

	*fd = sfd;

	IPC_RETURN_NO_ERROR;
}

struct ipc_error usock_accept (int32_t fd, int32_t * pfd)
{
	T_R ((pfd == NULL), IPC_ERROR_USOCK_ACCEPT__PATH_FILE_DESCRIPTOR);

	SECURE_DECLARATION (struct sockaddr_un, peer_addr);
	socklen_t peer_addr_size = 0;

	T_PERROR_RIPC (((*pfd = accept (fd, (struct sockaddr *)&peer_addr, &peer_addr_size)) < 0)
		, "accept", IPC_ERROR_USOCK_ACCEPT);

	IPC_RETURN_NO_ERROR;
}

struct ipc_error usock_close (int32_t fd)
{
	T_PERROR_RIPC ((close (fd) < 0), "close", IPC_ERROR_USOCK_CLOSE);
	IPC_RETURN_NO_ERROR;
}

struct ipc_error usock_remove (const char *path)
{
	SECURE_DECLARATION (struct stat, file_state);

	// if file exists, remove it
	T_R ((stat (path, &file_state) != 0), IPC_ERROR_USOCK_REMOVE__NO_FILE);
	T_R ((unlink (path) != 0), IPC_ERROR_USOCK_REMOVE__UNLINK);

	IPC_RETURN_NO_ERROR;
}
