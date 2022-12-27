const std = @import("std");
const hexdump = @import("./hexdump.zig");
const net = std.net;
const fmt = std.fmt;

const ipc = @import("./main.zig");

const print = std.debug.print;
const testing = std.testing;
const print_eq = @import("./util.zig").print_eq;

// TODO: file descriptors should have a specific type (but i32 is used in std.net...).

// TODO: path => std.XXX.YYY, not simple [] const u8

// TODO: both Connection and pollfd store file descriptors.
//       Connection stores either Stream (server) or Address (client).

// TODO: API should completely obfuscate the inner structures.
//       Only structures in this file should be necessary.

var should_quit: bool = false;

fn create_service() !void {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var ctx = try ipc.Context.init(allocator);
    defer ctx.deinit(); // There. Can't leak. Isn't Zig wonderful?

    const path = "/tmp/.TEST_USOCK";

    // SERVER SIDE: creating a service.
    _ = try ctx.server_init(path);

    // TODO: signal handler, to quit when asked

    var some_event: ipc.Event = undefined;
    ctx.timer = 2000; // 1 second
    while(true) {
        some_event = try ctx.wait_event();
        switch (some_event.t) {
            .CONNECTION => {
                print("New connection: {}!\n", .{some_event});
            },
            .TIMER => {
                print("Timer!\n", .{});
            },
            .EXTERNAL => {
                print("Message received from a non IPC socket.\n", .{});
                print("NOT IMPLEMENTED, YET. It's a suicide, then.\n", .{});
                break;
            },
            .SWITCH => {
                print("Message to send to a corresponding fd.\n", .{});
                print("NOT IMPLEMENTED, YET. It's a suicide, then.\n", .{});
                break;
            },
            .DISCONNECTION => {
                print("User disconnected.\n", .{});
            },
            .MESSAGE => {
                print("New message. {}\n", .{some_event});
                print("Let's echo, once\n", .{});
                if (some_event.m) |m| {
                    try ctx.schedule(m);
                }
            },
            .LOOKUP => {
                print("Client asking for a service through ipcd.\n", .{});
                print("NOT IMPLEMENTED, YET. It's a suicide, then.\n", .{});
                break;
            },
            .TX => {
                print("Message sent.\n", .{});
            },
            .NOT_SET => {
                print("Event type not set. Something is wrong, let's suicide.\n", .{});
                break;
            },
            .ERROR => {
                print("A problem occured, event: {}, let's suicide\n", .{some_event});
                break;
            },
        }

        if (should_quit) {
            break;
        }
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
