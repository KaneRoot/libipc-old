#include "ipc.h"

#include "utils.h"
#include <unistd.h>

#include <assert.h>
#include <stdio.h>
#include <errno.h> // error numbers

#include <stdlib.h>
#include <string.h>

// print structures
#include "message.h"


#define IPC_CONNECTION_TYPE_IPC		 'i'
#define IPC_CONNECTION_TYPE_EXTERNAL 'a'

void service_path (char *path, const char *sname, int32_t index, int32_t version)
{
    assert (path != NULL);
    assert (sname != NULL);

    memset (path, 0, PATH_MAX);

	char * rundir = getenv ("IPC_RUNDIR");
	if (rundir == NULL)
		rundir = RUNDIR;

    snprintf (path, PATH_MAX-1, "%s/%s-%d-%d", rundir, sname, index, version);
}

/*calculer le max filedescriptor*/
static int32_t get_max_fd (struct ipc_connection_infos *cinfos)
{
    int32_t i;
    int32_t max = 0;

    for (i = 0; i < cinfos->size; i++ ) {
        if (cinfos->cinfos[i]->fd > max) {
            max = cinfos->cinfos[i]->fd;
        } 
    }

    return max;
}

enum ipc_errors ipc_server_init (char **env, struct ipc_connection_info *srv, const char *sname)
{
    if (env == NULL)
        return IPC_ERROR_SERVER_INIT__NO_ENVIRONMENT_PARAM;

    if (srv == NULL)
        return IPC_ERROR_SERVER_INIT__NO_SERVICE_PARAM;

    if (sname == NULL)
        return IPC_ERROR_SERVER_INIT__NO_SERVER_NAME_PARAM;

	// TODO: loop over environment variables
	// any IPC_NETWORK_* should be shared with the network service
	// in order to route requests over any chosen protocol stack
	// ex: IPC_NETWORK_AUDIO="tor://some.example.com/"
    env = env;

    // gets the service path
	char buf [PATH_MAX];
	memset (buf, 0, PATH_MAX);
    service_path (buf, sname, srv->index, srv->version);

    // gets the service path
	if (srv->spath != NULL) {
		free (srv->spath);
		srv->spath = NULL;
	}

	size_t s = strlen (buf);
	srv->spath = malloc (s+1);
	if (srv->spath == NULL) {
		return IPC_ERROR_SERVER_INIT__MALLOC;
	}
	memcpy (srv->spath, buf, s);
	srv->spath[s] = '\0'; // to be sure

    enum ipc_errors ret = usock_init (&srv->fd, srv->spath);
	if (ret != IPC_ERROR_NONE) {
		handle_err ("ipc_server_init", "usock_init");
		return ret;
	}

	return IPC_ERROR_NONE;
}

enum ipc_errors ipc_connection (char **env, struct ipc_connection_info *srv, const char *sname)
{
	// TODO: loop over environment variables
	// any IPC_NETWORK_* should be shared with the network service
	// in order to route requests over any chosen protocol stack
	// ex: IPC_NETWORK_AUDIO="tor://some.example.com/"
    env = env;
    if (env == NULL)
        return IPC_ERROR_CONNECTION__NO_ENVIRONMENT_PARAM;

    assert (srv != NULL);
    assert (sname != NULL);

    if (srv == NULL) {
        return IPC_ERROR_CONNECTION__NO_SERVER;
    }

    if (sname == NULL) {
        return IPC_ERROR_CONNECTION__NO_SERVICE_NAME;
    }

    // gets the service path
	char buf [PATH_MAX];
	memset (buf, 0, PATH_MAX);
    service_path (buf, sname, srv->index, srv->version);

    enum ipc_errors ret = usock_connect (&srv->fd, buf);
	if (ret != IPC_ERROR_NONE) {
		handle_err ("ipc_connection", "usock_connect ret");
		return ret;
	}

    return IPC_ERROR_NONE;
}

enum ipc_errors ipc_server_close (struct ipc_connection_info *srv)
{
	usock_close (srv->fd);
	enum ipc_errors ret = usock_remove (srv->spath);
	if (srv->spath != NULL) {
		free (srv->spath);
		srv->spath = NULL;
	}
	return ret;
}

enum ipc_errors ipc_close (struct ipc_connection_info *p)
{
    return usock_close (p->fd);
}

enum ipc_errors ipc_accept (struct ipc_connection_info *srv, struct ipc_connection_info *p)
{
    assert (srv != NULL);
    assert (p != NULL);

