const std = @import("std");
const hexdump = @import("./hexdump.zig");
const testing = std.testing;
const net = std.net;
const fmt = std.fmt;

// TODO: file descriptors should have a specific type (but i32 is used in std.net...).

// TODO: path => std.XXX.YYY, not simple [] const u8

// TODO: both Connection and pollfd store file descriptors.
//       Connection stores either Stream (server) or Address (client).

// TODO: API should completely obfuscate the inner structures.
//       Only structures in this file should be necessary.

const CBEvent = @import("./callback.zig").CBEvent;
const Connection = @import("./connection.zig").Connection;
const Message = @import("./message.zig").Message;
const Event = @import("./event.zig").Event;
const Switch = @import("./switch.zig").Switch;
const print_eq = @import("./util.zig").print_eq;

const print = std.debug.print;

const Messages = @import("./message.zig").Messages;
const Switches = @import("./switch.zig").Switches;
const Connections = @import("./connection.zig").Connections;
const Context = @import("./context.zig").Context;
pub const PollFD = std.ArrayList(i32);

test {
    _ = @import("./callback.zig");
    _ = @import("./connection.zig");
    _ = @import("./context.zig");
    _ = @import("./event.zig");
    _ = @import("./message.zig");
    _ = @import("./switch.zig");
    _ = @import("./util.zig");
}

fn create_service() !void {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();

    const allocator = gpa.allocator();
    var ctx = try Context.init(allocator);
    defer ctx.deinit(); // There. Can't leak. Isn't Zig wonderful?

    const path = "/tmp/.TEST_USOCK";

    // SERVER SIDE: creating a service.
    _ = try ctx.server_init(path);

    var some_event = try ctx.wait_event();
    switch (some_event.t) {
        .CONNECTION => {
            print("New connection!\n", .{});
        },
        else => {
            print("New event: {}\n", .{some_event.t});
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
