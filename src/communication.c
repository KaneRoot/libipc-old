#include <sys/select.h>
#include <unistd.h>

#include <stdio.h>
#include <errno.h>		// error numbers

#include <stdlib.h>
#include <string.h>

#include "ipc.h"
#include "utils.h"

// print structures
#include "message.h"

struct ipc_error service_path (char *path, const char *sname, int32_t index, int32_t version)
{
	T_R ((path == NULL), IPC_ERROR_SERVICE_PATH__NO_PATH);
	T_R ((sname == NULL), IPC_ERROR_SERVICE_PATH__NO_SERVICE_NAME);

	memset (path, 0, PATH_MAX);

	char *rundir = getenv ("IPC_RUNDIR");
	if (rundir == NULL)
		rundir = RUNDIR;

	snprintf (path, PATH_MAX - 1, "%s/%s-%d-%d", rundir, sname, index, version);

	IPC_RETURN_NO_ERROR;
}

static int32_t get_max_fd (struct ipc_connection_infos *cinfos)
{
	size_t i;
	int32_t max = 0;

	for (i = 0; i < cinfos->size; i++) {
		if (cinfos->cinfos[i]->fd > max) {
			max = cinfos->cinfos[i]->fd;
		}
	}

	return max;
}

struct ipc_error ipc_server_init (char **env, struct ipc_connection_info *srv, const char *sname)
{
	T_R ((env == NULL), IPC_ERROR_SERVER_INIT__NO_ENVIRONMENT_PARAM);
	T_R ((srv == NULL), IPC_ERROR_SERVER_INIT__NO_SERVICE_PARAM);
	T_R ((sname == NULL), IPC_ERROR_SERVER_INIT__NO_SERVER_NAME_PARAM);

#if 0
	// For server init, no need for networkd evaluation

	// TODO: loop over environment variables
	// any IPC_NETWORK_* should be shared with the network service
	// in order to route requests over any chosen protocol stack
	// ex: IPC_NETWORK_AUDIO="tor://some.example.com/"
	for (size_t i = 0; env[i] != NULL; i++) {
		// TODO: check for every IPC_NETWORK_* environment variable
	}
#endif

	// gets the service path
	SECURE_BUFFER_DECLARATION (char, buf, PATH_MAX);
	TEST_IPC_RR (service_path (buf, sname, srv->index, srv->version), "cannot get server path");

	// gets the service path
	if (srv->spath != NULL) {
		free (srv->spath);
		srv->spath = NULL;
	}

	size_t s = strlen (buf);

	SECURE_BUFFER_HEAP_ALLOCATION_R (srv->spath, s + 1,, IPC_ERROR_SERVER_INIT__MALLOC);
	memcpy (srv->spath, buf, s);
	srv->spath[s] = '\0';	// to be sure

	TEST_IPC_RETURN_ON_ERROR (usock_init (&srv->fd, srv->spath));

	IPC_RETURN_NO_ERROR;
}

struct ipc_error ipc_write_fd (int fd, const struct ipc_message *m);

// when networkd is not working properly (or do not retrieve the service): srv->fd = 0
struct ipc_error ipc_contact_networkd (struct ipc_connection_info *srv, const char *sname)
{
	T_R ((srv == NULL), IPC_ERROR_CONTACT_NETWORKD__NO_SERVER_PARAM);
	T_R ((sname == NULL), IPC_ERROR_CONTACT_NETWORKD__NO_SERVICE_NAME_PARAM);

	char *networkvar = getenv ("IPC_NETWORK");
	if (networkvar == NULL) {
		srv->fd = 0;
		IPC_RETURN_NO_ERROR;
	}
	// TODO: is there another, more interesting way to do this?
	// currently, IPC_NETWORK is shared with the network service
	// in order to route requests over any chosen protocol stack
	// ex: IPC_NETWORK="audio tor://some.example.com/audio ;pong tls://pong.example.com/pong"

	// printf ("IPC_NETWORK: %s\n", networkvar);