	if (srv == NULL) {
		return IPC_ERROR_ACCEPT__NO_SERVICE_PARAM;
	}

	if (p == NULL) {
		return IPC_ERROR_ACCEPT__NO_CLIENT_PARAM;
	}

    enum ipc_errors ret = usock_accept (srv->fd, &p->fd);
	if (ret != IPC_ERROR_NONE) {
		handle_err ("ipc_accept", "usock_accept");
		return IPC_ERROR_ACCEPT;
	}

	p->type = IPC_CONNECTION_TYPE_IPC;

    return IPC_ERROR_NONE;
}

enum ipc_errors ipc_read (const struct ipc_connection_info *p, struct ipc_message *m)
{
    return ipc_message_read (p->fd, m);
}

enum ipc_errors ipc_write (const struct ipc_connection_info *p, const struct ipc_message *m)
{
    return ipc_message_write (p->fd, m);
}

enum ipc_errors handle_new_connection (struct ipc_connection_info *cinfo
		, struct ipc_connection_infos *cinfos
		, struct ipc_connection_info **new_client)
{
	if (cinfo == NULL) {
		return IPC_ERROR_HANDLE_NEW_CONNECTION__NO_CINFO_PARAM;
	}

	if (cinfos == NULL) {
		return IPC_ERROR_HANDLE_NEW_CONNECTION__NO_CINFOS_PARAM;
	}

    *new_client = malloc (sizeof(struct ipc_connection_info));
	if (*new_client == NULL) {
		return IPC_ERROR_HANDLE_NEW_CONNECTION__MALLOC;
	}
    memset(*new_client, 0, sizeof(struct ipc_connection_info));

	enum ipc_errors ret = ipc_accept (cinfo, *new_client);
    if (ret != IPC_ERROR_NONE) {
		return ret;
    }

	ret = ipc_add (cinfos, *new_client);
    if (ret != IPC_ERROR_NONE) {
		return ret;
    }

	return IPC_ERROR_NONE;
}

enum ipc_errors ipc_wait_event (struct ipc_connection_infos *cinfos
		, struct ipc_connection_info *cinfo // NULL for clients
        , struct ipc_event *event)
{
    assert (cinfos != NULL);

	if (cinfos == NULL) {
		return IPC_ERROR_WAIT_EVENT__NO_CLIENTS_PARAM;
	}

	if (event == NULL) {
		return IPC_ERROR_WAIT_EVENT__NO_EVENT_PARAM;
	}

	IPC_EVENT_CLEAN(event);

    int32_t i, j;
    /* master file descriptor list */
    fd_set master;
    fd_set readf;

    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&readf);

    /* maximum file descriptor number */
    /* keep track of the biggest file descriptor */
    int32_t fdmax = get_max_fd (cinfos);

    /* listening socket descriptor */
    int32_t listener;
	if (cinfo != NULL) {
		listener = cinfo->fd;

		/* add the listener to the master set */
		FD_SET(listener, &master);

		// if listener is max fd
		if (fdmax < listener)
			fdmax = listener;
	}

    for (i=0; i < cinfos->size; i++) {
        FD_SET(cinfos->cinfos[i]->fd, &master);
    }

	readf = master;
	if(select(fdmax+1, &readf, NULL, NULL, NULL) == -1) {
		perror("select");
		return IPC_ERROR_WAIT_EVENT__SELECT;
	}

	for (i = 0; i <= fdmax; i++) {
		if (FD_ISSET(i, &readf)) {
			if (cinfo != NULL && i == listener) {
				// connection
				struct ipc_connection_info *new_client = NULL;
				enum ipc_errors ret = handle_new_connection (cinfo, cinfos, &new_client);
				if (ret != IPC_ERROR_NONE) {
					// TODO: quit the program
					return ret;
				}
				IPC_EVENT_SET (event, IPC_EVENT_TYPE_CONNECTION, NULL, new_client);
				return IPC_ERROR_NONE;
			} else {
				for(j = 0; j < cinfos->size; j++) {
					if(i == cinfos->cinfos[j]->fd ) {

						struct ipc_connection_info *pc = cinfos->cinfos[j];

						// no treatment of the socket if external socket
						if (pc->type == IPC_CONNECTION_TYPE_EXTERNAL) {
							IPC_EVENT_SET (event, IPC_EVENT_TYPE_EXTRA_SOCKET, NULL, pc);
							return IPC_ERROR_NONE;
						}

						// listen to what they have to say (disconnection or message)
						// then add a client to `event`, the ipc_event structure
						enum ipc_errors ret;
						struct ipc_message *m = NULL;
						m = malloc (sizeof(struct ipc_message));
						if (m == NULL) {
							return IPC_ERROR_NOT_ENOUGH_MEMORY;
						}
						memset (m, 0, sizeof (struct ipc_message));

						// current talking client
						ret = ipc_read (pc, m);
						if (ret != IPC_ERROR_NONE && ret != IPC_ERROR_CLOSED_RECIPIENT) {
							handle_err ("ipc_wait_event", "ipc_read");
							ipc_message_empty (m);
							free (m);

							// if there is a problem, just remove the client
							ret = ipc_close (pc);
							if (ret != IPC_ERROR_NONE) {
								handle_err( "ipc_wait_event", "ipc_close");
							}

							ret = ipc_del (cinfos, pc);
							if (ret != IPC_ERROR_NONE) {
								handle_err( "ipc_wait_event", "ipc_del");
							}

							IPC_EVENT_SET(event, IPC_EVENT_TYPE_ERROR, NULL, pc);
							return ret;
						}

						// disconnection: close the client then delete it from cinfos
						if (ret == IPC_ERROR_CLOSED_RECIPIENT) {
							ret = ipc_close (pc);
							if (ret != IPC_ERROR_NONE) {
								handle_err( "ipc_wait_event", "ipc_close");
							}

							ret = ipc_del (cinfos, pc);
							if (ret != IPC_ERROR_NONE) {
								handle_err( "ipc_wait_event", "ipc_del");
							}
							ipc_message_empty (m);
							free (m);

							IPC_EVENT_SET(event, IPC_EVENT_TYPE_DISCONNECTION, NULL, pc);

							// warning: do not forget to free the ipc_client structure
							return IPC_ERROR_NONE;
						}

						IPC_EVENT_SET (event, IPC_EVENT_TYPE_MESSAGE, m, pc);
						return IPC_ERROR_NONE;
					}
				}
			}
		}
	}

	return IPC_ERROR_NONE;
}

