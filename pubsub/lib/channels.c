#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../core/error.h"
#include "../../core/client.h"

#include "channels.h"

void pubsubd_channel_print (const struct channel *chan)
{
    if (chan->chan == NULL) {
        handle_err ("pubsubd_channel_print", "chan->chan == NULL");
    }

    printf ( "\033[32mchan %s\033[00m\n", chan->chan);

    if (chan->subs == NULL) {
        handle_err ("pubsubd_channel_print", "chan->subs == NULL");
    }
    else {
        ipc_clients_print (chan->subs);
    }
}

void pubsubd_channels_print (const struct channels *chans)
{
    printf ("\033[36mmchannels\033[00m\n");

    if (chans == NULL) {
        handle_err ("pubsubd_channels_print", "chans == NULL");
        return ;
    }

    struct channel *chan = NULL;
    LIST_FOREACH(chan, chans, entries) {
        pubsubd_channel_print (chan);
    }
}

void pubsubd_channels_init (struct channels *chans) { LIST_INIT(chans); }

struct channel * pubsubd_channels_add (struct channels *chans, const char *chan)
{
    if(chans == NULL || chan == NULL) {
        handle_err ("pubsubd_channels_add", "chans == NULL or chan == NULL");
        return NULL;
    }

    struct channel *n = malloc (sizeof (struct channel));
    memset (n, 0, sizeof (struct channel));
    pubsubd_channel_new (n, chan);

    LIST_INSERT_HEAD(chans, n, entries);

    return n;
}

void pubsubd_channels_del (struct channels *chans, struct channel *c)
{
    struct channel *todel = pubsubd_channel_get (chans, c);
    if(todel != NULL) {
        pubsubd_channel_free (todel);
        LIST_REMOVE(todel, entries);
        free (todel);
        todel = NULL;
    }
}

void pubsubd_channels_del_all (struct channels *chans)
{
    if (!chans)
        return;

    struct channel *c = NULL;

    while (!LIST_EMPTY(chans)) {
        c = LIST_FIRST(chans);
        LIST_REMOVE(c, entries);
        pubsubd_channel_free (c);
        free (c);
        c = NULL;
    }
}

int pubsubd_channel_new (struct channel *c, const char * name)
{
    if (c == NULL) {
        return 1;
    }

    size_t nlen = (strlen (name) > BUFSIZ) ? BUFSIZ : strlen (name);

    if (c->chan == NULL)
        c->chan = malloc (nlen +1);

    memset (c->chan, 0, nlen +1);
    memcpy (c->chan, name, nlen);
    c->chanlen = nlen;

    c->subs = malloc (sizeof (struct ipc_clients));
    memset (c->subs, 0, sizeof (struct ipc_clients));

    return 0;
}

void pubsubd_channel_free (struct channel * c)
{
    if (c == NULL)
        return;

    if (c->chan != NULL) {
        free (c->chan);
        c->chan = NULL;
    }

    if (c->subs != NULL) {
        ipc_clients_free (c->subs);
        free (c->subs);
    }
}

struct channel * pubsubd_channel_search (struct channels *chans, char *chan)
{
    struct channel * np = NULL;
    LIST_FOREACH(np, chans, entries) {
        // TODO debug
        // printf ("pubsubd_channel_search: %s (%ld) vs %s (%ld)\n"
        //         , np->chan, np->chanlen, chan, strlen(chan));
        if (np->chanlen == strlen (chan)
                && strncmp (np->chan, chan, np->chanlen) == 0) {
        //    printf ("pubsubd_channel_search: FOUND\n");
            return np;
        }
    }
    return NULL;
}

struct channel * pubsubd_channel_get (struct channels *chans, struct channel *c)
{
    struct channel * np = NULL;
    LIST_FOREACH(np, chans, entries) {
        if (pubsubd_channel_eq (np, c))
            return np;
    }
    return NULL;
}

int pubsubd_channel_eq (const struct channel *c1, const struct channel *c2)
{
    return c1->chanlen == c2->chanlen &&
        strncmp (c1->chan, c2->chan, c1->chanlen) == 0;
}

void pubsubd_channel_subscribe (const struct channel *c, struct ipc_client *p)
{
    ipc_client_add (c->subs, p);
}

void pubsubd_channel_unsubscribe (const struct channel *c, struct ipc_client *p)
{
    ipc_client_del (c->subs, p);
}

void pubsubd_channels_subscribe (struct channels *chans
        , char *chname, struct ipc_client *p)
{
    struct channel *chan = pubsubd_channel_search (chans, chname);
    if (chan == NULL) {
        printf ("chan %s non existent : creation\n", chname);
        chan = pubsubd_channels_add (chans, chname);
    }

    pubsubd_channel_subscribe (chan, p);
}

void pubsubd_channels_unsubscribe (struct channels *chans
        , char *chname, struct ipc_client *p)
{
    struct channel *chan = pubsubd_channel_search (chans, chname);
    if (chan == NULL) {
        return;
    }

    pubsubd_channel_unsubscribe (chan, p);
}

void pubsubd_channels_unsubscribe_everywhere (struct channels *chans
        , struct ipc_client *p)
{
    struct channel * chan = NULL;
    LIST_FOREACH(chan, chans, entries) {
        pubsubd_channel_unsubscribe (chan, p);
    }
}
