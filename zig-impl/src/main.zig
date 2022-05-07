const std = @import("std");
const testing = std.testing;
const net = std.net;
const fmt = std.fmt;

// TODO: file descriptors should have a specific type (but i32 is used in std.net...).

// TODO: path => std.XXX.YYY, not simple [] const u8

// TODO: both Connection and pollfd store file descriptors.
//       Connection stores either Stream (server) or Address (client).

// TODO: API should completely obfuscate the inner structures.
//       Only structures in this file should be necessary.

pub const RUNDIR = "/run/ipc/";
pub const IPC_HEADER_SIZE = 6;
pub const IPC_BASE_SIZE = 2000000; // 2 MB, plenty enough space for messages
pub const IPC_MAX_MESSAGE_SIZE = IPC_BASE_SIZE-IPC_HEADER_SIZE;
pub const IPC_VERSION = 4;

const print = std.debug.print;

pub const Messages = std.ArrayList(Message);
pub const Switches = std.ArrayList(Switch);
pub const Connections = std.ArrayList(Connection);
pub const PollFD = std.ArrayList(i32);

pub const IPC_TYPE = enum {
    UNIX_SOCKETS
};

pub const Message = struct {

    pub const Type = enum {
        SERVER_CLOSE,
        ERROR,
        DATA,
        NETWORK_LOOKUP,
    };

    @"type": Message.Type,    // Internal message type.
    user_type: u8,            // User-defined message type (arbitrary).
    fd: usize,                // File descriptor concerned about this message.
    payload: []const u8,

    const Self = @This();

    // TODO
    //pub fn initFromConnection(fd: usize) Self {
    //    return Self{
    //        .@"type"   = Message.Type.ERROR,
    //        .user_type = 8,
    //        .fd        = fd,
    //        .payload   = "hello",
    //    };
    //}

    pub fn init(fd: usize,
                @"type": Message.Type,
                user_type: u8,
                payload: []const u8) Self {
        return Message {
            .fd = fd,
            .@"type" = @"type",
            .user_type = user_type,
            .payload = payload,
        };
    }

    pub fn format(self: Self, comptime _: []const u8, _: fmt.FormatOptions, out_stream: anytype) !void {
        try fmt.format(out_stream, "fd: {}, {}, usertype {}, payload: [{s}]",
            .{self.fd, self.@"type", self.user_type, self.payload} );
    }
};

test "Message - creation and display" {
    print("\n", .{});
    // fd type usertype payload
    var s = "hello!!";
    var m = Message.init(1, Message.Type.DATA, 3, s);

    print("message:\t[{}]\n", .{m});
    print("\n", .{});
}