	SECURE_BUFFER_DECLARATION (char, columnthensname, BUFSIZ);
	columnthensname[0] = ';';
	memcpy (columnthensname + 1, sname, strlen (sname));

	if (strncmp (networkvar, sname, strlen (sname)) != 0 && strstr (networkvar, columnthensname) == NULL) {
		// printf ("sname %s not found\n", sname);
		srv->fd = 0;
		IPC_RETURN_NO_ERROR;
	}
	// printf ("(;)sname %s found\n", sname);

	// gets the service path
	SECURE_BUFFER_DECLARATION (char, buf, PATH_MAX);
	TEST_IPC_RR (service_path (buf, "network", 0, 0), "cannot get network service path");

	int networkdfd = 0;

	TEST_IPC_RETURN_ON_ERROR (usock_connect (&networkdfd, buf));

	SECURE_DECLARATION (struct ipc_message, msg);
	msg.type = MSG_TYPE_NETWORK_LOOKUP;
	msg.user_type = MSG_TYPE_NETWORK_LOOKUP;

	SECURE_BUFFER_DECLARATION (char, content, BUFSIZ);
	snprintf (content, BUFSIZ, "%s;%s", sname, networkvar);

	msg.length = strlen (content);
	msg.payload = content;

	TEST_IPC_RR (ipc_write_fd (networkdfd, &msg), "cannot send a message to networkd");

	struct ipc_error ret = ipc_receive_fd (networkdfd, &srv->fd);
	if (ret.error_code == IPC_ERROR_NONE) {
		usock_close (networkdfd);
	}

	return ret;
}

struct ipc_error ipc_connection (char **env, struct ipc_connection_info *srv, const char *sname)
{
	T_R ((env == NULL), IPC_ERROR_CONNECTION__NO_ENVIRONMENT_PARAM);
	T_R ((srv == NULL), IPC_ERROR_CONNECTION__NO_SERVER);
	T_R ((sname == NULL), IPC_ERROR_CONNECTION__NO_SERVICE_NAME);

	TEST_IPC_P (ipc_contact_networkd (srv, sname), "error during networkd connection");

	// if networkd did not initiate the connection
	if (srv->fd <= 0) {
		// gets the service path
		SECURE_BUFFER_DECLARATION (char, buf, PATH_MAX);
		TEST_IPC_RR (service_path (buf, sname, srv->index, srv->version), "cannot get server path");
		TEST_IPC_RETURN_ON_ERROR (usock_connect (&srv->fd, buf));
	}

	IPC_RETURN_NO_ERROR;
}

struct ipc_error ipc_server_close (struct ipc_connection_info *srv)
{
	usock_close (srv->fd);
	struct ipc_error ret = usock_remove (srv->spath);
	if (srv->spath != NULL) {
		free (srv->spath);
		srv->spath = NULL;
	}
	return ret;
}

struct ipc_error ipc_close (struct ipc_connection_info *p)
{
	return usock_close (p->fd);
}

struct ipc_error ipc_accept (struct ipc_connection_info *srv, struct ipc_connection_info *p)
{
	T_R ((srv == NULL), IPC_ERROR_ACCEPT__NO_SERVICE_PARAM);
	T_R ((p == NULL), IPC_ERROR_ACCEPT__NO_CLIENT_PARAM);

	TEST_IPC_RR (usock_accept (srv->fd, &p->fd), "cannot accept IPC connection");
	p->type = IPC_CONNECTION_TYPE_IPC;

	IPC_RETURN_NO_ERROR;
}

// receive then format in an ipc_message structure
struct ipc_error ipc_read (const struct ipc_connection_info *p, struct ipc_message *m)
{
	T_R ((m == NULL), IPC_ERROR_READ__NO_MESSAGE_PARAM);

	char *buf = NULL;
	size_t msize = IPC_MAX_MESSAGE_SIZE;

	// on error or closed recipient, the buffer already freed
	TEST_IPC_RETURN_ON_ERROR (usock_recv (p->fd, &buf, &msize));
	TEST_IPC_RETURN_ON_ERROR_FREE (ipc_message_format_read (m, buf, msize), buf);