// store and remove only pointers on allocated structures
enum ipc_errors ipc_add (struct ipc_connection_infos *cinfos, struct ipc_connection_info *p)
{
    assert(cinfos != NULL);
    assert(p != NULL);

    if (cinfos == NULL) {
        return IPC_ERROR_ADD__NO_PARAM_CLIENTS;
    }

    if (p == NULL) {
        return IPC_ERROR_ADD__NO_PARAM_CLIENT;
    }

    cinfos->size++;
	if (cinfos->size == 1 && cinfos->cinfos == NULL) {
		// first allocation
		cinfos->cinfos = malloc (sizeof(struct ipc_connection_info));
	}
	else {
		cinfos->cinfos = realloc(cinfos->cinfos, sizeof(struct ipc_connection_info) * cinfos->size);
	}


    if (cinfos->cinfos == NULL) {
        return IPC_ERROR_ADD__EMPTY_LIST;
    }

    cinfos->cinfos[cinfos->size - 1] = p;
    return IPC_ERROR_NONE;
}

enum ipc_errors ipc_del (struct ipc_connection_infos *cinfos, struct ipc_connection_info *p)
{
    assert(cinfos != NULL);
    assert(p != NULL);

    if (cinfos == NULL) {
        return IPC_ERROR_DEL__NO_CLIENTS_PARAM;
    }

    if (p == NULL) {
        return IPC_ERROR_DEL__NO_CLIENT_PARAM;
    }

    if (cinfos->cinfos == NULL) {
        return IPC_ERROR_DEL__EMPTY_LIST;
    }

    int32_t i;
    for (i = 0; i < cinfos->size; i++) {
        if (cinfos->cinfos[i] == p) {
			// TODO: possible memory leak if the ipc_connection_info is not free'ed
            cinfos->cinfos[i] = cinfos->cinfos[cinfos->size-1];
            cinfos->size--;
            if (cinfos->size == 0) {
                ipc_connections_free (cinfos);
            }
            else {
                cinfos->cinfos = realloc(cinfos->cinfos, sizeof(struct ipc_connection_info) * cinfos->size);

                if (cinfos->cinfos == NULL) {
                    return IPC_ERROR_DEL__EMPTIED_LIST;
                }
            }

            return IPC_ERROR_NONE;
        }
    }

