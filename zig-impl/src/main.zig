const std = @import("std");
const testing = std.testing;

// TODO: file descriptors should have a specific type
//       (however, usize is pointer size).

// TODO: path => std.XXX.YYY, not simple [] const u8


pub const RUNDIR = "/run/ipc/";
pub const IPC_HEADER_SIZE = 6;
pub const IPC_BASE_SIZE = 2000000; // 2 MB, plenty enough space for messages
pub const IPC_MAX_MESSAGE_SIZE =  IPC_BASE_SIZE-IPC_HEADER_SIZE;
pub const IPC_VERSION = 4;

const print = std.debug.print;

pub const IPC_TYPE = enum {
    UNIX_SOCKETS
};

pub const MessageType = enum {
    SERVER_CLOSE,
    ERR,
    DATA,
    NETWORK_LOOKUP,
};

pub const Message = struct {

    @"type": MessageType,     // Internal message type.
    user_type: u8,            // User-defined message type (arbitrary).
    fd: usize,                // File descriptor concerned about this message.
    payload: []const u8,

    const Self = @This();

    pub fn init(fd: usize,
                @"type": MessageType,
                user_type: u8,
                payload: []const u8) Self {
        return Message {
            .fd = fd,
            .@"type" = @"type",
            .user_type = user_type,
            .payload = payload,
        };
    }

    pub fn format(
        self: Self,
        comptime _: []const u8,    // No need.
        _: std.fmt.FormatOptions,  // No need.
        out_stream: anytype,
    ) !void {
        try std.fmt.format(out_stream, "fd: {}, type {}, usertype {}, payload: {s}",
            .{self.fd, self.@"type", self.user_type, self.payload} );
    }
    // del == list.swapRemove(index)
};

test "Message - creation and display" {
    print("\n", .{});
    // fd type usertype payload
    var s = "hello!!";
    var m = Message.init(1, MessageType.DATA, 3, s);

    print("message:\t[{}]\n", .{m});
    print("\n", .{});
}

pub const Messages = std.ArrayList(Message);

//  Event types.
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

pub const EventType = enum {
    NOT_SET,       // Default. TODO: should we keep this?
    ERROR,         // A problem occured.
    EXTRA_SOCKET,  // Message received from a non IPC socket.
    SWITCH,        // Message to send to a corresponding fd.
    CONNECTION,    // New user.
    DISCONNECTION, // User disconnected.
    MESSAGE,       // New message.
    LOOKUP,        // Client asking for a service through ipcd.
    TIMER,         // Timeout in the poll(2) function.
    TX,            // Message sent.
};

// For IO callbacks (switching).
pub const EventCallBack = enum {
    NO_ERROR,      // No error. A message was generated.
    FD_CLOSING,    // The fd is closing.
    FD_ERROR,      // Generic error.
    PARSING_ERROR, // The message was read but with errors.
    IGNORE,        // The message should be ignored (protocol specific).
};

pub const Event = struct {

    @"type": EventType,
    index: u32,
    origin: usize,
    m: ?*Message,  // message pointer

    const Self = @This();

    pub fn init(@"type": EventType,
            index: u32,
            origin: usize,
            m: ?*Message) Self {
        return Self {
           .@"type" = @"type",
           .index   = index,
           .origin  = origin,
           .m       = m,
        };
    }

    pub fn set(self: *Self,
               @"type": EventType,
               index: u32,
               origin: usize,
               m: ?*Message) void {
        self.@"type" = @"type";
        self.index = index;
        self.origin = origin;
        self.m = m;
    }

    pub fn clean(self: *Self) void {
        self.@"type" = EventType.NOT_SET;
        self.index = @as(u8,0);
        self.origin = @as(usize,0);
        if (self.m) |message| {
            message.deinit();
        }
        self.m = null;
    }

    pub fn format(
        self: Self,
        comptime _: []const u8,    // No need.
        _: std.fmt.FormatOptions,  // No need.
        out_stream: anytype,
    ) !void {
        try std.fmt.format(out_stream
            , "type {}, origin: {}, index {}, message: [{}]"
            , .{ self.@"type", self.origin, self.index, self.m} );
    }

};

test "Event - creation and display" {
    print("\n", .{});
    var s = "hello!!";
    // fd type usertype payload
    var m = Message.init(1, MessageType.DATA, 3, s);
    // type index origin message
    var e = Event.init(EventType.CONNECTION, 5, 8, &m);

    print("event:\t[{}]\n", .{e});
    print("\n", .{});
}

