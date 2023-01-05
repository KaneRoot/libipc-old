const std = @import("std");
const hexdump = @import("./hexdump.zig");
const net = std.net;
const fmt = std.fmt;
const os = std.os;

const print = std.debug.print;
const ipc = @import("./main.zig");
const Message = ipc.Message;

pub fn main() !u8 {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var ctx = try ipc.Context.init(allocator);
    defer ctx.deinit(); // There. Can't leak. Isn't Zig wonderful?

    // The service to contact, either provided with the SERVICE envvar
    // or simply using "pong".
    var should_free_service_to_contact: bool = true;
    var service_to_contact = std.process.getEnvVarOwned(allocator, "SERVICE") catch blk: {
        should_free_service_to_contact = false;
        break :blk "pong";
    };
    defer {
        if (should_free_service_to_contact)
            allocator.free(service_to_contact);
    }

    var pongfd = try ctx.connect_ipc(service_to_contact);
    var message = try Message.init(pongfd, allocator, "bounce me");
    try ctx.schedule(message);

    var some_event: ipc.Event = undefined;
    ctx.timer = 2000; // 2 seconds
    while(true) {
        some_event = try ctx.wait_event();
        switch (some_event.t) {
            .TIMER => {
                print("Timer!\n", .{});
            },

            .MESSAGE => {
                if (some_event.m) |m| {
                    print("message has been bounced: {}\n", .{m});
                    m.deinit();
                    break;
                }
                else {
                    print("Received empty message, ERROR.\n", .{});
                    break;
                }
            },

            .TX => {
                print("Message sent.\n", .{});
            },

            else => {
                print("Unexpected event: {}, let's suicide\n", .{some_event});
                break;
            },
        }
    }

    print("Goodbye\n", .{});
    return 0;
}
