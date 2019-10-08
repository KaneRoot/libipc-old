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

// #define IPC_DEBUG 3

/**
 * TODO: non blocking read
 */

enum ipc_errors usock_send (const int32_t fd, const char *buf, size_t len, size_t *sent)
{
    ssize_t ret = 0;
    ret = send (fd, buf, len, MSG_NOSIGNAL);
    T_R ((ret <= 0), IPC_ERROR_USOCK_SEND);
	*sent = ret;
    return IPC_ERROR_NONE;
}

// *len is changed to the total message size read (header + payload)
enum ipc_errors usock_recv (const int32_t fd, char **buf, size_t *len)
{
    T_R ((buf == NULL), IPC_ERROR_USOCK_RECV__NO_BUFFER);
    T_R ((len == NULL), IPC_ERROR_USOCK_RECV__NO_LENGTH);

    int32_t ret_recv = 0;

	if (*len == 0)
		*len = IPC_MAX_MESSAGE_SIZE;

    if (*buf == NULL) {
        // do not allocate too much memory
        if (*len > IPC_MAX_MESSAGE_SIZE) {
            LOG_ERROR ("usock_recv: len > IPC_MAX_MESSAGE_SIZE");
            *len = IPC_MAX_MESSAGE_SIZE;
		}
		SECURE_BUFFER_HEAP_ALLOCATION (*buf,*len + IPC_HEADER_SIZE, , return (IPC_ERROR_NOT_ENOUGH_MEMORY));
    }

	uint32_t msize = 0;
	uint32_t msize_read = 0;

	do {
		ret_recv = recv (fd, *buf, *len, 0);
#ifdef IPC_DEBUG
		if (ret_recv > 0) {
			print_hexa ("msg recv", (uint8_t *)*buf, ret_recv);
			fflush(stdout);
		}
#endif

		if (ret_recv > 0) {
			if (msize == 0) {
				memcpy (&msize, *buf + 1, sizeof msize);
			}
			msize = ntohl (msize);

			if (msize >= IPC_MAX_MESSAGE_SIZE) {
#ifdef IPC_DEBUG
				print_hexa ("msg recv", (uint8_t *)*buf, ret_recv);
				fflush(stdout);
#endif
			}
			T_R ((msize > IPC_MAX_MESSAGE_SIZE), IPC_ERROR_USOCK_RECV__MESSAGE_SIZE);
			msize_read += ret_recv - IPC_HEADER_SIZE;
		}
		else if (ret_recv < 0) {
			if (*buf != NULL) {
				free (*buf);
				*buf = NULL;
			}
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
					LOG_ERROR ("usock_recv: unsupported error");
			}

			LOG_ERROR ("usock_recv: recv < 0");
			return IPC_ERROR_USOCK_RECV;
		}

#if 0
#if defined(IPC_WITH_ERRORS) && IPC_WITH_ERRORS > 2
		LOG_ERROR ("fragmentation: message size read %u, should read %u", msize_read, msize);
#endif
#endif
	} while (msize > msize_read);

	*len = msize + IPC_HEADER_SIZE;

	// 1 on none byte received, indicates a closed recipient
	if (ret_recv == 0) {
		if (*buf != NULL) {
			free (*buf);
			*buf = NULL;
		}
		*len = 0;
		return IPC_ERROR_CLOSED_RECIPIENT;
	}

    // print_hexa ("msg recv", (uint8_t *)*buf, *len);
    // fflush(stdout);
    return IPC_ERROR_NONE;
}

enum ipc_errors usock_connect (int32_t *fd, const char *path)
{
    T_R ((fd == NULL), IPC_ERROR_USOCK_CONNECT__WRONG_FILE_DESCRIPTOR);
    T_R ((path == NULL), IPC_ERROR_USOCK_CONNECT__EMPTY_PATH);

    SECURE_DECLARATION (struct sockaddr_un, my_addr);
    my_addr.sun_family = AF_UNIX;

    int32_t sfd;
    socklen_t peer_addr_size = sizeof(struct sockaddr_un);

    T_PERROR_R (((sfd = socket (AF_UNIX, SOCK_SEQPACKET, 0)) == -1), "socket", IPC_ERROR_USOCK_CONNECT__SOCKET);
    strncpy(my_addr.sun_path, path, (strlen (path) < PATH_MAX) ? strlen(path) : PATH_MAX);

	/** TODO: massive series of tests */
    T_PERROR_F_R (
			  /** test */         (connect(sfd, (struct sockaddr *) &my_addr, peer_addr_size) == -1)
			, /** perror */       "connect"
			, /** log error */    ("unix socket connection to the path %s not possible", path)
			, /** return value */ IPC_ERROR_USOCK_CONNECT__CONNECT);

    *fd = sfd;

    return IPC_ERROR_NONE;
}

enum ipc_errors usock_init (int32_t *fd, const char *path)
{
    T_R ((fd == NULL), IPC_ERROR_USOCK_INIT__EMPTY_FILE_DESCRIPTOR);
    T_R ((path == NULL), IPC_ERROR_USOCK_INIT__EMPTY_PATH);

    SECURE_DECLARATION (struct sockaddr_un, my_addr);
    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, path, strlen (path));

    int32_t sfd;
    socklen_t peer_addr_size;

    T_PERROR_R (((sfd = socket (AF_UNIX, SOCK_SEQPACKET, 0)) == -1), "socket", IPC_ERROR_USOCK_INIT__WRONG_FILE_DESCRIPTOR);

	// delete the unix socket if already created
	// ignore otherwise
	usock_remove (path);

    peer_addr_size = sizeof(struct sockaddr_un);

    T_PERROR_R ((bind (sfd, (struct sockaddr *) &my_addr, peer_addr_size) == -1), "bind", IPC_ERROR_USOCK_INIT__BIND);
    T_PERROR_R ((listen (sfd, LISTEN_BACKLOG) == -1), "listen", IPC_ERROR_USOCK_INIT__LISTEN);

    *fd = sfd;

    return IPC_ERROR_NONE;
}

enum ipc_errors usock_accept (int32_t fd, int32_t *pfd)
{
    T_R ((pfd == NULL), IPC_ERROR_USOCK_ACCEPT__PATH_FILE_DESCRIPTOR);

    SECURE_DECLARATION (struct sockaddr_un, peer_addr);
    socklen_t peer_addr_size = 0;

    T_PERROR_R (((*pfd = accept (fd, (struct sockaddr *) &peer_addr, &peer_addr_size)) < 0), "accept", IPC_ERROR_USOCK_ACCEPT);

    return IPC_ERROR_NONE;
}

enum ipc_errors usock_close (int32_t fd)
{
	T_PERROR_R ((close (fd) < 0), "close", IPC_ERROR_USOCK_CLOSE);
    return IPC_ERROR_NONE;
}

enum ipc_errors usock_remove (const char *path)
{
	SECURE_DECLARATION(struct stat, file_state);

	// if file exists, remove it
	T_R ((stat (path, &file_state) != 0), IPC_ERROR_USOCK_REMOVE__NO_FILE);
	T_R ((unlink (path) != 0), IPC_ERROR_USOCK_REMOVE__UNLINK);

	return IPC_ERROR_NONE;
}
