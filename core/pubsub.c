#include "pubsub.h"
#include <stdlib.h>

#include <string.h> // strndup

void pubsubd_msg_serialize (const struct pubsub_msg *msg, char **data, size_t *len)
{
    if (msg == NULL || data == NULL || len == NULL) {
        fprintf (stderr, "pubsubd_msg_send: msg or data or len == NULL");
        return;
    }

    /* Preallocate the map structure */
    cbor_item_t * root = cbor_new_definite_map(1);
    /* Add the content */
    cbor_map_add(root, (struct cbor_pair) {
            .key = cbor_move(cbor_build_uint8((unsigned char) msg->type)),
            .value = cbor_move(cbor_build_bytestring((unsigned char*) msg->data, msg->datalen))
            });

    size_t buffer_size;
    *len = cbor_serialize_alloc (root, (unsigned char **) data, &buffer_size);
    cbor_decref(&root);
}

void pubsubd_msg_unserialize (struct pubsub_msg *msg, const char *buf, size_t mlen)
{
    if (msg == NULL) {
        fprintf (stderr
                , "\033[31merr: pubsubd_msg_unserialize, msg NULL\033[00m\n");
        return;
    }
    
    if (buf == NULL) {
        fprintf (stderr
                , "\033[31merr: pubsubd_msg_unserialize, buf NULL\033[00m\n");
        return;
    }

    if (mlen > BUFSIZ) {
        fprintf (stderr
                , "\033[31merr: pubsubd_msg_unserialize, mlen %ld\033[00m\n"
                , mlen);
        return;
    }

    // CBOR reading, from buf to pubsub_msg structure
    struct cbor_load_result result;
    cbor_item_t * item = cbor_load ((unsigned char *) buf, mlen, &result);

    struct cbor_pair * pair = cbor_map_handle (item);
    cbor_mutable_data data = cbor_bytestring_handle (pair->value);

    msg->type = cbor_get_uint8 (pair->key);
    if (msg->type != PUBSUB_TYPE_DISCONNECT) {
        msg->datalen = cbor_bytestring_length (pair->value);
        msg->data = malloc (msg->datalen +1);
        memset (msg->data, 0, msg->datalen +1);
        memcpy (msg->data, data, msg->datalen);
    }

    /* Deallocate the result */
    cbor_decref (&item);
}

void pubsubd_msg_free (struct pubsub_msg *msg)
{
    if (msg == NULL) {
        fprintf (stderr, "\033[31merr: pubsubd_msg_free, msg NULL\033[00m\n");
        return;
    }

    if (msg->chan) {
        free (msg->chan);
        msg->chan = NULL;
    }
    if (msg->data) {
        free (msg->data);
        msg->data = NULL;
    }
}

void pubsubd_msg_print (const struct pubsub_msg *msg)
{
    printf ("msg: type=%d chan=%s, data=%s\n"
            , msg->type, msg->chan, msg->data);
}

#define PUBSUB_SUBSCRIBER_ACTION_STR_PUB    "pub"
#define PUBSUB_SUBSCRIBER_ACTION_STR_SUB    "sub"
#define PUBSUB_SUBSCRIBER_ACTION_STR_BOTH   "both"
#define PUBSUB_SUBSCRIBER_ACTION_STR_QUIT   "quit"

// enum app_list_elm_action {PUBSUB_QUIT = 1, PUBSUB_PUB, PUBSUB_SUB, PUBSUB_BOTH};

char * pubsub_action_to_str (enum app_list_elm_action action)
{
    switch (action) {
        case PUBSUB_PUB : return strdup (PUBSUB_SUBSCRIBER_ACTION_STR_PUB);
        case PUBSUB_SUB : return strdup (PUBSUB_SUBSCRIBER_ACTION_STR_SUB);
        case PUBSUB_BOTH : return strdup (PUBSUB_SUBSCRIBER_ACTION_STR_BOTH);
        case PUBSUB_QUIT : return strdup (PUBSUB_SUBSCRIBER_ACTION_STR_QUIT);
    }

    return NULL;
}

void pubsub_connection (struct service *srv, struct process *p, enum app_list_elm_action action, const char *channame)
{
    char * straction = NULL;
    straction = pubsub_action_to_str (action);

    char line[BUFSIZ];
    memset (line, 0, BUFSIZ);

    // line fmt : index version action chan
    // "quit" action is also possible (see pubsubd_quit)
    snprintf (line, BUFSIZ, "%d %d %s %s\n"
            , p->index
            , p->version
            , straction
            , channame);
    line[BUFSIZ -1] = '\0'; // to be sure

    // send the connection line in the $TMP/<service> pipe
    app_srv_connection (srv, line, strlen (line));

    if (straction != NULL)
        free (straction);
}

void pubsub_disconnect (struct process *p)
{
    struct pubsub_msg m;
    memset (&m, 0, sizeof (struct pubsub_msg));
    m.type = PUBSUB_TYPE_DISCONNECT;

    char *buf = NULL;
    size_t msize = 0;
    pubsubd_msg_serialize (&m, &buf, &msize);

    int ret = app_write (p, buf, msize);
    if (ret != (int) msize) {
        fprintf (stderr, "err: can't disconnect\n");
    }

    pubsubd_msg_free (&m);
    if (buf != NULL) {
        free (buf);
    }
}

// tell the service to stop
void pubsubd_quit (struct service *srv)
{
    // line fmt : 0 0 0 quit
    char line[BUFSIZ];
    snprintf (line, BUFSIZ, "0 0 0 quit\n");
    app_srv_connection (srv, line, strlen (line));
}

void pubsub_msg_send (struct process *p, const struct pubsub_msg * m)
{
    size_t msize = 0;
    char * buf = NULL;
    pubsubd_msg_serialize (m, &buf, &msize);

    app_write (p, buf, msize);

    free(buf);
}

void pubsub_msg_recv (struct process *p, struct pubsub_msg *m)
{
    // read the message from the process
    size_t mlen = 0;
    char *buf = NULL;
    while (buf == NULL || mlen == 0) {
        app_read (p, &buf, &mlen);
    }

    pubsubd_msg_unserialize (m, buf, mlen);

    if (buf != NULL) {
        free (buf);
    }

}