pub const Event = struct {

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

    pub const Type = enum {
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
    pub const CallBack = enum {
        NO_ERROR,      // No error. A message was generated.
        FD_CLOSING,    // The fd is closing.
        FD_ERROR,      // Generic error.
        PARSING_ERROR, // The message was read but with errors.
        IGNORE,        // The message should be ignored (protocol specific).
    };

    @"type": Event.Type,
    index: u32,
    origin: usize,
    m: ?*Message,  // message pointer

    const Self = @This();

    pub fn init(@"type": Event.Type,
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
               @"type": Event.Type,
               index: u32,
               origin: usize,
               m: ?*Message) void {
        self.@"type" = @"type";
        self.index = index;
        self.origin = origin;
        self.m = m;
    }

    pub fn clean(self: *Self) void {
        self.@"type" = Event.Type.NOT_SET;
        self.index = @as(u8,0);
        self.origin = @as(usize,0);
        if (self.m) |message| {
            message.deinit();
        }
        self.m = null;
    }

    pub fn format(self: Self, comptime _: []const u8, _: fmt.FormatOptions, out_stream: anytype) !void {
        try fmt.format(out_stream
            , "{}, origin: {}, index {}, message: [{}]"
            , .{ self.@"type", self.origin, self.index, self.m} );
    }

};

test "Event - creation and display" {
    print("\n", .{});
    var s = "hello!!";
    // fd type usertype payload
    var m = Message.init(1, Message.Type.DATA, 3, s);
    // type index origin message
    var e = Event.init(Event.Type.CONNECTION, 5, 8, &m);

    print("event:\t[{}]\n", .{e});
    print("\n", .{});
}

pub const Connection = struct {

    pub const Type = enum {
        IPC,      // Standard connection.
        EXTERNAL, // Non IPC connection (TCP, UDP, etc.).
        SERVER,   // Messages received = new connections.
        SWITCHED, // IO operations should go through registered callbacks.
    };

    @"type": Connection.Type,
    more_to_read: bool,
    path: ?[] const u8, // Not always needed.

    server: ?net.StreamServer = null,
    client: ?net.StreamServer.Connection = null,

    const Self = @This();

    pub fn init(@"type": Connection.Type, path: ?[] const u8) Self {
        return Self {
           .@"type"      = @"type",
           .more_to_read = false,
           .path         = path,
        };
    }

    pub fn deinit(self: *Self) void {
        if (self.server) |s| { s.deinit(); }
    }

    pub fn format(self: Self, comptime _: []const u8, _: fmt.FormatOptions, out_stream: anytype) !void {
        try fmt.format(out_stream
            , "{}, more_to_read {}, path {s}"
            , .{ self.@"type", self.more_to_read, self.path} );
        if (self.server) |s| {
            try fmt.format(out_stream, "{}" , .{s});
        }
        if (self.client) |c| {
            try fmt.format(out_stream, "{}" , .{c});
        }
    }
};

test "Connection - creation and display" {
    print("\n", .{});
    // origin destination
    var path = "/some/path";
    var c1 = Connection.init(Connection.Type.EXTERNAL, path);
    var c2 = Connection.init(Connection.Type.IPC     , null);
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

    pub fn format(self: Self, comptime _: []const u8, _: fmt.FormatOptions, out_stream: anytype) !void {
        try fmt.format(out_stream
            , "switch {} <-> {}"
            , .{ self.origin, self.destination} );
    }
};

test "Switch - creation and display" {
    // origin destination
    var s = Switch.init(3,8);
    print("\nswitch:\t[{}]\n", .{s});
    print("\n", .{});
}

// Context of the whole networking state.
pub const Context = struct {
    allocator: std.mem.Allocator,  // Memory allocator.
    connections: Connections,      // Keep track of connections.

    // TODO: List of "pollfd" structures within cinfos,
    //       so we can pass it to poll(2). Share indexes with 'connections'.
    //       For now, this list doesn't do anything.
    //       Can even be replaced in a near future.
    pollfd: PollFD,      // File descriptors.

    tx: Messages,        // Messages to send, once their fd is available.
    switchdb: ?Switches, // Relations between fd.

    timer: ?i32 = null,  // No timer by default (no TIMER event).

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
        print("context deinit\n", .{});
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

    // Both simple connection and the switched one share this code.
    fn connect_ (self: *Self, ctype: Connection.Type, path: []const u8) !i32 {
        var stream = try net.connectUnixSocket(path);
        const newfd = stream.handle;
        errdefer std.os.closeSocket(newfd);
        var newcon = Connection.init(ctype, path);
        newcon.client = stream;
        try self.connections.append(newcon);
        try self.pollfd.append(newfd);
        return newfd;
    }

    // Return the new fd. Can be useful to the caller.
    pub fn connect(self: *Self, path: []const u8) !i32 {
        print("connection to:\t{s}\n", .{path});
        return self.connect_ (Connection.Type.IPC, path);
    }

    // Connection to a service, but with switched with the client fd.
    pub fn connection_switched(self: *Self
            , path: [] const u8
            , clientfd: i32) !i32 {
        print("connection switched from {} to path {s}\n", .{clientfd, path});
        var newfd = try self.connect_ (Connection.Type.SWITCHED, path);
        // TODO: record switch.
        return newfd;
    }

    // Create a unix socket.
    // Store std lib structures in the context.
    pub fn server_init(self: *Self, path: [] const u8) !net.StreamServer {
        print("context server init {s}\n", .{path});
        var server = net.StreamServer.init(.{});
        var socket_addr = try net.Address.initUnix(path);
        try server.listen(socket_addr);

        const newfd = server.sockfd orelse return error.SocketLOL;
        var newcon = Connection.init(Connection.Type.SERVER, path);
        newcon.server = server;
        try self.connections.append(newcon);
        try self.pollfd.append(newfd);
        return server;
    }

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
        var m = Message.init(0, Message.Type.DATA, 1, payload);
        return m;
    }

    pub fn read_fd (_: *Self, fd: i32) !Message {
        // TODO: read the actual content.
        print("read fd {}\n", .{fd});
        var payload = "hello!!";
        // fd type usertype payload
        var m = Message.init(0, Message.Type.DATA, 1, payload);
        return m;
    }

    // Wait an event.
    pub fn wait_event(self: *Self) !Event {
        // TODO: remove these debug prints.
        // for (self.pollfd.items) |fd| {
        //     print("listening to fd {}\n", .{fd});
        // }
        if (self.timer) |t| { print("listening for MAXIMUM {} us\n", .{t}); }
        else                { print("listening (no timer)\n", .{});         }

        // TODO: listening to these file descriptors.
        var event = Event.init(Event.Type.CONNECTION, 5, 8, null);
        return event;
    }

    pub fn close(self: *Self, index: usize) !void {
        // REMINDER: connections and pollfd have the same length
        if (index >= self.pollfd.items.len) {
            return error.IndexOutOfBounds;
        }

        // close the connection and remove it from the two structures
        // TODO: actually close the file descriptor.
        var con = self.connections.swapRemove(index);
        if (con.server) |s| {
            // Remove service's UNIX socket file.
            var addr = s.listen_address;
            var path = std.mem.sliceTo(&addr.un.path, 0);
            std.fs.cwd().deleteFile(path) catch {};
        }
        if (con.client) |c| {
            // Close the client's socket.
            c.stream.close();
        }
        _ = self.pollfd.swapRemove(index);
    }

    pub fn close_all(self: *Self) !void {
        while(self.connections.items.len > 0) { try self.close(0); }
    }

    pub fn format(self: Self, comptime form: []const u8, options: fmt.FormatOptions, out_stream: anytype) !void {
        try fmt.format(out_stream
            , "context ({} connections and {} messages):"
            , .{self.connections.items.len, self.tx.items.len});

        for (self.connections.items) |con| {
            try fmt.format(out_stream, "\n- ", .{});
            try con.format(form, options, out_stream);
        }

        for (self.tx.items) |tx| {
            try fmt.format(out_stream, "\n- ", .{});
            try tx.format(form, options, out_stream);
        }
    }

    // PRIVATE API

    fn read_ (_: *Self, client: net.StreamServer.Connection, buf: [] u8) !usize {
        return try client.stream.reader().read(buf);
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
//    var m = Message.init(0, Message.Type.DATA, 1, payload);
//
//    // type index origin message
//    var e = Event.init(Event.Type.CONNECTION, 5, 8, &m);

    var c = Context.init(allocator);
    defer c.deinit(); // There. Can't leak. Isn't Zig wonderful?

    const path = "/tmp/.TEST_USOCK";

    // SERVER SIDE: creating a service.
    var server = try c.server_init(path);
    defer server.deinit();
    defer std.fs.cwd().deleteFile(path) catch {}; // Once done, remove file.


    // CLIENT SIDE: connection to a service.
    //_ = try c.connect(path);

    // TODO: connection to a server, but switched with clientfd "3".
    // _ = try c.connection_switched(path, 3);

    // print ("Context: {}\n", .{c});
    // print("\n", .{});

    const S = struct {
        fn clientFn() !void {
            const socket = try net.connectUnixSocket(path);
            defer socket.close();
            print("So we're a client now... path: {s}\n", .{path});

            _ = try socket.writer().writeAll("Hello world!");
        }
    };

    const t = try std.Thread.spawn(.{}, S.clientFn, .{});
    defer t.join();

    // Server.accept returns a net.StreamServer.Connection.
    var client = try server.accept();
    defer client.stream.close();
    var buf: [16]u8 = undefined;
    const n = try client.stream.reader().read(&buf);

    try testing.expectEqual(@as(usize, 12), n);
    try testing.expectEqualSlices(u8, "Hello world!", buf[0..n]);
}


// FIRST
fn create_service() !void {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();

    const allocator = gpa.allocator();
    var ctx = Context.init(allocator);
    defer ctx.deinit(); // There. Can't leak. Isn't Zig wonderful?

    const path = "/tmp/.TEST_USOCK";

    // SERVER SIDE: creating a service.
    _ = try ctx.server_init(path);
    var event = try ctx.wait_event();
    switch (event.@"type") {
        .CONNECTION => {
            print("New connection!\n", .{});
        },
        else => {
            print("New event: {}\n", .{event.@"type"});
        },
    }

    // Server.accept returns a net.Connection (handle = fd, addr = net.Address).
    // var client = try server.accept();
    // var buf: [4096]u8 = undefined;
    // const n = try ctx.read_ (client, &buf);

    // print("new client: {}\n", .{client});
    // print("{} bytes: {s}\n", .{n, buf});
    print("End the create_service function\n", .{});
}

pub fn main() !u8 {
    try create_service();
    return 0;
}

// export fn add(a: i32, b: i32) i32 {
//     return a + b;
// }

// test "basic add functionality" {
//     try testing.expect(add(3, 7) == 10);
// }
