const std = @import("std");
const testing = std.testing;
const net = std.net;
const os = std.os;
const fmt = std.fmt;

const log = std.log.scoped(.libipc_context);

const receive_fd = @import("./exchange-fd.zig").receive_fd;

const Timer = std.time.Timer;

const CBEvent = @import("./callback.zig").CBEvent;
const Connection = @import("./connection.zig").Connection;
const Message = @import("./message.zig").Message;
const Event = @import("./event.zig").Event;
const Switch = @import("./switch.zig").Switch;
const print_eq = @import("./util.zig").print_eq;

const Messages = @import("./message.zig").Messages;
const SwitchDB = @import("./switch.zig").SwitchDB;
const Connections = @import("./connection.zig").Connections;
const CBEventType = @import("./main.zig").CBEvent.Type;

pub const PollFD = std.ArrayList(std.os.pollfd);

pub const IPC_HEADER_SIZE = 4; // Size (4 bytes) then content.
pub const IPC_BASE_SIZE = 100000; // 100 KB, plenty enough space for messages
pub const IPC_MAX_MESSAGE_SIZE = IPC_BASE_SIZE-IPC_HEADER_SIZE;
pub const IPC_VERSION = 1;

// Context of the whole networking state.
pub const Context = struct {
    rundir: [] u8,
    allocator: std.mem.Allocator,  // Memory allocator.
    connections: Connections,      // Keep track of connections.

    // "pollfd" structures passed to poll(2). Same indexes as "connections".
    pollfd: PollFD,  // .fd (fd_t) + .events (i16) + .revents (i16)

    tx: Messages,       // Messages to send, once their fd is available.
    switchdb: SwitchDB, // Relations between fd.

    timer: ?i32 = null,  // No timer by default (no TIMER event).

    const Self = @This();

    // Context initialization:
    // - init structures (provide the allocator)
    pub fn init(allocator: std.mem.Allocator) !Self {
        var rundir = std.process.getEnvVarOwned(allocator, "RUNDIR") catch |err| switch(err) {
            error.EnvironmentVariableNotFound => blk: {
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
           , .switchdb = SwitchDB.init(allocator)
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
                log.err("context.deinit(): IndexOutOfBounds", .{});
            },
        };
        self.allocator.free(self.rundir);
        self.connections.deinit();
        self.pollfd.deinit();
        for (self.tx.items) |m| {
            m.deinit();
        }
        self.tx.deinit();
        self.switchdb.deinit();
    }

    // Both simple connection and the switched one share this code.
    fn connect_ (self: *Self, ctype: Connection.Type, path: []const u8) !i32 {
        var stream = try net.connectUnixSocket(path);
        const newfd = stream.handle;
        errdefer std.os.closeSocket(newfd);
        var newcon = Connection.init(ctype, null);
        try self.add_ (newcon, newfd);
        return newfd;
    }

    fn connect_ipcd (self: *Self, service_name: []const u8
                    , connection_type: Connection.Type) !?i32 {

        const buffer_size = 10000;
        var buffer: [buffer_size]u8 = undefined;
        var fba = std.heap.FixedBufferAllocator.init(&buffer);
        var allocator = fba.allocator();

        // Get IPC_NETWORK environment variable
        // IPC_NETWORK is shared with the network service to choose the protocol stack,
        // according to the target service.
        //
        // Example, connecting to 'audio' service through tor service:
        //   IPC_NETWORK="audio tor://some.example.com/audio"
        //
        // Routing directives can be chained using " ;" separator:
        //   IPC_NETWORK="audio https://example.com/audio ;pong tls://pong.example.com/pong"
        var network_envvar = std.process.getEnvVarOwned(allocator, "IPC_NETWORK") catch |err| switch(err) {
            // error{ OutOfMemory, EnvironmentVariableNotFound, InvalidUtf8 } (ErrorSet)
            error.EnvironmentVariableNotFound => {
                log.debug("no IPC_NETWORK envvar: IPCd won't be contacted", .{});
                return null;
            }, // no need to contact IPCd
            else => { return err; },
        };

        var lookupbuffer: [buffer_size]u8 = undefined;
        var lookupfbs = std.io.fixedBufferStream(&lookupbuffer);
        var lookupwriter = lookupfbs.writer();
        try lookupwriter.print("{s};{s}", .{service_name, network_envvar});

        // Try to connect to the IPCd service
        var ipcdfd = try self.connect_service("ipc");
        defer self.close_fd (ipcdfd) catch {}; // in any case, connection should be closed

        // Send LOOKUP message
        //   content: target service name;${IPC_NETWORK}
        //   example: pong;pong tls://example.com:8998/pong

        var m = try Message.init (ipcdfd, allocator, lookupfbs.getWritten());
        try self.write (m);

        // Read LOOKUP response
        //   case error: ignore and move on (TODO)
        //   else: get fd sent by IPCd then close IPCd fd
        var reception_buffer: [2000]u8 = undefined;
        var reception_size: usize = 0;
        var newfd = try receive_fd (ipcdfd, &reception_buffer, &reception_size);
        if (reception_size == 0) {
            return error.IPCdFailedNoMessage;
        }

        var response: []u8 = reception_buffer[0..reception_size];

        if (! std.mem.eql(u8, response, "ok")) {
            return error.IPCdFailedNotOk;
        }
        var newcon = Connection.init(connection_type, null);
        try self.add_ (newcon, newfd);
        return newfd;
    }

    /// TODO: Add a new connection, but takes care of memory problems:
    /// in case one of the arrays cannot sustain another entry, the other
    /// won't be added.
    fn add_ (self: *Self, new_connection: Connection, fd: os.socket_t) !void {
        try self.connections.append(new_connection);
        try self.pollfd.append(.{ .fd = fd
                                , .events = std.os.linux.POLL.IN
                                , .revents = 0 });
    }

    fn fd_to_index (self: Self, fd: i32) !usize {
        var i: usize = 0;
        while(i < self.pollfd.items.len) {
           if (self.pollfd.items[i].fd == fd) {
               return i;
           }
           i += 1;
        }
        return error.IndexNotFound;
    }

    /// Connect to the service directly, without reaching IPCd first.
    /// Return the connection FD.
    pub fn connect_service (self: *Self, service_name: []const u8) !i32 {
        var buffer: [1000]u8 = undefined;
        var fbs = std.io.fixedBufferStream(&buffer);
        var writer = fbs.writer();

        try self.server_path(service_name, writer);
        var path = fbs.getWritten();

        return self.connect_ (Connection.Type.IPC, path);
    }

    /// Tries to connect to IPCd first, then the service (if needed).
    /// Return the connection FD.
    pub fn connect_ipc (self: *Self, service_name: []const u8) !i32 {
        // First, try ipcd.
        if (try self.connect_ipcd (service_name, Connection.Type.IPC)) |fd| {
            log.debug("Connected via IPCd, fd is {}", .{fd});
            return fd;
        }
        // In case this doesn't work, connect directly.
        return try self.connect_service (service_name);
    }

    /// Add a new file descriptor to follow, labeled as EXTERNAL.
    /// Useful for protocol daemons (ex: TCPd) listening to a socket for external connections,
    /// clients trying to reach a libipc service.
    pub fn add_external (self: *Self, newfd: i32) !void {
        var newcon = Connection.init(Connection.Type.EXTERNAL, null);
        try self.add_ (newcon, newfd);
    }

    fn accept_new_client(self: *Self, event: *Event, server_index: usize) !void {
        // net.StreamServer
        var serverfd = self.pollfd.items[server_index].fd;
        var path = self.connections.items[server_index].path orelse return error.ServerWithNoPath;
        var server = net.StreamServer {
              .sockfd = serverfd
            , .kernel_backlog = 100
            , .reuse_address  = false
            , .listen_address = try net.Address.initUnix(path)
        };
        var client = try server.accept(); // net.StreamServer.Connection

        const newfd = client.stream.handle;
        var newcon = Connection.init(Connection.Type.IPC, null);
        try self.add_ (newcon, newfd);

        const sfd = server.sockfd orelse return error.SocketLOL; // TODO
        // WARNING: imply every new item is last
        event.set(Event.Type.CONNECTION, self.pollfd.items.len - 1, sfd, null);
    }

    // Create a unix socket.
    // Store std lib structures in the context.
    pub fn server_init(self: *Self, service_name: [] const u8) !net.StreamServer {
        var buffer: [1000]u8 = undefined;
        var fbs = std.io.fixedBufferStream(&buffer);
        var writer = fbs.writer();

        try self.server_path(service_name, writer);
        var path = fbs.getWritten();

        var server = net.StreamServer.init(.{});
        var socket_addr = try net.Address.initUnix(path);
        try server.listen(socket_addr);

        const newfd = server.sockfd orelse return error.SocketLOL; // TODO
        // Store the path in the Connection structure, so the UNIX socket file can be removed later.
        var newcon = Connection.init(Connection.Type.SERVER, try self.allocator.dupeZ(u8, path));
        try self.add_ (newcon, newfd);
        return server;
    }

    pub fn write (_: *Self, m: Message) !void {
        // Message contains the fd, no need to search for
        // the right structure to copy, let's just recreate
        // a Stream from the fd.
        var stream = net.Stream { .handle = m.fd };

        var buffer = [_]u8{0} ** IPC_MAX_MESSAGE_SIZE;
        var fbs = std.io.fixedBufferStream(&buffer);
        var writer = fbs.writer();

        _ = try m.write(writer); // returns paylen

        _ = try stream.write (fbs.getWritten());
    }

    pub fn schedule (self: *Self, m: Message) !void {
        try self.tx.append(m);
    }

    /// Read from a client (indexed by a FD).
    pub fn read_fd (self: *Self, fd: i32) !?Message {
        return try self.read(try self.fd_to_index (fd));
    }

    pub fn add_switch(self: *Self, fd1: i32, fd2: i32) !void {
        var index_origin = try self.fd_to_index(fd1);
        var index_destinataire = try self.fd_to_index(fd2);

        self.connections.items[index_origin].t = Connection.Type.SWITCHED;
        self.connections.items[index_destinataire].t = Connection.Type.SWITCHED;

        try self.switchdb.add_switch(fd1,fd2);
    }

    pub fn set_switch_callbacks(self: *Self, fd: i32
        , in  : *const fn (origin: i32, mcontent: [*]u8, mlen: *u32) CBEventType
        , out : *const fn (origin: i32, mcontent: [*]const u8, mlen: u32) CBEventType) !void {
        try self.switchdb.set_callbacks(fd,in, out);
    }

    pub fn read (self: *Self, index: usize) !?Message {
        if (index >= self.pollfd.items.len) {
            return error.IndexOutOfBounds;
        }

        var buffer = [_]u8{0} ** IPC_MAX_MESSAGE_SIZE;
        var packet_size: usize = undefined;

        // TODO: this is a problem from the network API in Zig,
        //       servers and clients are different, they aren't just fds.
        //       Maybe there is something to change in the API.
        if (self.connections.items[index].t == .SERVER) {
            return error.messageOnServer;
        }

        // This may be kinda hacky, idk.
        var fd = self.pollfd.items[index].fd;
        var stream: net.Stream = .{ .handle = fd };
        packet_size = try stream.read(buffer[0..]);

        // Let's handle this as a disconnection.
        if (packet_size <= 4) {
            return null;
        }

        return try Message.read(fd, buffer[0..], self.allocator);
    }

    /// Before closing the fd, test it via the 'fcntl' syscall.
    /// This is useful for switched connections: FDs could be closed without libipc being informed.
    fn safe_close_fd (self: *Self, fd: i32) void {
        var should_close = true;
        _ = std.os.fcntl(fd, std.os.F.GETFD, 0) catch {
            should_close = false;
        };
        if (should_close) {
            self.close_fd(fd) catch {};
        }
    }

    // Wait for an event.
    pub fn wait_event(self: *Self) !Event {
        var current_event: Event = Event.init(Event.Type.ERROR, 0, 0, null);
        var wait_duration: i32 = -1; // -1 == unlimited

        if (self.timer) |t| { log.debug("listening (timer: {} ms)", .{t}); wait_duration = t; }
        else                { log.debug("listening (no timer)", .{}); }

        // Make sure we listen to the right file descriptors,
        // setting POLLIN & POLLOUT flags.
        for (self.pollfd.items) |*fd| {
            fd.events |= std.os.linux.POLL.IN; // just to make sure
        }

        for (self.tx.items) |m| {
            for (self.pollfd.items) |*fd| {
                if (fd.fd == m.fd) {
                    fd.events |= std.os.linux.POLL.OUT; // just to make sure
                }
            }
        }

        // before initiate a timer
        var timer = try Timer.start();

        // Polling.
        var count: usize = undefined;

        count = try os.poll(self.pollfd.items, wait_duration);

        if (count < 0) {
            log.err("there is a problem: poll < 0", .{});
            current_event = Event.init(Event.Type.ERROR, 0, 0, null);
            return current_event;
        }

        var duration = timer.read() / 1000000; // ns -> ms
        if (count == 0) {
            if (duration >= wait_duration) {
                current_event = Event.init(Event.Type.TIMER, 0, 0, null);
            }
            else {
                // In case nothing happened, and poll wasn't triggered by time out.
                current_event = Event.init(Event.Type.ERROR, 0, 0, null);
            }
            return current_event;
        }

        // handle messages
        //   => loop over self.pollfd.items
        for (self.pollfd.items) |*fd, i| {
            // .revents is POLLIN
            if(fd.revents & std.os.linux.POLL.IN > 0) {
                // SERVER = new connection
                if (self.connections.items[i].t == .SERVER) {
                    try self.accept_new_client(&current_event, i);
                    return current_event;
                }
                // SWITCHED = send message to the right dest (or drop the switch)
                else if (self.connections.items[i].t == .SWITCHED) {
                    current_event = self.switchdb.handle_event_read (i, fd.fd);
                    switch (current_event.t) {
                        .SWITCH_RX => {
                            try self.schedule(current_event.m.?);
                        },
                        .DISCONNECTION => {
                            var dest = try self.switchdb.getDest(fd.fd);
                            log.debug("disconnection from {} -> removing {}, too", .{fd.fd, dest});
                            self.switchdb.nuke(fd.fd);
                            self.safe_close_fd(fd.fd);
                            self.safe_close_fd(dest);
                        },
                        .ERROR => {
                            var dest = try self.switchdb.getDest(fd.fd);
                            log.warn("error from {} -> removing {}, too", .{fd.fd, dest});
                            self.switchdb.nuke(fd.fd);
                            self.safe_close_fd(fd.fd);
                            self.safe_close_fd(dest);
                        },
                        else => {
                            log.warn("switch rx incoherent error: {}", .{current_event.t});
                            return error.incoherentSwitchError;
                        },
                    }
                    return current_event;
                }
                // EXTERNAL = user handles IO
                else if (self.connections.items[i].t == .EXTERNAL) {
                    return Event.init(Event.Type.EXTERNAL, i, fd.fd, null);
                }
                // otherwise = new message or disconnection
                else {
                    var maybe_message = self.read(i) catch |err| switch(err) {
                        error.ConnectionResetByPeer => {
                            log.warn("connection reset by peer", .{});
                            try self.close(i);
                            return Event.init(Event.Type.DISCONNECTION, i, fd.fd, null);
                        },
                        else => { return err; },
                    };

                    if (maybe_message) |m| {
                        return Event.init(Event.Type.MESSAGE_RX, i, fd.fd, m);
                    }

                    try self.close(i);
                    return Event.init(Event.Type.DISCONNECTION, i, fd.fd, null);
                }
            }

            // .revent is POLLOUT
            if(fd.revents & std.os.linux.POLL.OUT > 0) {
                fd.events &= ~ @as(i16, std.os.linux.POLL.OUT);

                var index: usize = undefined;
                for (self.tx.items) |m, index_| {
                    if (m.fd == self.pollfd.items[i].fd) {
                        index = index_;
                        break;
                    }
                }

                var m = self.tx.swapRemove(index);

                // SWITCHED = write message for its switch buddy (callbacks)
                if (self.connections.items[i].t == .SWITCHED) {
                    current_event = self.switchdb.handle_event_write (i, m);
                    // Message inner memory is already freed.
                    switch (current_event.t) {
                        .SWITCH_TX => {
                        },
                        .ERROR => {
                            var dest = try self.switchdb.getDest(fd.fd);
                            log.warn("error from {} -> removing {}, too", .{fd.fd, dest});
                            self.switchdb.nuke(fd.fd);
                            self.safe_close_fd(fd.fd);
                            self.safe_close_fd(dest);
                        },
                        else => {
                            log.warn("switch tx incoherent error: {}", .{current_event.t});
                            return error.incoherentSwitchError;
                        },
                    }
                    return current_event;
                }
                else {
                    // otherwise = write message for the msg.fd
                    try self.write (m);
                    m.deinit();
                    return Event.init(Event.Type.MESSAGE_TX, i, fd.fd, null);
                }
            }
            // .revent is POLLHUP
            if(fd.revents & std.os.linux.POLL.HUP > 0) {
                // handle disconnection
                current_event = Event.init(Event.Type.DISCONNECTION, i, fd.fd, null);
                try self.close(i);
                return current_event;
            }
            // if fd revent is POLLERR or POLLNVAL
            if ((fd.revents & std.os.linux.POLL.HUP  > 0) or
                (fd.revents & std.os.linux.POLL.NVAL > 0)) {
                return Event.init(Event.Type.ERROR, i, fd.fd, null);
            }
        }

        return current_event;
    }

    /// Remove a connection based on its file descriptor.
    pub fn close_fd(self: *Self, fd: i32) !void {
        try self.close(try self.fd_to_index (fd));
    }

    pub fn close(self: *Self, index: usize) !void {
        // REMINDER: connections and pollfd have the same length
        if (index >= self.pollfd.items.len) {
            return error.IndexOutOfBounds;
        }

        // close the connection and remove it from the two structures
        var con = self.connections.swapRemove(index);
        // Remove service's UNIX socket file.
        if (con.path) |path| {
            std.fs.cwd().deleteFile(path) catch {};
            self.allocator.free(path);
        }
        var pollfd = self.pollfd.swapRemove(index);
        std.os.close(pollfd.fd);

        // Remove all its non-sent messages.
        var i: usize = 0;
        while (true) {
            if (i >= self.tx.items.len)
                break;

            if (self.tx.items[i].fd == pollfd.fd) {
                var m = self.tx.swapRemove(i);
                m.deinit();
                continue;
            }

            i += 1;
        }
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
};

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
    var server = c.server_init("simple-context-test") catch |err| switch(err) {
        error.FileNotFound => {
            log.err("cannot init server at {s}", .{path});
            return err;
        },
        else => return err,
    };
    defer std.fs.cwd().deleteFile(path) catch {}; // Once done, remove file.

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

        var m = try Message.init(0, allocator, "Hello world!");
        defer m.deinit();
        _ = try m.write(message_writer);

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
    var server = c.server_init("simple-context-test") catch |err| switch(err) {
        error.FileNotFound => {
            log.err("cannot init server at {s}", .{path});
            return err;
        },
        else => return err,
    };
    defer std.fs.cwd().deleteFile(path) catch {}; // Once done, remove file.

    const t = try std.Thread.spawn(.{}, ConnectThenSendMessageThread.clientFn, .{});
    defer t.join();

    // Server.accept returns a net.StreamServer.Connection.
    var client = try server.accept();
    defer client.stream.close();
    var buf: [1000]u8 = undefined;
    const n = try client.stream.reader().read(&buf);
    var m = try Message.read(8, buf[0..n], allocator); // 8 == random client's fd number
    defer m.deinit();

    try testing.expectEqual(@as(usize, 12), m.payload.len);
    try testing.expectEqualSlices(u8, m.payload, "Hello world!");
}