    return IPC_ERROR_DEL__CANNOT_FIND_CLIENT;
}

void ipc_connections_free  (struct ipc_connection_infos *cinfos)
{
    if (cinfos->cinfos != NULL) {
		for (size_t i = 0; i < cinfos->size ; i++) {
			free (cinfos->cinfos[i]);
		}
        free (cinfos->cinfos);
        cinfos->cinfos = NULL;
    }
    cinfos->size = 0;
}

struct ipc_connection_info * ipc_connection_copy (const struct ipc_connection_info *p)
{
    if (p == NULL)
        return NULL;

    struct ipc_connection_info * copy = malloc (sizeof(struct ipc_connection_info));

    if (copy == NULL)
        return NULL;

    memset (copy, 0, sizeof (struct ipc_connection_info));
    memcpy (copy, p, sizeof (struct ipc_connection_info));

    return copy;
}

int8_t ipc_connection_eq (const struct ipc_connection_info *p1, const struct ipc_connection_info *p2)
{
    return (p1->type == p2->type && p1->version == p2->version && p1->index == p2->index && p1->fd == p2->fd);
}

// create the client service structure
enum ipc_errors ipc_connection_gen (struct ipc_connection_info *cinfo
		, uint32_t index, uint32_t version, int fd, char type)
{
	if (cinfo == NULL) {
		return IPC_ERROR_CONNECTION_GEN__NO_CINFO;
	}

	cinfo->type = type;
    cinfo->version = version;
    cinfo->index = index;
	cinfo->fd = fd;

	return IPC_ERROR_NONE;
}

// add an arbitrary file descriptor to read
enum ipc_errors ipc_add_fd (struct ipc_connection_infos *cinfos, int fd)
{
	if (cinfos == NULL) {
		return IPC_ERROR_ADD_FD__NO_PARAM_CINFOS;
	}

	struct ipc_connection_info *cinfo = NULL;

	cinfo = malloc (sizeof(struct ipc_connection_info));
	if (cinfo == NULL) {
		return IPC_ERROR_NOT_ENOUGH_MEMORY;
	}
	memset (cinfo, 0, sizeof(struct ipc_connection_info));

	enum ipc_errors ret;
	ipc_connection_gen (cinfo, 0, 0, fd, IPC_CONNECTION_TYPE_EXTERNAL);

	return ipc_add (cinfos, cinfo);
}

// remove a connection from its file descriptor
enum ipc_errors ipc_del_fd (struct ipc_connection_infos *cinfos, int fd)
{
    assert(cinfos != NULL);
	if (cinfos == NULL) {
		return IPC_ERROR_DEL_FD__NO_PARAM_CINFOS;
	}

    if (cinfos->cinfos == NULL) {
        return IPC_ERROR_DEL_FD__EMPTY_LIST;
    }

    int32_t i;
    for (i = 0; i < cinfos->size; i++) {
        if (cinfos->cinfos[i]->fd == fd) {
			free (cinfos->cinfos[i]);
            cinfos->size--;
            if (cinfos->size == 0) {
				// free cinfos->cinfos
                ipc_connections_free (cinfos);
            }
            else {
				cinfos->cinfos[i] = cinfos->cinfos[cinfos->size];
                cinfos->cinfos = realloc(cinfos->cinfos, sizeof(struct ipc_connection_info) * cinfos->size);

                if (cinfos->cinfos == NULL) {
                    return IPC_ERROR_DEL_FD__EMPTIED_LIST;
                }
            }

            return IPC_ERROR_NONE;
        }
    }

    return IPC_ERROR_DEL_FD__CANNOT_FIND_CLIENT;
}

void ipc_connection_print (struct ipc_connection_info *cinfo)
{
    if (cinfo == NULL) {
		return;
	}

	printf ("fd %d: index %d, version %d, type %c"
			, cinfo->fd, cinfo->index, cinfo->version, cinfo->type);

	if (cinfo->spath != NULL) {
		printf (", path %s\n", cinfo->spath);
	}
	else {
		printf ("\n");
	}
}

void ipc_connections_print (struct ipc_connection_infos *cinfos)
{
    int32_t i;
    for (i = 0; i < cinfos->size; i++) {
        printf("[%d] : ", i);
        ipc_connection_print(cinfos->cinfos[i]);
    }
}
