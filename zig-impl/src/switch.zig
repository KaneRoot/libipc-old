const std = @import("std");
const testing = std.testing;
const fmt = std.fmt;

const net = std.net;

const ipc = @import("./main.zig");
const Message  = ipc.Message;
const CBEventType = ipc.CBEvent.Type;

const Allocator = std.mem.Allocator;

const print_eq = @import("./util.zig").print_eq;
const print = std.debug.print;

const Event = ipc.Event;

pub const Switches = struct {
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

    // Read message from a switched fd.
    pub fn read (self: *Self, fd: i32) !?Message {
        var managedconnection = self.db.get(fd);
        var message: Message = undefined;

        var r: CBEventType = managedconnection.in(fd, &message);

        switch (r) {
            // The message should be ignored (protocol specific).
            .IGNORE => { return null; },
            // No error. A message was generated.
            .NO_ERROR => {
                message.fd = managedconnection.dest;
                return message;
            },
            .FD_CLOSING => { return error.closeFD; },
            // Generic error, or the message was read but with errors.
            .FD_ERROR
            .PARSING_ERROR => {
                return error.generic;
            },
        }

        unreachable;
    }

    // Write a message to a switched fd.
    pub fn write (self: *Self, message: Message) !void {

        var managedconnection = self.db.get(message.fd);
        var r = managedconnection.out(managedconnection.dest, message);

        switch (r) {
            // The message should be ignored (protocol specific).
            // No error. A message was generated.
            .NO_ERROR => {
                return;
            },
            .FD_CLOSING => { return error.closeFD; },
            // Generic error, or the message was read but with errors.
            .IGNORE
            .FD_ERROR
            .PARSING_ERROR => {
                return error.generic;
            },
        }

        unreachable;
    }

    pub fn handle_event_read (self: *Self, event: *Event, index: usize, fd: i32) void {
        var message: ?Message = null;
        message = self.read (fd) catch |err| switch(err) {
            error.closeFD => {
                event.* = Event.init(Event.Type.DISCONNECTION, index, fd, null);
                return;
            },
            error.generic => {
                event.* = Event.init(Event.Type.ERROR, index, fd, null);
                return;
            },
        };
        event.* = Event.init(Event.Type.SWITCH_RX, index, fd, message);
        return;
    }

    pub fn handle_event_write (self: *Self, event: *Event, index: usize, message: Message) void {
        defer message.deinit();
        var fd = message.fd;
        self.write(message) catch |err| switch(err) {
            error.closeFD => {
                event.* = Event.init(Event.Type.DISCONNECTION, index, fd, null);
                return;
            },
            error.generic => {
                event.* = Event.init(Event.Type.ERROR, index, fd, null);
                return;
            },
        };
        event.* = Event.init(Event.Type.SWITCH_TX, index, fd, null);
        return;
    }
};

pub const ManagedConnection = struct {
    dest : i32,
    in  : *const fn (origin: i32, m: *Message) CBEventType = default_in,
    out : *const fn (origin: i32, m: *Message) CBEventType = default_out,
};

test "creation and display" {
   const config = .{.safety = true};
   var gpa = std.heap.GeneralPurposeAllocator(config){};
   defer _ = gpa.deinit();
   const allocator = gpa.allocator();

   var switchdb = Switches.init(allocator);
   defer switchdb.deinit();

   try switchdb.db.put(5, ManagedConnection {.dest = 6});
   try switchdb.db.put(6, ManagedConnection {.dest = 5});

   try print_eq("{ (5,6)(6,5) }", .{switchdb});
}

// test "read" {
//    const config = .{.safety = true};
//    var gpa = std.heap.GeneralPurposeAllocator(config){};
//    defer _ = gpa.deinit();
//    const allocator = gpa.allocator();
// 
//    var switchdb = Switches.init(allocator);
//    defer switchdb.deinit();
// 
// }

