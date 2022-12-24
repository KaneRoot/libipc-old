const std = @import("std");
const hexdump = @import("./hexdump.zig");
const testing = std.testing;
const net = std.net;
const os = std.os;
const fmt = std.fmt;

const print = std.debug.print;

const CBEvent = @import("./callback.zig").CBEvent;
const Connection = @import("./connection.zig").Connection;
const Message = @import("./message.zig").Message;
const Event = @import("./event.zig").Event;
const Switch = @import("./switch.zig").Switch;
const print_eq = @import("./util.zig").print_eq;

const Messages = @import("./message.zig").Messages;
const Switches = @import("./switch.zig").Switches;
const Connections = @import("./connection.zig").Connections;

pub const PollFD = std.ArrayList(std.os.pollfd);

// Context of the whole networking state.
pub const Context = struct {
    pub const IPC_HEADER_SIZE = 5; // Size (5 bytes) then content.
    pub const IPC_BASE_SIZE = 2000000; // 2 MB, plenty enough space for messages
    pub const IPC_MAX_MESSAGE_SIZE = IPC_BASE_SIZE-IPC_HEADER_SIZE;
    pub const IPC_VERSION = 1;

    rundir: [] u8,
    allocator: std.mem.Allocator,  // Memory allocator.
    connections: Connections,      // Keep track of connections.

    // TODO: List of "pollfd" structures within cinfos,
    //       so we can pass it to poll(2). Share indexes with 'connections'.
    //       For now, this list doesn't do anything.
    //       Can even be replaced in a near future.
    pollfd: PollFD,  // .fd (fd_t) + .events (i16) + .revents (i16)

    tx: Messages,        // Messages to send, once their fd is available.
    switchdb: ?Switches, // Relations between fd.

    timer: ?i32 = null,  // No timer by default (no TIMER event).

    const Self = @This();

    // Context initialization:
    // - init structures (provide the allocator)
    pub fn init(allocator: std.mem.Allocator) !Self {
        var rundir = std.process.getEnvVarOwned(allocator, "RUNDIR") catch |err| switch(err) {
            error.EnvironmentVariableNotFound => blk: {
                // print("RUNTIME variable not set, using default /tmp/libipc-run/\n", .{});
                break :blk try allocator.dupeZ(u8, "/tmp/libipc-run/");
            },
            else => {
                return err;
            },
        };

        return Self {
             .rundir = rundir
           , .connections = Connections.init(allocator)
           , .pollfd = PollFD.init(allocator)
           , .tx = Messages.init(allocator)
           , .switchdb = null
           , .allocator = allocator
        };
    }

    // create a server path for the UNIX socket based on the service name
    pub fn server_path(self: *Self, service_name: []const u8, writer: anytype) !void {
        try writer.print("{s}/{s}", .{self.rundir, service_name});
    }

    pub fn deinit(self: *Self) void {
        self.close_all() catch |err| switch(err){
            error.IndexOutOfBounds => {
                print("context.deinit(): IndexOutOfBounds\n", .{});
            },
        };
        self.allocator.free(self.rundir);
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
        try self.pollfd.append(.{ .fd = newfd
                                , .events = std.os.linux.POLL.IN
                                , .revents = 0 });
        return newfd;
    }

    // Return the new fd. Can be useful to the caller.
    pub fn connect(self: *Self, path: []const u8) !i32 {
        // print("connection to:\t{s}\n", .{path});
        return self.connect_ (Connection.Type.IPC, path);
    }

    // Connection to a service, but with switched with the client fd.
//    pub fn connection_switched(self: *Self
//            , path: [] const u8
//            , clientfd: i32) !i32 {
//        // print("connection switched from {} to path {s}\n", .{clientfd, path});
//        var newfd = try self.connect_ (Connection.Type.SWITCHED, path);
//        // TODO: record switch.
//        return newfd;
//    }

    // Create a unix socket.
    // Store std lib structures in the context.
    pub fn server_init(self: *Self, path: [] const u8) !net.StreamServer {
        // print("context server init {s}\n", .{path});
        var server = net.StreamServer.init(.{});
        var socket_addr = try net.Address.initUnix(path);
        try server.listen(socket_addr);

        const newfd = server.sockfd orelse return error.SocketLOL;
        var newcon = Connection.init(Connection.Type.SERVER, path);
        newcon.server = server;
        try self.connections.append(newcon);
        try self.pollfd.append(.{ .fd = newfd
                                , .events = std.os.linux.POLL.IN
                                , .revents = 0 });
        return server;
    }

    pub fn write (self: *Self, m: Message) !void {
        print("write fd {}\n", .{m.fd});
        self.tx.append(m);
    }

    pub fn read (self: *Self, index: u32) !Message {
        if (index >= self.pollfd.items.len) {
            return error.IndexOutOfBounds;
        }
        print("read index {}\n", .{index});
        return self.read_fd(self.pollfd.items[index].fd);
    }

    pub fn read_fd (self: *Self, fd: i32) !Message {
        print("read fd {}\n", .{fd});

        // TODO: read the actual content.
        var payload = "hello!!";

        var m = Message.init(fd, Message.Type.DATA, self.allocator, payload);
        return m;
    }

    // Wait an event.
    pub fn wait_event(self: *Self) !Event {
        var current_event: Event = undefined;
        var wait_duration: i32 = -1; // -1 == unlimited

        if (self.timer) |t| { wait_duration = t; }
        else                { print("listening (no timer)\n", .{});         }

        // print("listening for MAXIMUM {} ms\n", .{wait_duration});

        // Make sure we listen to the right file descriptors,
        // setting POLLIN & POLLOUT flags.
        for (self.pollfd.items) |*fd| {
            // print("listening to fd {}\n", .{fd.fd});
            fd.events = std.os.linux.POLL.IN; // just to make sure
        }

        for (self.tx.items) |m| {
            print("wait for writing a message to fd {}\n", .{m.fd});
            for (self.pollfd.items) |*fd| {
                if (fd.fd == m.fd) {
                    fd.events = std.os.linux.POLL.OUT; // just to make sure
                }
            }
        }

        // TODO: before initiate a timer

        // Polling.
        var count: usize = undefined;

        // print("Let's wait for an event (either stdin or unix socket)\n", .{});
        print("fds: {any}\n", .{self.pollfd.items});
        count = try os.poll(self.pollfd.items, wait_duration);
        print("fds NOW: {any}\n", .{self.pollfd.items});

        // TODO: timer = end - start; if 0 => return timer event

        if (count == 0) {
            current_event = Event.init(Event.Type.TIMER, 0, 0, null);
            return current_event;
        }

        // TODO: handle messages
        //   => loop over ctx.size
        //     => if pollfd[i].revents is set to POLLIN
		//          => if fd is SERVER   => new connection
		//          => if fd is SWITCHED => msg to exchange (or drop the switch)
		//          => if fd is EXTERNAL => let user handle IO operations
		//          => otherwise         => new message or disconnection
        //     => if fd revent is POLLOUT
		//          => if SWITCHED => write message for its switch buddy
		//          => otherwise   => write message for the msg.fd
		//        if fd revent is POLLHUP
		//          => handle disconnection:
		//             close + remove fd from pollfd + return event
		//        if fd revent is POLLERR or POLLNVAL
		//          => return error event

        return current_event;
    }

    pub fn close(self: *Self, index: usize) !void {
        // REMINDER: connections and pollfd have the same length
        if (index >= self.pollfd.items.len) {
            return error.IndexOutOfBounds;
        }

        // close the connection and remove it from the two structures
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

//test "Simple structures - init, display and memory check" {
//    // origin destination
//    var s = Switch.init(3,8);
//    var payload = "hello!!";
//    // fd type payload
//    var m = Message.init(0, Message.Type.DATA, payload);
//
//    // type index origin message
//    var e = Event.init(Event.Type.CONNECTION, 5, 8, &m);

//    // CLIENT SIDE: connection to a service.
//    _ = try c.connect(path);

//    // TODO: connection to a server, but switched with clientfd "3".
//    _ = try c.connection_switched(path, 3);
//}

// Creating a new thread: testing UNIX communication.
// This is a client sending a raw "Hello world!" bytestring,
// not an instance of Message.
const CommunicationTestThread = struct {
    fn clientFn() !void {
        const config = .{.safety = true};
        var gpa = std.heap.GeneralPurposeAllocator(config){};
        defer _ = gpa.deinit();
        const allocator = gpa.allocator();

        var c = try Context.init(allocator);
        defer c.deinit(); // There. Can't leak. Isn't Zig wonderful?

        var buffer: [1000]u8 = undefined;
        var fbs = std.io.fixedBufferStream(&buffer);
        var writer = fbs.writer();

        try c.server_path("simple-context-test", writer);
        var path = fbs.getWritten();
        const socket = try net.connectUnixSocket(path);
        defer socket.close();
        _ = try socket.writer().writeAll("Hello world!");
    }
};

test "Context - creation, display and memory check" {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();

    const allocator = gpa.allocator();

    var c = try Context.init(allocator);
    defer c.deinit(); // There. Can't leak. Isn't Zig wonderful?

    var buffer: [1000]u8 = undefined;
    var fbs = std.io.fixedBufferStream(&buffer);
    var writer = fbs.writer();
    try c.server_path("simple-context-test", writer);
    var path = fbs.getWritten();

    // SERVER SIDE: creating a service.
    var server = try c.server_init(path);
    defer server.deinit();
    defer std.fs.cwd().deleteFile(path) catch {}; // Once done, remove file.

    // print ("Context: {}\n", .{c});
    // print("\n", .{});
    const t = try std.Thread.spawn(.{}, CommunicationTestThread.clientFn, .{});
    defer t.join();

    // Server.accept returns a net.StreamServer.Connection.
    var client = try server.accept();
    defer client.stream.close();
    var buf: [16]u8 = undefined;
    const n = try client.stream.reader().read(&buf);

    try testing.expectEqual(@as(usize, 12), n);
    try testing.expectEqualSlices(u8, "Hello world!", buf[0..n]);
}

// Creating a new thread: testing UNIX communication.
// This is a client sending a an instance of Message.
const ConnectThenSendMessageThread = struct {
    fn clientFn() !void {
        const config = .{.safety = true};
        var gpa = std.heap.GeneralPurposeAllocator(config){};
        defer _ = gpa.deinit();
        const allocator = gpa.allocator();

        var c = try Context.init(allocator);
        defer c.deinit(); // There. Can't leak. Isn't Zig wonderful?

        var path_buffer: [1000]u8 = undefined;
        var path_fbs = std.io.fixedBufferStream(&path_buffer);
        var path_writer = path_fbs.writer();
        try c.server_path("simple-context-test", path_writer);
        var path = path_fbs.getWritten();

        // Actual UNIX socket connection.
        const socket = try net.connectUnixSocket(path);
        defer socket.close();

        // Writing message into a buffer.
        var message_buffer: [1000]u8 = undefined;
        var message_fbs = std.io.fixedBufferStream(&message_buffer);
        var message_writer = message_fbs.writer();
        // 'fd' parameter is not taken into account here (no loop)

        var m = try Message.init(0, Message.Type.DATA, allocator, "Hello world!");
        defer m.deinit();
        _ = try m.write(message_writer);

        // print("So we're a client now... path: {s}\n", .{path});
        _ = try socket.writer().writeAll(message_fbs.getWritten());
    }
};


 test "Context - creation, echo once" {
     const config = .{.safety = true};
     var gpa = std.heap.GeneralPurposeAllocator(config){};
     defer _ = gpa.deinit();

     const allocator = gpa.allocator();

     var c = try Context.init(allocator);
     defer c.deinit(); // There. Can't leak. Isn't Zig wonderful?

     var buffer: [1000]u8 = undefined;
     var fbs = std.io.fixedBufferStream(&buffer);
     var writer = fbs.writer();
     try c.server_path("simple-context-test", writer);
     var path = fbs.getWritten();

     // SERVER SIDE: creating a service.
     var server = try c.server_init(path);
     defer server.deinit();
     defer std.fs.cwd().deleteFile(path) catch {}; // Once done, remove file.

     const t = try std.Thread.spawn(.{}, ConnectThenSendMessageThread.clientFn, .{});
     defer t.join();

     // Server.accept returns a net.StreamServer.Connection.
     var client = try server.accept();
     defer client.stream.close();
     var buf: [1000]u8 = undefined;
     const n = try client.stream.reader().read(&buf);
     var m = try Message.read(buf[0..n], allocator);
     defer m.deinit();

     try testing.expectEqual(@as(usize, 12), m.payload.len);
     try testing.expectEqualSlices(u8, m.payload, "Hello world!");
 }