	free (buf);

	IPC_RETURN_NO_ERROR;	// propagates ipc_message_format return
}

struct ipc_error ipc_write_fd (int fd, const struct ipc_message *m)
{
	T_R ((m == NULL), IPC_ERROR_WRITE__NO_MESSAGE_PARAM);

	char *buf = NULL;
	size_t msize = 0;
	ipc_message_format_write (m, &buf, &msize);

	size_t nbytes_sent = 0;
	TEST_IPC_RETURN_ON_ERROR_FREE (usock_send (fd, buf, msize, &nbytes_sent), buf);

	if (buf != NULL) {
		free (buf);
	}
	// what was sent != what should have been sent
	T_R ((nbytes_sent != msize), IPC_ERROR_WRITE__NOT_ENOUGH_DATA);

	IPC_RETURN_NO_ERROR;
}

struct ipc_error ipc_write (const struct ipc_connection_info *p, const struct ipc_message *m)
{
	return ipc_write_fd (p->fd, m);
}

struct ipc_error handle_new_connection (struct ipc_connection_info *cinfo, struct ipc_connection_infos *cinfos
	, struct ipc_connection_info **new_client)
{
	T_R ((cinfo == NULL), IPC_ERROR_HANDLE_NEW_CONNECTION__NO_CINFO_PARAM);
	T_R ((cinfos == NULL), IPC_ERROR_HANDLE_NEW_CONNECTION__NO_CINFOS_PARAM);

	SECURE_BUFFER_HEAP_ALLOCATION_R (*new_client, sizeof (struct ipc_connection_info),,
		IPC_ERROR_HANDLE_NEW_CONNECTION__MALLOC);

	TEST_IPC_RR (ipc_accept (cinfo, *new_client), "cannot accept the client during handle_new_connection");
	TEST_IPC_RR (ipc_add (cinfos, *new_client), "cannot add the new accepted client");

	IPC_RETURN_NO_ERROR;
}

// new connection from a client
struct ipc_error handle_connection (struct ipc_event *event, struct ipc_connection_infos *cinfos
	, struct ipc_connection_info *cinfo)
{
	// connection
	struct ipc_connection_info *new_client = NULL;

	TEST_IPC_RR (handle_new_connection (cinfo, cinfos, &new_client), "cannot add new client");

	IPC_EVENT_SET (event, IPC_EVENT_TYPE_CONNECTION, NULL, new_client);
	IPC_RETURN_NO_ERROR;
}

