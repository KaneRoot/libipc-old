#include "client.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

struct ipc_client * ipc_server_client_copy (const struct ipc_client *p)
{
    if (p == NULL)
        return NULL;

    struct ipc_client * copy = malloc (sizeof(struct ipc_client));
    memset (copy, 0, sizeof (struct ipc_client));
    memcpy (copy, p, sizeof (struct ipc_client));

    return copy;
}

int ipc_server_client_eq (const struct ipc_client *p1, const struct ipc_client *p2)
{
    return (p1->version == p2->version && p1->index == p2->index
            && p1->proc_fd == p2->proc_fd);
}

void ipc_server_client_gen (struct ipc_client *p
        , unsigned int index, unsigned int version)
{
    p->version = version;
    p->index = index;
}

int ipc_client_add (struct ipc_clients *clients, struct ipc_client *p)
{
    assert(clients != NULL);
    assert(p != NULL);

    clients->size++;
    clients->clients = realloc(clients->clients
            , sizeof(struct ipc_client) * clients->size);

    if (clients->clients == NULL) {
        return -1;
    }

    clients->clients[clients->size - 1] = p;
    return 0;
}

int ipc_client_del (struct ipc_clients *clients, struct ipc_client *p)
{
    assert(clients != NULL);
    assert(p != NULL);

    if (clients->clients == NULL) {
        return -1;
    }

    int i;
    for (i = 0; i < clients->size; i++) {
        if (clients->clients[i] == p) {

            clients->clients[i] = clients->clients[clients->size-1];
            clients->size--;
            if (clients->size == 0) {
                ipc_clients_free (clients);
            }
            else {
                clients->clients = realloc(clients->clients
                        , sizeof(struct ipc_client) * clients->size);

                if (clients->clients == NULL) {
                    return -2;
                }
            }

            return 0;
        }
    }

    return -3;
}

void client_print (struct ipc_client *p)
{
    if (p != NULL)
        printf ("client %d : index %d, version %d\n"
                , p->proc_fd, p->index, p->version);
}

void ipc_clients_print (struct ipc_clients *ap)
{
    int i;
    for (i = 0; i < ap->size; i++) {
        printf("%d : ", i);
        client_print(ap->clients[i]);
    }
}

void ipc_clients_free (struct ipc_clients *ap)
{
    if (ap->clients != NULL) {
        free (ap->clients);
        ap->clients = NULL;
    }
    ap->size = 0;
}


