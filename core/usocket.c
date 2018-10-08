#include "usocket.h"
#include "utils.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

int usock_send (const int fd, const char *buf, ssize_t len, ssize_t *sent)
{
    ssize_t ret = 0;
    //printf ("%ld bytes to write\n", len);
    ret = send (fd, buf, len, 0);
    if (ret <= 0) {
        handle_err ("usock_send", "send ret <= 0");
		return -1;
	}
	*sent = ret;
    return 0;
}

int usock_recv (const int fd, char **buf, ssize_t *len)
{
    assert(buf != NULL);
    assert(len != NULL);

    ssize_t ret = 0;

    if (buf == NULL) {
        handle_err ("usock_recv", "buf == NULL");
        return -1;
    }

    if (len == NULL) {
        handle_err ("usock_recv", "len == NULL");
        return -1;
    }

    if (*buf == NULL) {
        // do not allocate too much memory
        if (*len > BUFSIZ)
            handle_err ("usock_recv", "len > BUFSIZ");
        if (*len == 0)
            *len = BUFSIZ;
        *buf = malloc ((*len < BUFSIZ) ? *len : BUFSIZ);
    }

    ret = recv (fd, *buf, *len, 0);
    if (ret < 0) {
		if (*buf != NULL)
			free (*buf);

        handle_err ("usock_recv", "recv ret < 0");
		perror("recv");
		*len = 0;

		switch (ret) {

			// The receive buffer pointer(s) point outside the process's address space.
			case EFAULT:
				handle_err ("usock_recv", "critical error: use of unallocated memory, quitting...");
				exit (1);

			// Invalid argument passed.
			case EINVAL:
				handle_err ("usock_recv", "critical error: invalid arguments to read(2), quitting...");
				exit (1);

			// Could not allocate memory for recvmsg().
			case ENOMEM:
				handle_err ("usock_recv", "critical error: cannot allocate memory, quitting...");
				exit (1);

			// The argument sockfd is an invalid descriptor.
			case EBADF:
				handle_err ("usock_recv", "critical error: invalid descriptor, quitting...");
				exit (1);

			// The file descriptor sockfd does not refer to a socket.
			case ENOTSOCK:
				handle_err ("usock_recv", "critical error: fd is not a socket, quitting...");
				exit (1);

			// The socket is associated with a connection-oriented protocol and has not
			// been connected (see connect(2) and accept(2)).
			case ENOTCONN:
				handle_err ("usock_recv", "critical error: read(2) on a non connected socket, quitting...");
				exit (1);

			// EWOULDBLOCK
			case EAGAIN:

			// A remote host refused to allow the network connection
			// (typically because it is not running the requested service).
			case ECONNREFUSED:

			// The receive was interrupted by delivery of a signal before
			// any data were available; see signal(7).
			case EINTR:

			default:
				handle_err ("usock_recv", "unsupported error");
				;
		}
        return -1;
    }

    *len = ret;

	// 1 on none byte received, indicates a closed recipient
	if (ret == 0) {
		if (*buf != NULL) {
			free (*buf);
			*buf = NULL;
		}
		return 1;
	}

    // print_hexa ("msg recv", (unsigned char *)*buf, *len);
    // fflush(stdout);
    return 0;
}

int usock_connect (int *fd, const char *path)
{
    assert (fd != NULL);
    assert (path != NULL);

    if (fd == NULL) {
        handle_err ("usock_connect", "fd == NULL");
        return -1;
    }

    if (path == NULL) {
        handle_err ("usock_connect", "path == NULL");
        return -1;
    }

    int sfd;
    struct sockaddr_un my_addr;
    socklen_t peer_addr_size;

    sfd = socket (AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        handle_err ("usock_connect", "sfd == -1");
        return -1;
    }

    // clear structure 
    memset(&my_addr, 0, sizeof(struct sockaddr_un));

    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, path, (strlen (path) < PATH_MAX) ? strlen(path) : PATH_MAX);

    peer_addr_size = sizeof(struct sockaddr_un);
    if(connect(sfd, (struct sockaddr *) &my_addr, peer_addr_size) == -1) {
        handle_err ("usock_connect", "connect == -1");
        perror("connect");
        exit(errno);
    }

    *fd = sfd;

    return 0;
}

int usock_init (int *fd, const char *path)
{
    assert (fd != NULL);
    assert (path != NULL);

    if (fd == NULL) {
        handle_err ("usock_init", "fd == NULL");
        return -1;
    }

    if (path == NULL) {
        handle_err ("usock_init", "path == NULL");
        return -1;
    }

    int sfd;
    struct sockaddr_un my_addr;
    socklen_t peer_addr_size;

    sfd = socket (AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        handle_err ("usock_init", "sfd == -1");
        return -1;
    }

    // clear structure 
    memset(&my_addr, 0, sizeof(struct sockaddr_un));

    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, path, strlen (path));

    // TODO FIXME
    // delete the unix socket if already created

    peer_addr_size = sizeof(struct sockaddr_un);

    if (bind (sfd, (struct sockaddr *) &my_addr, peer_addr_size) == -1) {
        handle_err ("usock_init", "bind == -1");
        perror("bind");
        return -1;
    }

    if (listen (sfd, LISTEN_BACKLOG) == -1) {
        handle_err ("usock_init", "listen == -1");
        perror("listen");
        return -1;
    }

    *fd = sfd;

    return 0;
}

int usock_accept (int fd, int *pfd)
{
    assert (pfd != NULL);

    if (pfd == NULL) {
        handle_err ("usock_accept", "pfd == NULL");
        return -1;
    }

    struct sockaddr_un peer_addr;
    memset (&peer_addr, 0, sizeof (struct sockaddr_un));
    socklen_t peer_addr_size = 0;

    *pfd = accept (fd, (struct sockaddr *) &peer_addr, &peer_addr_size);
    if (*pfd < 0) {
        handle_err ("usock_accept", "accept < 0");
        perror("listen");
        return -1;
    }

    return 0;
}

int usock_close (int fd)
{
    int ret = 0;

	ret = close (fd);
    if (ret < 0) {
        handle_err ("usock_close", "close ret < 0");
        perror ("closing");
		return -1;
    }
    return 0;
}

int usock_remove (const char *path)
{
    return unlink (path);
}
