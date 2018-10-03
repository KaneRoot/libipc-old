#ifndef __CHANNELS_H__
#define __CHANNELS_H__

#include "../../core/queue.h"
#include "../../core/process.h"

// head of the list
LIST_HEAD(channels, channel);

// element of the list
// channel : chan name + chan name length + a list of applications
struct channel {
    char *chan;
    size_t chanlen;
    struct ipc_process_array *subs;
    LIST_ENTRY(channel) entries;
};

//  simple channel
int pubsubd_channel_new (struct channel *c, const char *name);
struct channel * pubsubd_channel_get (struct channels *chans, struct channel *c);
void pubsubd_channel_free (struct channel *c);
int pubsubd_channel_eq (const struct channel *c1, const struct channel *c2);
void pubsubd_channel_print (const struct channel *c);

// list of channels
void pubsubd_channels_init (struct channels *chans);
void pubsubd_channels_print (const struct channels *chans);
struct channel * pubsubd_channels_add (struct channels *chans, const char *chan);
void pubsubd_channels_del (struct channels *chans, struct channel *c);
void pubsubd_channels_del_all (struct channels *chans);
struct channel * pubsubd_channel_search (struct channels *chans, char *chan);

// add and remove subscribers
void pubsubd_channel_subscribe (const struct channel *c, struct ipc_process *p);
void pubsubd_channel_unsubscribe (const struct channel *c, struct ipc_process *p);

void pubsubd_channels_subscribe (struct channels *chans
        , char *chname, struct ipc_process *p);
void pubsubd_channels_unsubscribe (struct channels *chans
        , char *chname, struct ipc_process *p);

void pubsubd_channels_unsubscribe_everywhere (struct channels *chans
        , struct ipc_process *p);

#endif
