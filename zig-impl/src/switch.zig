const std = @import("std");
const testing = std.testing;
const fmt = std.fmt;

const net = std.net;

const ipc = @import("./main.zig");
const Message  = ipc.Message;
const CBEventType = ipc.CBEvent.Type;

const Allocator = std.mem.Allocator;

const util = @import("./util.zig");
const print_eq = util.print_eq;
const log = std.log.scoped(.libipc_switch);

const Event = ipc.Event;

/// SwitchDB: store relations between clients and services.
///
/// A protocol service, such as TPCd can handle "external" communications (TCP in this case)
/// meaning that a client can connect to this service through a canal that isn't a simple
/// libipc UNIX socket, and this client is then connected to a local service.
/// OTOH, a local client can ask TCPd to establish a connection to a remote service.
/// In both cases, at least one of the connection isn't libipc-based and should be
/// handled in a specific way that only TPCd (or another protocol service) can.
///
/// TCPd marks both file descriptors as "related" (add_switch) so libipc can automatically
/// handle messages between the client and the service. Any input from one end will be sent
/// to the other.
/// TCPd registers functions to handle specific input and output operations from and to the
/// remote connection (set_callbacks).
///
/// At any point, TCPd can safely close a connection and remote it from the SwitchDB (nuke),
/// resulting in the removal of both the connection's FD and its related FD (both the client
/// and the service connections are removed).
///
/// Currently, libipc automatically closes both the client and its service when an error occurs.
pub const SwitchDB = struct {
    const Self = @This();

    db: std.AutoArrayHashMap(i32, ManagedConnection),

    pub fn init (allocator: Allocator) Self {
        return Self {
            .db = std.AutoArrayHashMap(i32, ManagedConnection).init(allocator),
        };
    }

    pub fn deinit (self: *Self) void {
        self.db.deinit();
    }

    pub fn format(self: Self, comptime _: []const u8, _: fmt.FormatOptions, out_stream: anytype) !void {
        for(self.db.keys()) |k,i| {
            try fmt.format(out_stream, "({},{})", .{k, self.db.values()[i].dest});
        }
    }

    pub fn add_switch(self: *Self, fd1: i32, fd2: i32) !void {
        try self.db.put(fd1, ManagedConnection {.dest = fd2});
        try self.db.put(fd2, ManagedConnection {.dest = fd1});
    }

    pub fn set_callbacks(self: *Self, fd: i32
        , in  : *const fn (origin: i32, mcontent: [*]u8,       mlen: *u32) CBEventType
        , out : *const fn (origin: i32, mcontent: [*]const u8, mlen: u32)  CBEventType) !void {
        var managedconnection = self.db.get(fd) orelse return error.unregisteredFD;
        managedconnection.in = in;
        managedconnection.out = out;
    }

    /// Dig the "db" hashmap, perform "in" fn, may provide a message.
    /// Errors from the "in" fn are reported as Zig errors.
    pub fn read (self: *Self, fd: i32) !?Message {
        // assert there is an entry with this fd as a key.
        var managedconnection = self.db.get(fd) orelse return error.unregisteredFD;

        var buffer = [_]u8{0} ** 100000; // TODO: buffer size
        var message_size: u32 = @truncate(u32, buffer.len);
        var r: CBEventType = managedconnection.in(fd, &buffer, &message_size);

        switch (r) {
            // The message should be ignored (protocol specific).
            CBEventType.IGNORE     => { return null; },
            CBEventType.NO_ERROR   => {
                // TODO: read message
                // TODO: better allocator?
                // TODO: better errors?
                var message: Message
                    = Message.read(managedconnection.dest
                                  , buffer[0..message_size]
                                  , std.heap.c_allocator) catch {
                    return error.generic;
                };
                return message;
            },
            CBEventType.FD_CLOSING => { return error.closeFD; },
            // Generic error, or the message was read but with errors.
            CBEventType.ERROR      => { return error.generic; },
        }

        unreachable;
    }

    /// Dig the "db" hashmap and perform "out" fn.
    /// Errors from the "out" fn are reported as Zig errors.
    pub fn write (self: *Self, message: Message) !void {
        // assert there is an entry with this fd as a key.
        var managedconnection = self.db.get(message.fd) orelse return error.unregisteredFD;

        var buffer = [_]u8{0} ** 100000; // TODO: buffer size
        var fbs = std.io.fixedBufferStream(&buffer);
        var writer = fbs.writer();

        // returning basic errors, no details.
        _ = message.write(writer) catch return error.generic;
        var written = fbs.getWritten();

        var r = managedconnection.out(message.fd, written.ptr, @truncate(u32, written.len));

        switch (r) {
            // The message should be ignored (protocol specific).
            // No error. A message was generated.
            CBEventType.NO_ERROR => {
                return;
            },
            CBEventType.FD_CLOSING => { return error.closeFD; },
            // Generic error, or the message was read but with errors.
            CBEventType.IGNORE,
            CBEventType.ERROR => {
                return error.generic;
            },
        }

        unreachable;
    }

    /// From a message to read on a socket to an Event.
    pub fn handle_event_read (self: *Self, index: usize, fd: i32) Event {
        var message: ?Message = null;
        message = self.read (fd) catch |err| switch(err) {
            error.closeFD => {
                return Event.init(Event.Type.DISCONNECTION, index, fd, null);
            },
            error.unregisteredFD,
            error.generic => {
                return Event.init(Event.Type.ERROR, index, fd, null);
            },
        };
        return Event.init(Event.Type.SWITCH_RX, index, fd, message);
    }

    /// Message is free'd in any case.
    pub fn handle_event_write (self: *Self, index: usize, message: Message) Event {
        defer message.deinit();
        var fd = message.fd;
        self.write(message) catch |err| switch(err) {
            error.closeFD => {
                return Event.init(Event.Type.DISCONNECTION, index, fd, null);
            },
            error.unregisteredFD,
            error.generic => {
                return Event.init(Event.Type.ERROR, index, fd, null);
            },
        };
        return Event.init(Event.Type.SWITCH_TX, index, fd, null);
    }

    /// Simple wrapper around self.db.get.
    pub fn getDest (self: *Self, fd: i32) !i32 {
        return self.db.get(fd).?.dest;
    }

    /// Remove both entries (client and service) from the DB.
    pub fn nuke (self: *Self, fd: i32) void {
        if (self.db.fetchSwapRemove(fd)) |kv| {
            _ = self.db.swapRemove(kv.value.dest);
        }
    }
};