// new message
struct ipc_error handle_message (struct ipc_event *event, struct ipc_connection_infos *cinfos
	, struct ipc_connection_info *pc, struct ipc_switchings *switchdb)
{
	// if the socket is associated to another one for networkd
	// read and write automatically and provide a new IPC_EVENT_TYPE indicating the switch
	if (switchdb != NULL) {
		int talkingfd = pc->fd;
		int correspondingfd = ipc_switching_get (switchdb, talkingfd);
		if (correspondingfd != -1) {
			char *buf = NULL;
			size_t msize = 0;

			TEST_IPC_T_P_I_R (
						/* function to test */ usock_recv (talkingfd, &buf, &msize)
						, /* error condition */ ret.error_code != IPC_ERROR_NONE
						&& ret.error_code != IPC_ERROR_CLOSED_RECIPIENT
						, /* to do on error */ if (buf != NULL) free (buf);
						IPC_EVENT_SET (event, IPC_EVENT_TYPE_ERROR, NULL, pc)
						, /* return function */ return (ret)) ;

			/** TODO: there is a message, send it to the corresponding fd **/
			if (msize > 0) {
				size_t nbytes_sent = 0;
				TEST_IPC_RETURN_ON_ERROR_FREE (usock_send (correspondingfd, buf, msize, &nbytes_sent), buf);

				if (nbytes_sent != msize) {
					// LOG_ERROR ("wrote not enough data from %d to fd %d", talkingfd, correspondingfd);
					IPC_EVENT_SET (event, IPC_EVENT_TYPE_ERROR, NULL, pc);
					IPC_RETURN_NO_ERROR;	// FIXME: return something else, maybe?
				}
				// LOG_DEBUG ("received a message on fd %d => switch to fd %d", talkingfd, correspondingfd);

				if (buf != NULL)
					free (buf);

				// everything is OK: inform networkd of a successful transfer
				IPC_EVENT_SET (event, IPC_EVENT_TYPE_SWITCH, NULL, pc);
				IPC_RETURN_NO_ERROR;
			} else if (msize == 0) {
				int delfd;

				delfd = ipc_switching_del (switchdb, talkingfd);
				if (delfd >= 0) {
					close (delfd);
					ipc_del_fd (cinfos, delfd);
				}

				close (talkingfd);
				ipc_del_fd (cinfos, talkingfd);

#if 0
				if (delfd >= 0) {
					LOG_DEBUG ("disconnection of %d (and related fd %d)", talkingfd, delfd);
				} else {
					LOG_DEBUG ("disconnection of %d", talkingfd);
				}
#endif

				IPC_EVENT_SET (event, IPC_EVENT_TYPE_DISCONNECTION, NULL, pc);
				IPC_RETURN_ERROR (IPC_ERROR_CLOSED_RECIPIENT);
			}
		}
	}
	// no treatment of the socket if external socket
	if (pc->type == IPC_CONNECTION_TYPE_EXTERNAL) {
		IPC_EVENT_SET (event, IPC_EVENT_TYPE_EXTRA_SOCKET, NULL, pc);
		IPC_RETURN_NO_ERROR;
	}
	// listen to what they have to say (disconnection or message)
	// then add a client to `event`, the ipc_event structure
	SECURE_DECLARATION (struct ipc_error, ret);
	struct ipc_message *m = NULL;

	SECURE_BUFFER_HEAP_ALLOCATION_R (m, sizeof (struct ipc_message),, IPC_ERROR_HANDLE_MESSAGE__NOT_ENOUGH_MEMORY);

	// current talking client
	ret = ipc_read (pc, m);
	if (ret.error_code != IPC_ERROR_NONE && ret.error_code != IPC_ERROR_CLOSED_RECIPIENT) {
		struct ipc_error rvalue = ret;	// store the final return value
		ipc_message_empty (m);
		free (m);

		// if there is a problem, just remove the client
		TEST_IPC_P (ipc_close (pc), "cannot close a connection in handle_message");
		TEST_IPC_P (ipc_del (cinfos, pc), "cannot delete a connection in handle_message");

		IPC_EVENT_SET (event, IPC_EVENT_TYPE_ERROR, NULL, pc);
		return rvalue;
	}
	// disconnection: close the client then delete it from cinfos
	if (ret.error_code == IPC_ERROR_CLOSED_RECIPIENT) {
		TEST_IPC_P (ipc_close (pc), "cannot close a connection on closed recipient in handle_message");
		TEST_IPC_P (ipc_del (cinfos, pc), "cannot delete a connection on closed recipient in handle_message");

		ipc_message_empty (m);
		free (m);

		IPC_EVENT_SET (event, IPC_EVENT_TYPE_DISCONNECTION, NULL, pc);

		// warning: do not forget to free the ipc_client structure
		IPC_RETURN_NO_ERROR;
	}

	IPC_EVENT_SET (event, IPC_EVENT_TYPE_MESSAGE, m, pc);
	IPC_RETURN_NO_ERROR;
}

