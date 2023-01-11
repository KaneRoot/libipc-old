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

// Standard library is unecessary complex regarding networking.
// libipc drops it and uses plain old file descriptors instead.

// API should completely obfuscate the inner structures.
// Only libipc structures should be necessary to write any networking code,
// users should only work with Context and Message, mostly.

// QUESTION: should libipc use std.fs.path and not simple [] const u8?

fn create_service() !void {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var ctx = try ipc.Context.init(allocator);
    defer ctx.deinit(); // There. Can't leak. Isn't Zig wonderful?

    // SERVER SIDE: creating a service.
    _ = try ctx.server_init("ipc");

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
    ctx.timer = 1000; // 1 second
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

            .SWITCH_RX => {
                print("Message has been received (SWITCH).\n", .{});
                print("NOT IMPLEMENTED, YET. It's a suicide, then.\n", .{});
                break;
            },

            .SWITCH_TX => {
                print("Message has been sent (SWITCH).\n", .{});
                print("NOT IMPLEMENTED, YET. It's a suicide, then.\n", .{});
                break;
            },

            .MESSAGE => {
                print("Client asking for a service through ipcd.\n", .{});
                defer ctx.close_fd (some_event.origin) catch {};
                if (some_event.m) |m| {
                    print("{}\n", .{m});
                    defer m.deinit(); // Do not forget to free the message payload.

                    // 1. split message
                    var iterator = std.mem.split(u8, m.payload, ";");
                    var service_to_contact = iterator.first();
                    // print("service to contact: {s}\n", .{service_to_contact});
                    var final_destination: ?[]const u8 = null;

                    // 2. find relevant part of the message
                    while (iterator.next()) |next| {
                        // print("next part: {s}\n", .{next});
                        var iterator2 = std.mem.split(u8, next, " ");
                        var sname = iterator2.first();
                        var target = iterator2.next();
                        if (target) |t| {
                            // print ("sname: {s} - target: {s}\n", .{sname, t});
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
                    //       Should contact <protocol>d.

                    if (final_destination) |dest| {
                        print("Let's contact {s} (original service requested: {s})\n"
                            , .{dest, service_to_contact});

                        var iterator3 = std.mem.split(u8, dest, "://");
                        var protocol = iterator3.first();
                        print("Protocol: {s}\n" , .{protocol});

                        // 1. in case there is no URI
                        if (std.mem.eql(u8, protocol, dest)) {
                            var newfd = try ctx.connect_service (dest);
                            send_fd (some_event.origin, "ok", newfd);
                            try ctx.close_fd (newfd);
                        }
                        else if (std.mem.eql(u8, protocol, "unix")) {
                            var newfd = try ctx.connect_service (iterator3.next().?);
                            send_fd (some_event.origin, "ok", newfd);
                            try ctx.close_fd (newfd);
                        }
                        // 2. else, contact <protocol>d or directly the dest in case there is none.
                        else {
                            print("should contact {s}d: TODO\n", .{protocol});
                            var servicefd = try ctx.connect_service (protocol);
                            defer ctx.close_fd (servicefd) catch {};
                            // TODO: make a simple protocol between IPCd and <protocol>d
                            //       NEED inform about the connection (success or fail)
                            //       FIRST DRAFT:
                            //       - IPCd: send a message containing the destination
                            //       - PROTOCOLd: send "ok" to inform the connection is established
                            //       - PROTOCOLd: send "no" in case there was an error

                            var message = try Message.init(servicefd, allocator, dest);
                            defer message.deinit();
                            try ctx.write(message);
                            var response_from_service = try ctx.read_fd(servicefd);
                            if (response_from_service) |r| {
                                defer r.deinit();
                                if (std.mem.eql(u8, r.payload, "ok")) {
                                    // OK
                                    print("service has established the connection\n", .{});
                                    send_fd (some_event.origin, "ok", servicefd);
                                }
                                else if (std.mem.eql(u8, r.payload, "ne")) {
                                    // PROBLEM
                                    print("service cannot establish the connection\n", .{});
                                    // TODO
                                }
                                else {
                                    print("service isn't working properly, its response is: {s}\n", .{r.payload});
                                    // TODO
                                }
                            }
                            else {
                                // No message = should be handled as a disconnection.
                                print("No response from service: let's drop everything\n", .{});
                            }
                        }
                    }
                }
                else {
                    // There is a problem: ipcd was contacted without providing
                    // a message, meaning there is nothing to do. This should be
                    // explicitely warned about.
                    var response = try Message.init(some_event.origin
                                , allocator
                                , "lookup message without data");
                    defer response.deinit();
                    try ctx.write(response);
                }
            },

            .TX => {
                print("Message sent.\n", .{});
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
