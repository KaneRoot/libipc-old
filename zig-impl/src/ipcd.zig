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

// TODO: standard library is unecessary complex regarding networking.
//       Should libipc drop it and use plain old file descriptors instead?

// TODO: path => std.XXX.YYY, not simple [] const u8

// TODO: both Connection and pollfd store file descriptors.
//       Connection stores either Stream (server) or Address (client).

// TODO: API should completely obfuscate the inner structures.
//       Only structures in this file should be necessary.

fn create_service() !void {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var ctx = try ipc.Context.init(allocator);
    defer ctx.deinit(); // There. Can't leak. Isn't Zig wonderful?

    // SERVER SIDE: creating a service.
    _ = try ctx.server_init("ipcd");

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

            .LOOKUP => {
                print("Client asking for a service through ipcd.\n", .{});
                if (some_event.m) |m| {
                    print("Message: {}\n", .{m});

                    // 1. split message
                    print("payload is: {s}\n", .{m.payload});
                    var iterator = std.mem.split(u8, m.payload, ";");
                    var service_to_contact = iterator.first();
                    print("service to contact: {s}\n", .{service_to_contact});
                    var final_destination: ?[]const u8 = null;

                    // 2. find relevant part of the message
                    while (iterator.next()) |next| {
                        print("next part: {s}\n", .{next});
                        var iterator2 = std.mem.split(u8, next, " ");
                        var sname = iterator2.first();
                        var target = iterator2.next();
                        if (target) |t| {
                            print ("sname: {s} - target: {s}\n", .{sname, t});
                            if (std.mem.eql(u8, service_to_contact, sname)) {
                                final_destination = t;
                            }
                        }
                        else {
                            print("ERROR: no target in: {s}\n", .{next});
                        }
                    }
                    // 3. connect whether asked to and send a message
                    // TODO: currently only switching with other UNIX sockets ^^'.
                    //       Should include TCP connections in a near future.

                    if (final_destination) |dest| {
                        print("service IPCd should contact for the client: {s}, via {s}\n"
                            , .{service_to_contact, dest});

                        var newfd = try ctx.connect_service (dest);
                        send_fd (some_event.origin, "ok", newfd);
                        print("fd sent\n" , .{});
                        try ctx.close_fd (some_event.origin);
                        print("FD 1 removed\n" , .{});
                        try ctx.close_fd (newfd);
                        print("FDs removed\n" , .{});
                    }

                    m.deinit();
                }
                else {
                    // There is a problem: ipcd was contacted without providing
                    // a message, meaning there is nothing to do. This should be
                    // explicitely warned about.
                    var response = try Message.init(some_event.origin
                                , Message.Type.ERROR
                                , allocator
                                , "lookup message without data");
                    try ctx.write(response);
                    response.deinit();
                }
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
    }

    print("Goodbye\n", .{});
}

pub fn main() !u8 {
    try create_service();
    return 0;
}
