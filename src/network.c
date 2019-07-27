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
 * describe a protocol to get this working into networkd
 *   asking networkd for a fd with an URI
 *     URI should contain: who (the service name), where (destination), how (protocol)
 *   networkd initiates a communication with the requested service
 *   networkd sends the fd
 * get a networkd working with this
 */



enum ipc_errors ipc_receive_fd (int sock, int *fd)
{
	T_R ((fd == NULL), IPC_ERROR_RECEIVE_FD__NO_PARAM_FD);
	*fd = -1;

	SECURE_DECLARATION (struct msghdr, msg);
    SECURE_BUFFER_DECLARATION (char, c_buffer, 256);

    /* On Mac OS X, the struct iovec is needed, even if it points to minimal data */
    char m_buffer[1];
    struct iovec io = { .iov_base = m_buffer, .iov_len = sizeof(m_buffer) };
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;

    msg.msg_control = c_buffer;
    msg.msg_controllen = sizeof(c_buffer);

    T_PERROR_R ((recvmsg(sock, &msg, 0) <= 0), "recvmsg", IPC_ERROR_RECEIVE_FD__RECVMSG);

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

    memmove(fd, CMSG_DATA(cmsg), sizeof(*fd));

	return IPC_ERROR_NONE;
}

enum ipc_errors ipc_provide_fd (int sock, int fd)
{
    SECURE_DECLARATION (struct msghdr, msg);
    SECURE_BUFFER_DECLARATION (char, buf, CMSG_SPACE(sizeof(fd)));

    /* On Mac OS X, the struct iovec is needed, even if it points to minimal data */
    struct iovec io = { .iov_base = "", .iov_len = 1 };

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    struct cmsghdr * cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd));

    memmove(CMSG_DATA(cmsg), &fd, sizeof(fd));

    msg.msg_controllen = cmsg->cmsg_len;

    T_PERROR_R ((sendmsg(sock, &msg, 0) < 0), "sendmsg", IPC_ERROR_PROVIDE_FD__SENDMSG);

	return IPC_ERROR_NONE;
}

void ipc_switching_add (struct ipc_switchings *is, int orig, int dest)
{
    is->collection = realloc(is->collection, sizeof(struct ipc_switching) * (is->size+1));
    if (is->collection == NULL) {
        LOG_ERROR ("error realloc");
        exit (EXIT_FAILURE);
    }

    is->size++;

    is->collection[is->size-1].orig = orig;
    is->collection[is->size-1].dest = dest;
}

int ipc_switching_del (struct ipc_switchings *is, int fd)
{
    for (size_t i = 0; i < is->size; i++) {
        if (is->collection[i].orig == fd || is->collection[i].dest == fd) {
            int ret;

            if (fd == is->collection[i].orig) {
                ret = is->collection[i].dest;
            }
            else {
                ret = is->collection[i].orig;
            }

            is->collection[i].orig = is->collection[is->size-1].orig;
            is->collection[i].dest = is->collection[is->size-1].dest;

            size_t s = (is->size - 1) > 0 ? (is->size - 1) : 1;

            is->collection = realloc(is->collection, sizeof(struct ipc_switching) * s);
            if (is->collection == NULL) {
                LOG_ERROR ("error realloc");
                exit (EXIT_FAILURE);
            }

            is->size--;
            return ret;
        }
    }

    return -1;
}

int ipc_switching_get (struct ipc_switchings *is, int fd)
{
    for (size_t i = 0; i < is->size; i++) {
        if (is->collection[i].orig == fd) {
            return is->collection[i].dest;
        }
        else if (is->collection[i].dest == fd) {
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

void ipc_switching_print (struct ipc_switchings *is)
{
    for (size_t i = 0; i < is->size; i++) {
        LOG_DEBUG ("client %d - %d", is->collection[i].orig, is->collection[i].dest);
    }
}
