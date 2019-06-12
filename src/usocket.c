#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#include <arpa/inet.h>

#include "usocket.h"
#include "utils.h"

#define IPC_DEBUG

#undef handle_err
#define handle_err(a,b) fprintf (stderr, "FUNCTION %s LINE %d ERROR %s\n", a, __LINE__, b);

/**
 * TODO: non blocking read
 */

enum ipc_errors usock_send (const int32_t fd, const char *buf, ssize_t len, ssize_t *sent)
{
    ssize_t ret = 0;
    ret = send (fd, buf, len, MSG_NOSIGNAL);
    if (ret <= 0) {
        handle_err ("usock_send", "send ret <= 0");
		return IPC_ERROR_USOCK_SEND;
	}
	*sent = ret;
    return IPC_ERROR_NONE;
}

// *len is changed to the total message size read (header + payload)
enum ipc_errors usock_recv (const int32_t fd, char **buf, ssize_t *len)
{
    assert(buf != NULL);
    assert(len != NULL);

    int32_t ret_recv = 0;

    if (buf == NULL) {
        handle_err ("usock_recv", "buf == NULL");
        return IPC_ERROR_USOCK_RECV__NO_BUFFER;
    }

    if (len == NULL) {
        handle_err ("usock_recv", "len == NULL");
        return IPC_ERROR_USOCK_RECV__NO_LENGTH;
    }

    if (*buf == NULL) {
        // do not allocate too much memory
        if (*len > IPC_MAX_MESSAGE_SIZE) {
            handle_err ("usock_recv", "len > IPC_MAX_MESSAGE_SIZE");
            *len = IPC_MAX_MESSAGE_SIZE;
		}
        *buf = malloc (*len + IPC_HEADER_SIZE);
    }

	uint32_t msize = 0;
	uint32_t msize_read = 0;

	do {
		ret_recv = recv (fd, *buf, *len, 0);
		if (ret_recv > 0) {
			if (msize == 0) {
				memcpy (&msize, *buf + 1, sizeof msize);
			}
			else {
				printf ("pas la première boucle, msize == %u\n", msize);
			}
			msize = ntohl (msize);

			if (msize >= IPC_MAX_MESSAGE_SIZE) {
#ifdef IPC_DEBUG
				printf ("lecture bien passée : %d octets\n", ret_recv);
				print_hexa ("msg recv", (uint8_t *)*buf, ret_recv);
				fflush(stdout);
#endif
			}
			assert (msize < IPC_MAX_MESSAGE_SIZE);
			msize_read += ret_recv - IPC_HEADER_SIZE;

		}
		else if (ret_recv < 0) {
			if (*buf != NULL)
				free (*buf);
			*len = 0;

			switch (errno) {

				// The receive buffer pointer(s) point outside the process's address space.
				case EFAULT:
					handle_err ("usock_recv", "critical error: use of unallocated memory, quitting...");
					fprintf (stderr, "ERROR WHILE RECEIVING: EFAULT\n");
					break;

				// Invalid argument passed.
				case EINVAL:
					handle_err ("usock_recv", "critical error: invalid arguments to read(2), quitting...");
					fprintf (stderr, "ERROR WHILE RECEIVING: EINVAL\n");
					break;

				// Could not allocate memory for recvmsg().
				case ENOMEM:
					handle_err ("usock_recv", "critical error: cannot allocate memory, quitting...");
					fprintf (stderr, "ERROR WHILE RECEIVING: ENOMEM\n");
					break;

				// The argument sockfd is an invalid descriptor.
				case EBADF:
					handle_err ("usock_recv", "critical error: invalid descriptor, quitting...");
					fprintf (stderr, "ERROR WHILE RECEIVING: EBADF\n");
					break;

				// The file descriptor sockfd does not refer to a socket.
				case ENOTSOCK:
					handle_err ("usock_recv", "critical error: fd is not a socket, quitting...");
					fprintf (stderr, "ERROR WHILE RECEIVING: ENOTSOCK\n");
					break;

				// The socket is associated with a connection-oriented protocol and has not
				// been connected (see connect(2) and accept(2)).
				case ENOTCONN:
					handle_err ("usock_recv", "critical error: read(2) on a non connected socket, quitting...");
					fprintf (stderr, "ERROR WHILE RECEIVING: ENOTCONN\n");
					break;

				case EAGAIN:
					fprintf (stderr, "ERROR WHILE RECEIVING: EAGAIN / EWOULDBLOCK\n");
					break;

				// A remote host refused to allow the network connection
				// (typically because it is not running the requested service).
				case ECONNREFUSED:
					fprintf (stderr, "ERROR WHILE RECEIVING: ECONNREFUSED\n");
					break;

				// The receive was interrupted by delivery of a signal before
				// any data were available; see signal(7).
				case EINTR:
					handle_err ("usock_recv", "unsupported error");
					break;

				default:
					printf ("usock_recv unsupported error : %d\n", errno);
					handle_err ("usock_recv", "unsupported error");
			}

			handle_err ("usock_recv", "recv < 0");
			perror("recv");

			return IPC_ERROR_USOCK_RECV;
		}

#if defined(IPC_WITH_ERRORS) && IPC_WITH_ERRORS > 2
		printf ("fragmentation: message size read %u, should read %u\n", msize_read, msize);
#endif
	} while (msize > msize_read);

