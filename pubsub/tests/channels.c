#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/channels.h"
#include "../../core/error.h"

void fake_process (struct ipc_process *p
        , unsigned int index, unsigned int version, int fake_fd)
{
    p->version = version;
    p->index = index;
    p->proc_fd = fake_fd;
}

void phase1 ()
{
    struct channel chan1;
    memset (&chan1, 0, sizeof (struct channel));
    pubsubd_channel_new (&chan1, "chan1");

    struct channel chan2;
    memset (&chan2, 0, sizeof (struct channel));
    pubsubd_channel_new (&chan2, "chan2");

    printf ("chan1:");
    pubsubd_channel_print (&chan1);

    printf ("chan2:");
    pubsubd_channel_print (&chan2);

    pubsubd_channel_free (&chan1);
    pubsubd_channel_free (&chan2);
}

void phase2 ()
{
    struct channel chan1;
    memset (&chan1, 0, sizeof (struct channel));
    pubsubd_channel_new (&chan1, "chan1");

    struct channel chan2;
    memset (&chan2, 0, sizeof (struct channel));
    pubsubd_channel_new (&chan2, "chan1");

    printf ("chan1:");
    pubsubd_channel_print (&chan1);

    printf ("chan2:");
    pubsubd_channel_print (&chan2);

    if (pubsubd_channel_eq (&chan1, &chan2)) {
        printf ("chan1 == chan2\n");
    }
    else {
        handle_err ("phase2", "pubsubd_channel_eq (&chan1, &chan2) == 0");
    }

    pubsubd_channel_free (&chan1);
    pubsubd_channel_free (&chan2);
}

void phase3 ()
{
    struct channels chans;
    memset (&chans, 0, sizeof (struct channels));

    pubsubd_channels_init (&chans);
    struct channel * chan1 = pubsubd_channels_add (&chans, "chan1");
    struct channel * chan2 = pubsubd_channels_add (&chans, "chan2");
    pubsubd_channels_print (&chans);
    pubsubd_channels_del (&chans, chan1);
    pubsubd_channels_print (&chans);
    pubsubd_channels_del (&chans, chan2);
    pubsubd_channels_print (&chans);
}

void phase4 ()
{
    struct channels chans;
    memset (&chans, 0, sizeof (struct channels));

    pubsubd_channels_init (&chans);
    struct channel * chan1 = pubsubd_channels_add (&chans, "chan1");
    struct channel * chan2 = pubsubd_channels_add (&chans, "chan2");

    struct ipc_process proc1;
    fake_process (&proc1, 0, 0, 1);

    struct ipc_process proc2;
    fake_process (&proc2, 0, 0, 2);

    printf ("chan1: proc1, chan2: proc2\n");
    pubsubd_channel_subscribe (chan1, &proc1);
    pubsubd_channel_subscribe (chan2, &proc2);

    pubsubd_channels_print (&chans);
    pubsubd_channels_del_all (&chans);

    printf ("channels removed\n");
    pubsubd_channels_print (&chans);
}

void phase5 ()
{
    struct channels chans;
    memset (&chans, 0, sizeof (struct channels));

    pubsubd_channels_init (&chans);
    pubsubd_channels_add (&chans, "chan1");
    pubsubd_channels_add (&chans, "chan2");

    struct ipc_process proc1;
    fake_process (&proc1, 0, 0, 1);

    struct ipc_process proc2;
    fake_process (&proc2, 0, 0, 2);

    printf ("chan1 & 2 => proc1 and 2 added\n");
    pubsubd_channels_subscribe (&chans, "chan1", &proc1);
    pubsubd_channels_subscribe (&chans, "chan1", &proc2);

    pubsubd_channels_subscribe (&chans, "chan2", &proc1);
    pubsubd_channels_subscribe (&chans, "chan2", &proc2);

    pubsubd_channels_print (&chans);

    printf ("chan1 => proc1 removed\n");
    pubsubd_channels_unsubscribe (&chans, "chan1", &proc1);

    pubsubd_channels_print (&chans);
    pubsubd_channels_del_all (&chans);

    printf ("channels removed\n");
    pubsubd_channels_print (&chans);
}

int main(int argc, char * argv[])
{
    argc = argc;
    argv = argv;

    // phase1(); // new + print + free
    // phase2(); // new + print + eq + free

    // channels
    // phase3(); // channels init + add + print + del
    // phase4(); // channels del_all + channel subscribe
    phase5(); // channels del_all + channels subscribe + unsubscribe

    return EXIT_SUCCESS;
}