// For IO callbacks (switching).
// pub const Type = enum {
//     NO_ERROR,      // No error. A message was generated.
//     FD_CLOSING,    // The fd is closing.
//     FD_ERROR,      // Generic error.
//     PARSING_ERROR, // The message was read but with errors.
//     IGNORE,        // The message should be ignored (protocol specific).
// };

fn default_in (origin: i32, m: *Message) CBEventType {
    print ("receiving a message originated from {}\n", .{origin});
    var buffer: [2000000]u8 = undefined; // TODO: FIXME??
    var packet_size: usize = undefined;

    // This may be kinda hacky, idk.
    var stream: net.Stream = .{ .handle = origin };
    packet_size = stream.read(buffer[0..]) catch return CBEventType.FD_ERROR;

    // Let's handle this as a disconnection.
    if (packet_size <= 4) {
        return CBEventType.FD_CLOSING;
    }

    // TODO: handle memory errors.
    m.* = Message.read(origin, buffer[0..], std.heap.c_allocator)
          catch return CBEventType.FD_ERROR;

    return CBEventType.NO_ERROR;
}

fn default_out (origin: i32, m: *Message) CBEventType {
    print ("sending a message originated from {}\n", .{origin});
    // Message contains the fd, no need to search for
    // the right structure to copy, let's just recreate
    // a Stream from the fd.
    var stream = net.Stream { .handle = m.fd };

    var buffer: [200000]u8 = undefined; // TODO: buffer size
    var fbs = std.io.fixedBufferStream(&buffer);
    var writer = fbs.writer();

    // returning basic errors, no details.
    _ = m.write(writer) catch return CBEventType.FD_ERROR;
    _ = stream.write (fbs.getWritten()) catch return CBEventType.FD_ERROR;

    return CBEventType.NO_ERROR;
}

// // TODO: actual switching.
// pub const Switch = struct {
//     origin      : i32,
//     destination : i32,
//
//     orig_in  : *const fn (origin: i32, m: *Message) CBEventType,
//     orig_out : *const fn (origin: i32, m: *Message) CBEventType,
//     dest_in  : *const fn (origin: i32, m: *Message) CBEventType,
//     dest_out : *const fn (origin: i32, m: *Message) CBEventType,
//
//     const Self = @This();
//
//     pub fn init(origin: i32, destination: i32) Self {
//         return Self {
//              .origin = origin
//            , .destination = destination
//            , .orig_in  = default_in
//            , .orig_out = default_out
//            , .dest_in  = default_in
//            , .dest_out = default_out
//         };
//     }
//
//     pub fn set_callbacks(self: *Self, fd: i32,
//         in  : *const fn (origin: i32, m: *Message) CBEventType,
//         out : *const fn (origin: i32, m: *Message) CBEventType) void {
//
//         if (fd == self.origin) {
//             self.orig_in = in;
//             self.orig_out = out;
//         }
//         else {
//             self.dest_in = in;
//             self.dest_out = out;
//         }
//     }
//
//     pub fn format(self: Self, comptime _: []const u8, _: fmt.FormatOptions, out_stream: anytype) !void {
//         try fmt.format(out_stream
//             , "switch {} <-> {}"
//             , .{ self.origin, self.destination} );
//     }
// };

// test "Switch - creation and display" {
//     const config = .{.safety = true};
//     var gpa = std.heap.GeneralPurposeAllocator(config){};
//     defer _ = gpa.deinit();
//     const allocator = gpa.allocator();
//
//     var switchdb = Switches.init(allocator);
//     defer switchdb.deinit();
//
//     var first = Switch.init(3,8); // origin destination
//     var second = Switch.init(2,4); // origin destination
//     try switchdb.append(first);
//     try switchdb.append(second);
//
//     try print_eq("switch 3 <-> 8", first);
//     try print_eq("switch 2 <-> 4", second);
//
//     try testing.expect(2 == switchdb.items.len);
// }