	*len = msize + IPC_HEADER_SIZE;

	// 1 on none byte received, indicates a closed recipient
	if (ret_recv == 0) {
		if (*buf != NULL) {
			free (*buf);
			*buf = NULL;
		}
		return IPC_ERROR_CLOSED_RECIPIENT;
	}

    // print_hexa ("msg recv", (uint8_t *)*buf, *len);
    // fflush(stdout);
    return IPC_ERROR_NONE;
}

enum ipc_errors usock_connect (int32_t *fd, const char *path)
{
    assert (fd != NULL);
    assert (path != NULL);

    if (fd == NULL) {
        handle_err ("usock_connect", "fd == NULL");
        return IPC_ERROR_USOCK_CONNECT__WRONG_FILE_DESCRIPTOR;
    }

    if (path == NULL) {
        handle_err ("usock_connect", "path == NULL");
        return IPC_ERROR_USOCK_CONNECT__EMPTY_PATH;
    }

    int32_t sfd;
    struct sockaddr_un my_addr;
    socklen_t peer_addr_size;

    sfd = socket (AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        handle_err ("usock_connect", "sfd == -1");
        return IPC_ERROR_USOCK_CONNECT__SOCKET;
    }

    // clear structure 
    memset(&my_addr, 0, sizeof(struct sockaddr_un));

    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, path, (strlen (path) < PATH_MAX) ? strlen(path) : PATH_MAX);

    peer_addr_size = sizeof(struct sockaddr_un);
    if(connect(sfd, (struct sockaddr *) &my_addr, peer_addr_size) == -1) {
        handle_err ("usock_connect", "connect == -1");
        perror("connect");
        return IPC_ERROR_USOCK_CONNECT__CONNECT;
    }

    *fd = sfd;

    return IPC_ERROR_NONE;
}

enum ipc_errors usock_init (int32_t *fd, const char *path)
{
    assert (fd != NULL);
    assert (path != NULL);

    if (fd == NULL) {
        handle_err ("usock_init", "fd == NULL");
        return IPC_ERROR_USOCK_INIT__EMPTY_FILE_DESCRIPTOR;
    }

    if (path == NULL) {
        handle_err ("usock_init", "path == NULL");
        return IPC_ERROR_USOCK_INIT__EMPTY_PATH;
    }

    int32_t sfd;
    struct sockaddr_un my_addr;
    socklen_t peer_addr_size;

    sfd = socket (AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        handle_err ("usock_init", "sfd == -1");
        return IPC_ERROR_USOCK_INIT__WRONG_FILE_DESCRIPTOR;
    }

    // clear structure 
    memset(&my_addr, 0, sizeof(struct sockaddr_un));

    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, path, strlen (path));

	// delete the unix socket if already created
	// ignore otherwise
	usock_remove (path);

    peer_addr_size = sizeof(struct sockaddr_un);

    if (bind (sfd, (struct sockaddr *) &my_addr, peer_addr_size) == -1) {
        handle_err ("usock_init", "bind == -1");
        perror("bind");
        return IPC_ERROR_USOCK_INIT__BIND;
    }

    if (listen (sfd, LISTEN_BACKLOG) == -1) {
        handle_err ("usock_init", "listen == -1");
        perror("listen");
        return IPC_ERROR_USOCK_INIT__LISTEN;
    }

    *fd = sfd;

    return IPC_ERROR_NONE;
}

enum ipc_errors usock_accept (int32_t fd, int32_t *pfd)
{
    assert (pfd != NULL);

    if (pfd == NULL) {
        handle_err ("usock_accept", "pfd == NULL");
        return IPC_ERROR_USOCK_ACCEPT__PATH_FILE_DESCRIPTOR;
    }

    struct sockaddr_un peer_addr;
    memset (&peer_addr, 0, sizeof (struct sockaddr_un));
    socklen_t peer_addr_size = 0;

    *pfd = accept (fd, (struct sockaddr *) &peer_addr, &peer_addr_size);
    if (*pfd < 0) {
        handle_err ("usock_accept", "accept < 0");
        perror("listen");
        return IPC_ERROR_USOCK_ACCEPT;
    }

    return IPC_ERROR_NONE;
}

enum ipc_errors usock_close (int32_t fd)
{
    int32_t ret = 0;

	ret = close (fd);
    if (ret < 0) {
        handle_err ("usock_close", "close ret < 0");
        perror ("closing");
		return IPC_ERROR_USOCK_CLOSE;
    }
    return IPC_ERROR_NONE;
}

enum ipc_errors usock_remove (const char *path)
{
	struct stat file_state;
	memset (&file_state, 0, sizeof (struct stat));

	// if file exists, remove it
	int ret = stat (path, &file_state);
	if (ret == 0) {
		ret = unlink (path);
		if (ret != 0) {
			return IPC_ERROR_USOCK_REMOVE__UNLINK;
		}
		return IPC_ERROR_NONE;
	}

	return IPC_ERROR_USOCK_REMOVE__NO_FILE;
}
