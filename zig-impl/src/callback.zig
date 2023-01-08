pub const CBEvent = struct {

    //  CallBack Event types.
    //  In the main event loop, servers and clients can receive connections,
    //  disconnections, errors or messages from their pairs. They also can
    //  set a timer so the loop will allow a periodic routine (sending ping
    //  messages for websockets, for instance).
    //
    //  A few other events can occur.
    //
    //  Extra socket
    //    The main loop waiting for an event can be used as an unique entry
    //    point for socket management. libipc users can register sockets via
    //    ipc_add_fd allowing them to trigger an event, so events unrelated
    //    to libipc are managed the same way.
    //  Switch
    //    libipc can be used to create protocol-related programs, such as a
    //    websocket proxy allowing libipc services to be accessible online.
    //    To help those programs (with TCP-complient sockets), two sockets
    //    can be bound together, each message coming from one end will be
    //    automatically transfered to the other socket and a Switch event
    //    will be triggered.
    //  Look Up
    //    When a client establishes a connection to a service, it asks the
    //    ipc daemon (ipcd) to locate the service and establish a connection
    //    to it. This is a lookup.

    // For IO callbacks (switching).
    pub const Type = enum {
        NO_ERROR,      // No error. A message was generated.
        ERROR,         // Generic error.
        FD_CLOSING,    // The fd is closing.
        IGNORE,        // The message should be ignored (protocol specific).
    };
};
