const std = @import("std");
const testing = std.testing;
const fmt = std.fmt;

const Message = @import("./message.zig").Message;

const print_eq = @import("./util.zig").print_eq;

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
        ERROR,         // A problem occured.
        CONNECTION,    // New user.
        DISCONNECTION, // User disconnected.
        MESSAGE_RX,    // New message.
        MESSAGE_TX,    // Message sent.
        TIMER,         // Timeout in the poll(2) function.
        EXTERNAL,      // Message received from a non IPC socket.
        SWITCH_RX,     // Message received from a switched FD.
        SWITCH_TX,     // Message sent to a switched fd.
    };

    t: Event.Type,
    index: usize,
    origin: i32, // socket fd
    m: ?Message,  // message

    const Self = @This();

    pub fn init(t: Event.Type, index: usize, origin: i32, m: ?Message) Self {
        return Self { .t = t, .index = index, .origin = origin, .m = m, };
    }

    pub fn set(self: *Self, t: Event.Type, index: usize, origin: i32, m: ?Message) void {
        self.t = t;
        self.index = index;
        self.origin = origin;
        self.m = m;
    }

    pub fn clean(self: *Self) void {
        self.t = Event.Type.ERROR;
        self.index = @as(usize,0);
        self.origin = @as(i32,0);
        if (self.m) |message| {
            message.deinit();
        }
        self.m = null;
    }

    pub fn format(self: Self, comptime _: []const u8, _: fmt.FormatOptions, out_stream: anytype) !void {
        try fmt.format(out_stream
            , "{}, origin: {}, index {}, message: [{?}]"
            , .{ self.t, self.origin, self.index, self.m} );
    }

};

test "Event - creation and display" {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var s = "hello!!";
    var m = try Message.init(1, allocator, s); // fd type payload
    defer m.deinit();
    var e = Event.init(Event.Type.CONNECTION, 5, 8, m); // type index origin message

    try print_eq("event.Event.Type.CONNECTION, origin: 8, index 5, message: [fd: 1, payload: [hello!!]]", e);
}