pub const ConnectionType = enum {
    IPC,      // Standard connection.
    EXTERNAL, // Non IPC connection (TCP, UDP, etc.).
    SERVER,   // Messages received = new connections.
    SWITCHED, // IO operations should go through registered callbacks.
};

// RATIONALE: a connection is mostly a file descriptor,
//            but with a few other pieces of information.
//            Storing all data related to a connection in a single place
//            would be logical but very inefficient.
//            File descriptors are stored elsewhere (in the context),
//            packed together, in a single dedicated structure.
pub const ConnectionInfos = struct {
    @"type": ConnectionType,
    more_to_read: bool,
    path: ?[] const u8, // Not always needed.

    const Self = @This();

    pub fn init(@"type": ConnectionType, path: ?[] const u8) Self {
        return Self {
           .@"type"      = @"type",
           .more_to_read = false,
           .path         = path,
        };
    }

    pub fn format(
        self: Self,
        comptime _: []const u8,    // No need.
        _: std.fmt.FormatOptions,  // No need.
        out_stream: anytype,
    ) !void {
        try std.fmt.format(out_stream
            , "connection type {}, more_to_read {}, path {s}"
            , .{ self.@"type", self.more_to_read, self.path} );
    }
};

test "ConnectionInfos - creation and display" {
    print("\n", .{});
    // origin destination
    var path = "/some/path";
    var c1 = ConnectionInfos.init(ConnectionType.EXTERNAL, path);
    var c2 = ConnectionInfos.init(ConnectionType.IPC     , null);
    print("connection 1:\t[{}]\n", .{c1});
    print("connection 2:\t[{}]\n", .{c2});
    print("\n", .{});
}

// TODO: callbacks.
pub const Switch = struct {
    origin : usize,
    destination : usize,
//     enum ipccb (*orig_in)  (int origin_fd, struct ipc_message *m, short int *more_to_read);
//     enum ipccb (*orig_out) (int origin_fd, struct ipc_message *m);
//     enum ipccb (*dest_in)  (int origin_fd, struct ipc_message *m, short int *more_to_read);
//     enum ipccb (*dest_out) (int origin_fd, struct ipc_message *m);

    const Self = @This();

    pub fn init(origin: usize, destination: usize) Self {
        return Self {
           .origin = origin,
           .destination = destination,
        };
    }

    pub fn format(
        self: Self,
        comptime _: []const u8,    // No need.
        _: std.fmt.FormatOptions,  // No need.
        out_stream: anytype,
    ) !void {
        try std.fmt.format(out_stream
            , "switch {} <-> {}"
            , .{ self.origin, self.destination} );
    }
};

test "Switch - creation and display" {
    // origin destination
    var s = Switch.init(3,8);
    print("switch:\t[{}]\n", .{s});
    print("\n", .{});
}

pub const Switches = std.ArrayList(Switch);
pub const Connections = std.ArrayList(ConnectionInfos);
pub const PollFD = std.ArrayList(usize);