const ManagedConnection = struct {
    dest : i32,
    in  : *const fn (origin: i32, mcontent: [*]u8,       mlen: *u32) CBEventType = default_in,
    out : *const fn (origin: i32, mcontent: [*]const u8, mlen: u32)  CBEventType = default_out,
};

test "creation and display" {
   const config = .{.safety = true};
   var gpa = std.heap.GeneralPurposeAllocator(config){};
   defer _ = gpa.deinit();
   const allocator = gpa.allocator();

   var switchdb = SwitchDB.init(allocator);
   defer switchdb.deinit();

   try switchdb.db.put(5, ManagedConnection {.dest = 6});
   try switchdb.db.put(6, ManagedConnection {.dest = 5});

   try print_eq("{ (5,6)(6,5) }", .{switchdb});
}

fn successful_in (_: i32, mcontent: [*]u8, mlen: *u32) CBEventType {
    var m = Message.init(8, std.heap.c_allocator, "coucou") catch unreachable;
    defer m.deinit();

    var fbs = std.io.fixedBufferStream(mcontent[0..mlen.*]);
    var writer = fbs.writer();
    const bytes_written = m.write (writer) catch unreachable;
    mlen.* = @truncate(u32, bytes_written);
    return CBEventType.NO_ERROR;
}

fn successful_out (_: i32, _: [*]const u8, _: u32) CBEventType {
    return CBEventType.NO_ERROR;
}