struct ipc_error ipc_wait_event_networkd (struct ipc_connection_infos *cinfos
	, struct ipc_connection_info *cinfo	// NULL for clients
	, struct ipc_event *event, struct ipc_switchings *switchdb
	, double *timer)
{
	T_R ((cinfos == NULL), IPC_ERROR_WAIT_EVENT__NO_CLIENTS_PARAM);
	T_R ((event == NULL), IPC_ERROR_WAIT_EVENT__NO_EVENT_PARAM);

	IPC_EVENT_CLEAN (event);

	size_t i, j;
	/* master file descriptor list */
	fd_set master;
	fd_set readf;

	/* clear the master and temp sets */
	FD_ZERO (&master);
	FD_ZERO (&readf);

	/* maximum file descriptor number */
	/* keep track of the biggest file descriptor */
	int32_t fdmax = get_max_fd (cinfos);

	/* listening socket descriptor */
	int32_t listener;
	if (cinfo != NULL) {
		listener = cinfo->fd;

		/* add the listener to the master set */
		FD_SET (listener, &master);

		/* if listener is max fd */
		if (fdmax < listener)
			fdmax = listener;
	}

	for (i = 0; i < cinfos->size; i++) {
		FD_SET (cinfos->cinfos[i]->fd, &master);
	}

	readf = master;

	struct timeval *ptimeout = NULL;
	SECURE_DECLARATION (struct timeval, timeout);

	if (timer != NULL && *timer > 0.0) {
		timeout.tv_sec  = (long) *timer;
		timeout.tv_usec = (long) ((long)((*timer) * 1000000) % 1000000);
		ptimeout = &timeout;
	}

	T_PERROR_RIPC ((select (fdmax + 1, &readf, NULL, NULL, ptimeout) == -1), "select", IPC_ERROR_WAIT_EVENT__SELECT);

	if (ptimeout != NULL) {
		*timer = (double) timeout.tv_sec + (timeout.tv_usec / 1000000.0);
		if (*timer == 0) {
			IPC_EVENT_SET (event, IPC_EVENT_TYPE_TIMER, NULL, NULL);
			IPC_RETURN_NO_ERROR;
		}
	}

	for (i = 0; i <= (size_t) fdmax; i++) {
		if (FD_ISSET (i, &readf)) {
			if (cinfo != NULL && i == (size_t) listener) {
				return handle_connection (event, cinfos, cinfo);
			} else {
				for (j = 0; j < cinfos->size; j++) {
					if (i == (size_t) cinfos->cinfos[j]->fd) {
						return handle_message (event, cinfos, cinfos->cinfos[j], switchdb);
					}
				}
			}
		}
	}

	IPC_RETURN_NO_ERROR;
}

struct ipc_error ipc_wait_event (struct ipc_connection_infos *cinfos
	, struct ipc_connection_info *cinfo	// NULL for clients
	, struct ipc_event *event, double *timer)
{
	return ipc_wait_event_networkd (cinfos, cinfo, event, NULL, timer);
}

// store and remove only pointers on allocated structures
struct ipc_error ipc_add (struct ipc_connection_infos *cinfos, struct ipc_connection_info *p)
{
	T_R ((cinfos == NULL), IPC_ERROR_ADD__NO_PARAM_CLIENTS);
	T_R ((p == NULL), IPC_ERROR_ADD__NO_PARAM_CLIENT);

	cinfos->size++;
	if (cinfos->size == 1 && cinfos->cinfos == NULL) {
		// first allocation
		SECURE_BUFFER_HEAP_ALLOCATION_R (cinfos->cinfos, sizeof (struct ipc_connection_info),,
						IPC_ERROR_ADD__MALLOC);
	} else {
		cinfos->cinfos = realloc (cinfos->cinfos, sizeof (struct ipc_connection_info) * cinfos->size);
	}

	T_R ((cinfos->cinfos == NULL), IPC_ERROR_ADD__EMPTY_LIST);

	cinfos->cinfos[cinfos->size - 1] = p;
	IPC_RETURN_NO_ERROR;
}

struct ipc_error ipc_del (struct ipc_connection_infos *cinfos, struct ipc_connection_info *p)
{
	T_R ((cinfos == NULL), IPC_ERROR_DEL__NO_CLIENTS_PARAM);
	T_R ((p == NULL), IPC_ERROR_DEL__NO_CLIENT_PARAM);
	T_R ((cinfos->cinfos == NULL), IPC_ERROR_DEL__EMPTY_LIST);