// Context of the whole networking state.
pub const Context = struct {
    allocator: std.mem.Allocator,  // Memory allocator.
    connections: Connections,      // Keep track of connections.
    pollfd: PollFD,                // File descriptors.

    // TODO: List of "pollfd" structures within cinfos,
    //       so we can pass it to poll(2). Share indexes with 'connections'.
    // struct pollfd  *pollfd;

    tx: Messages,        // Messages to send, once their fd is available.
    switchdb: ?Switches, // Relations between fd.


    const Self = @This();

    // Context initialization:
    // - init structures (provide the allocator)
    pub fn init(allocator: std.mem.Allocator) Self {
        print("Context init\n", .{});
        return Self {
             .connections = Connections.init(allocator)
           , .pollfd = PollFD.init(allocator)
           , .tx = Messages.init(allocator)
           , .switchdb = null
           , .allocator = allocator
        };
    }

    pub fn deinit(self: *Self) void {
        print("connection deinit\n", .{});
        self.close_all() catch |err| switch(err){
            error.IndexOutOfBounds => {
                print("context.deinit(): IndexOutOfBounds\n", .{});
            },
        };
        self.connections.deinit();
        self.pollfd.deinit();
        self.tx.deinit();
        if (self.switchdb) |sdb| { sdb.deinit(); }
    }

    // Return the new fd. Can be useful to the caller.
    pub fn connect(self: *Self, path: []const u8) !usize {
        print("connection to {s}\n", .{path});
        const newfd = 0; // TODO
        var newcon = ConnectionInfos.init(ConnectionType.IPC, path);
        try self.connections.append(newcon);
        try self.pollfd.append(newfd);
        return newfd;
    }

    // Connection to a service, but with switched with the client fd.
    pub fn connection_switched(self: *Self
        , path: [] const u8
        , clientfd: usize) !usize {
        print("connection switched from {} to path {s}\n"
            , .{clientfd, path});
        const newfd = 0; // TODO
        var newcon = ConnectionInfos.init(ConnectionType.SWITCHED, path);
        try self.connections.append(newcon);
        try self.pollfd.append(newfd);
        // TODO: record switch.
        return newfd;
    }

    // Create a unix socket.
    pub fn server_init(self: *Self, path: [] const u8) !usize {
        print("context server init {s}\n", .{path});
        const newfd = 0; // TODO
        var newcon = ConnectionInfos.init(ConnectionType.SERVER, path);
        try self.connections.append(newcon);
        try self.pollfd.append(newfd);
        return newfd;
    }

    /// ipc_write   (ctx *, const struct ipc_message *m);
    pub fn write (self: *Self, m: Message) !void {
        print("write fd {}\n", .{m.fd});
        self.tx.append(m);
    }

    pub fn read (self: *Self, index: u32) !Message {
        // TODO: read the actual content.
        if (index >= self.pollfd.items.len) {
            return error.IndexOutOfBounds;
        }
        var fd = self.pollfd[index];
        print("read fd {} index {}\n", .{fd, index});
        var payload = "hello!!";
        // fd type usertype payload
        var m = Message.init(0, MessageType.DATA, 1, payload);
        return m;
    }

    pub fn read_fd (_: *Self, fd: usize) !Message {
        // TODO: read the actual content.
        print("read fd {}\n", .{fd});
        var payload = "hello!!";
        // fd type usertype payload
        var m = Message.init(0, MessageType.DATA, 1, payload);
        return m;
    }

    // Wait an event.
    pub fn wait_event(self: *Self, timer: *i32) !Event {
        for (self.pollfd.items) |fd| {
            print("listening to fd {}\n", .{fd});
        }
        print("listening for MAXIMUM {} us\n", .{timer});
        // TODO: listening to these file descriptors.
        var event = Event.init(EventType.CONNECTION, 5, 8, null);
        return event;
    }

    pub fn close(self: *Self, index: usize) !void {
        if (index >= self.pollfd.items.len) {
            return error.IndexOutOfBounds;
        }
        print("connection close index {}\n", .{index});
        // close the connection and remove it from the two structures
        if (self.connections.items.len > 0) {
            // TODO: actually close the file descriptor.
            _ = self.connections.swapRemove(index);
            _ = self.pollfd.swapRemove(index);
        }
    }

    pub fn close_all(self: *Self) !void {
        while(self.connections.items.len > 0) { try self.close(0); }
    }

    pub fn format(
        self: Self,
        comptime fmt: []const u8,
        options: std.fmt.FormatOptions,
        out_stream: anytype,
    ) !void {
        try std.fmt.format(out_stream
            , "context ({} connections and {} messages):"
            , .{self.connections.items.len, self.tx.items.len});

        for (self.connections.items) |con| {
            try std.fmt.format(out_stream, "\n- ", .{});
            try con.format(fmt, options, out_stream);
        }

        for (self.tx.items) |tx| {
            try std.fmt.format(out_stream, "\n- ", .{});
            try tx.format(fmt, options, out_stream);
        }
    }

};


// TODO
test "Context - creation, display and memory check" {
    print("\n", .{});
    // origin destination
    //var s = Switch.init(3,8);

    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();

    const allocator = gpa.allocator();

//    var payload = "hello!!";
//    // fd type usertype payload
//    var m = Message.init(0, MessageType.DATA, 1, payload);
//
//    // type index origin message
//    var e = Event.init(EventType.CONNECTION, 5, 8, &m);

    var c = Context.init(allocator);
    defer c.deinit(); // There. Can't leak. Isn't Zig wonderful?

    // Creating a service.
    _ = try c.server_init("/tmp/.TEST_USOCK");

    // Connection to a service.
    _ = try c.connect("/tmp/.TEST_USOCK");

    print ("Context: {}\n", .{c});
    print("\n", .{});
}

pub fn main() u8 {
    return 0;
}

// export fn add(a: i32, b: i32) i32 {
//     return a + b;
// }

// test "basic add functionality" {
//     try testing.expect(add(3, 7) == 10);
// }