test "successful exchanges" {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var switchdb = SwitchDB.init(allocator);
    defer switchdb.deinit();

    try switchdb.db.put(5, ManagedConnection {.dest = 6, .in = successful_in, .out = successful_out});
    try switchdb.db.put(6, ManagedConnection {.dest = 5, .in = successful_in, .out = successful_out});

    // should return a new message (hardcoded: fd 8, payload "coucou")
    var event_1: Event = switchdb.handle_event_read (1, 5);
    if (event_1.m) |m| { m.deinit(); }
    else               { return error.NoMessage; }

    // should return a new message (hardcoded: fd 8, payload "coucou")
    var event_2: Event = switchdb.handle_event_read (1, 6);
    if (event_2.m) |m| { m.deinit(); }
    else               { return error.NoMessage; }

    var message = try Message.init(6, allocator, "coucou");
    var event_3 = switchdb.handle_event_write (5, message);
    if (event_3.m) |_| { return error.ShouldNotCarryMessage; }
}

fn unsuccessful_in (_: i32, _: [*]const u8, _: *u32) CBEventType {
    return CBEventType.ERROR;
}

fn unsuccessful_out (_: i32, _: [*]const u8, _: u32) CBEventType {
    return CBEventType.ERROR;
}

test "unsuccessful exchanges" {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var switchdb = SwitchDB.init(allocator);
    defer switchdb.deinit();

    try switchdb.db.put(5, ManagedConnection {.dest = 6, .in = unsuccessful_in, .out = unsuccessful_out});
    try switchdb.db.put(6, ManagedConnection {.dest = 5, .in = unsuccessful_in, .out = unsuccessful_out});

    // should return a new message (hardcoded: fd 8, payload "coucou")
    var event_1: Event = switchdb.handle_event_read (1, 5);
    if (event_1.m) |_| { return error.ShouldNotCarryMessage; }

    // should return a new message (hardcoded: fd 8, payload "coucou")
    var event_2: Event = switchdb.handle_event_read (1, 6);
    if (event_2.m) |_| { return error.ShouldNotCarryMessage; }

    var message = try Message.init(6, allocator, "coucou");
    var event_3 = switchdb.handle_event_write (5, message);
    if (event_3.m) |_| { return error.ShouldNotCarryMessage; }
}

test "nuke 'em" {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var switchdb = SwitchDB.init(allocator);
    defer switchdb.deinit();

    try switchdb.db.put(5, ManagedConnection {.dest = 6, .in = unsuccessful_in, .out = unsuccessful_out});
    try switchdb.db.put(6, ManagedConnection {.dest = 5, .in = unsuccessful_in, .out = unsuccessful_out});

    try testing.expect(switchdb.db.count() == 2);
    switchdb.nuke(5);
    try testing.expect(switchdb.db.count() == 0);
}

fn default_in (origin: i32, mcontent: [*]u8, mlen: *u32) CBEventType {
    // This may be kinda hacky, idk.
    var stream: net.Stream = .{ .handle = origin };
    var packet_size: usize = stream.read(mcontent[0..mlen.*]) catch return CBEventType.ERROR;

    // Let's handle this as a disconnection.
    if (packet_size < 4) {
        log.debug("message is less than 4 bytes ({} bytes)", .{packet_size});
        return CBEventType.FD_CLOSING;
    }

    mlen.* = @truncate(u32, packet_size);

    return CBEventType.NO_ERROR;
}

fn default_out (fd: i32, mcontent: [*]const u8, mlen: u32) CBEventType {
    // Message contains the fd, no need to search for the right structure to copy,
    // let's just recreate a Stream from the fd.

    var to_send = mcontent[0..mlen];
    var stream = net.Stream { .handle = fd };
    _ = stream.write (to_send) catch return CBEventType.ERROR;
    return CBEventType.NO_ERROR;
}
