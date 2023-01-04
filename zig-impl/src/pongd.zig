const std = @import("std");
const hexdump = @import("./hexdump.zig");
const net = std.net;
const fmt = std.fmt;
const os = std.os;

const ipc = @import("./main.zig");
const Message = ipc.Message;

// Import send_fd this way in order to produce docs for exchange-fd functions.
const exchange_fd = @import("./exchange-fd.zig");
const send_fd = exchange_fd.send_fd;

const builtin = @import("builtin");
const native_os = builtin.target.os.tag;
const print = std.debug.print;
const testing = std.testing;
const print_eq = @import("./util.zig").print_eq;

fn create_service() !void {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var ctx = try ipc.Context.init(allocator);
    defer ctx.deinit(); // There. Can't leak. Isn't Zig wonderful?

    // SERVER SIDE: creating a service.
    _ = try ctx.server_init("pong");

    // signal handler, to quit when asked
    const S = struct {
        var should_quit: bool = false;

        fn handler(sig: i32, info: *const os.siginfo_t, _: ?*const anyopaque) callconv(.C) void {
            print ("A signal has been received: {}\n", .{sig});
            // Check that we received the correct signal.
            switch (native_os) {
                .netbsd => {
                    if (sig != os.SIG.HUP or sig != info.info.signo)
                        return;
                },
                else => {
                    if (sig != os.SIG.HUP and sig != info.signo)
                        return;
                },
            }
            should_quit = true;
        }
    };

    var sa = os.Sigaction{
        .handler = .{ .sigaction = &S.handler },
        .mask = os.empty_sigset, // Do not mask any signal.
        .flags = os.SA.SIGINFO,
    };

    // Quit on SIGHUP (kill -1).
    try os.sigaction(os.SIG.HUP, &sa, null);

    var some_event: ipc.Event = undefined;
    ctx.timer = 10000; // 10 seconds
    var count: u32 = 0;
    while(! S.should_quit) {
        some_event = try ctx.wait_event();
        switch (some_event.t) {
            .TIMER => {
                print("\rTimer! ({})", .{count});
                count += 1;
            },

            .CONNECTION => {
                print("New connection: {} so far!\n", .{ctx.pollfd.items.len});
            },

            .DISCONNECTION => {
                print("User {} disconnected, {} remainaing.\n"
                    , .{some_event.origin, ctx.pollfd.items.len});
            },

            .MESSAGE => {
                if (some_event.m) |m| {
                    print("New message: {}\n", .{m});
                    print("Let's echo it...\n", .{});
                    try ctx.schedule(m);
                }
                else {
                    print("Error while receiving new message.\n", .{});
                    print("Ignoring...\n", .{});
                }
            },

            .TX => {
                print("Message sent.\n", .{});
            },

            else => {
                print("Error: unexpected event: {}\n", .{some_event});
                print("Let's suicide.\n", .{});
                break;
            },
        }
    }

    print("Goodbye\n", .{});
}

pub fn main() !u8 {
    try create_service();
    return 0;
}
