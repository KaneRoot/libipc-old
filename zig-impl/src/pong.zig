const std = @import("std");
const hexdump = @import("./hexdump.zig");
const net = std.net;
const fmt = std.fmt;
const os = std.os;

const ipc = @import("./main.zig");

const builtin = @import("builtin");
const native_os = builtin.target.os.tag;
const print = std.debug.print;
const testing = std.testing;
const print_eq = @import("./util.zig").print_eq;

pub fn main() !u8 {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var ctx = try ipc.Context.init(allocator);
    defer ctx.deinit(); // There. Can't leak. Isn't Zig wonderful?

    // TODO: currently using a bullshit path.
    const path = "/tmp/.TEST_USOCK";

    // CLIENT SIDE: connecting to a service.
    var sfd = try ctx.connect (path);

    var first_msg = try ipc.Message.init(sfd, ipc.Message.Type.DATA, allocator, "hello this is pong!!");
    try ctx.write(first_msg);
    first_msg.deinit();

    var some_event: ipc.Event = undefined;
    ctx.timer = 10000; // 10 seconds
    while(true) {
        some_event = try ctx.wait_event();
        switch (some_event.t) {
            .TIMER => {
                print("Timer!\n", .{});
            },
            .MESSAGE => {
                if (some_event.m) |m| {
                    print("Message received: {}\n", .{m});
                    m.deinit();
                }
                else {
                    print("A message should have been received, weird\n", .{});
                }
                break;
            },
            else => {
                print("Unexpected message: {s}\n", .{some_event});
                break;
            },
        }
    }

    print("Goodbye\n", .{});
    return 0;
}