	size_t i;
	for (i = 0; i < cinfos->size; i++) {
		if (cinfos->cinfos[i] == p) {
			// TODO: possible memory leak if the ipc_connection_info is not free'ed
			cinfos->cinfos[i] = cinfos->cinfos[cinfos->size - 1];
			cinfos->size--;
			if (cinfos->size == 0) {
				ipc_connections_free (cinfos);
			} else {
				cinfos->cinfos = realloc (cinfos->cinfos, sizeof (struct ipc_connection_info) * cinfos->size);

				if (cinfos->cinfos == NULL) {
					IPC_RETURN_ERROR (IPC_ERROR_DEL__EMPTIED_LIST);
				}
			}

			IPC_RETURN_NO_ERROR;
		}
	}

	IPC_RETURN_ERROR (IPC_ERROR_DEL__CANNOT_FIND_CLIENT);
}

void ipc_connections_close (struct ipc_connection_infos *cinfos)
{
	if (cinfos->cinfos != NULL) {
		for (size_t i = 0; i < cinfos->size; i++) {
			ipc_close (cinfos->cinfos[i]);
			free (cinfos->cinfos[i]);
		}
		free (cinfos->cinfos);
		cinfos->cinfos = NULL;
	}
	cinfos->size = 0;
}

void ipc_connections_free (struct ipc_connection_infos *cinfos)
{
	if (cinfos->cinfos != NULL) {
		for (size_t i = 0; i < cinfos->size; i++) {
			free (cinfos->cinfos[i]);
		}
		free (cinfos->cinfos);
		cinfos->cinfos = NULL;
	}
	cinfos->size = 0;
}

// create the client service structure
struct ipc_error ipc_connection_gen (struct ipc_connection_info *cinfo
	, uint32_t index, uint32_t version
	, int fd, char type)
{
	T_R ((cinfo == NULL), IPC_ERROR_CONNECTION_GEN__NO_CINFO);

	cinfo->type = type;
	cinfo->version = version;
	cinfo->index = index;
	cinfo->fd = fd;

	IPC_RETURN_NO_ERROR;
}

// add an arbitrary file descriptor to read
struct ipc_error ipc_add_fd (struct ipc_connection_infos *cinfos, int fd)
{
	T_R ((cinfos == NULL), IPC_ERROR_ADD_FD__NO_PARAM_CINFOS);

	struct ipc_connection_info *cinfo = NULL;

	SECURE_BUFFER_HEAP_ALLOCATION_R (cinfo, sizeof (struct ipc_connection_info),,
					IPC_ERROR_ADD_FD__NOT_ENOUGH_MEMORY);

	ipc_connection_gen (cinfo, 0, 0, fd, IPC_CONNECTION_TYPE_EXTERNAL);

	return ipc_add (cinfos, cinfo);
}

// remove a connection from its file descriptor
struct ipc_error ipc_del_fd (struct ipc_connection_infos *cinfos, int fd)
{
	T_R ((cinfos == NULL), IPC_ERROR_DEL_FD__NO_PARAM_CINFOS);
	T_R ((cinfos->cinfos == NULL), IPC_ERROR_DEL_FD__EMPTY_LIST);

	size_t i;
	for (i = 0; i < cinfos->size; i++) {
		if (cinfos->cinfos[i]->fd == fd) {
			cinfos->cinfos[i]->fd = -1;
			free (cinfos->cinfos[i]);
			cinfos->size--;
			if (cinfos->size == 0) {
				// free cinfos->cinfos
				ipc_connections_free (cinfos);
			} else {
				cinfos->cinfos[i] = cinfos->cinfos[cinfos->size];
				cinfos->cinfos = realloc (cinfos->cinfos, sizeof (struct ipc_connection_info) * cinfos->size);

				if (cinfos->cinfos == NULL) {
					IPC_RETURN_ERROR (IPC_ERROR_DEL_FD__EMPTIED_LIST);
				}
			}

			IPC_RETURN_NO_ERROR;
		}
	}

	IPC_RETURN_ERROR (IPC_ERROR_DEL_FD__CANNOT_FIND_CLIENT);
}
