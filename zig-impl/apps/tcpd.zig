const std = @import("std");
const net = std.net;
const fmt = std.fmt;
const os = std.os;
const testing = std.testing;
const print = std.debug.print;

const ipc = @import("ipc");
const hexdump = ipc.hexdump;
const Message = ipc.Message;

// Import send_fd this way in order to produce docs for exchange-fd functions.
const exchange_fd = ipc.exchangefd;
const send_fd = exchange_fd.send_fd;

const builtin = @import("builtin");
const native_os = builtin.target.os.tag;
const print_eq = ipc.util.print_eq;
const URI = ipc.util.URI;

fn init_tcp_server(allocator: std.mem.Allocator, server: *net.StreamServer) !i32 {
    var address = std.process.getEnvVarOwned(allocator, "ADDRESS") catch |err| switch(err) {
        error.EnvironmentVariableNotFound => blk: {
            print ("no ADDRESS envvar: TCPd will listen on 127.0.0.1:9000\n", .{});
            break :blk try allocator.dupe(u8, "127.0.0.1:9000");
        },
        else => { return err; },
    };
    defer allocator.free(address);

    var iterator = std.mem.split(u8, address, ":");
    var real_tcp_address = iterator.first();
    var real_tcp_port = try std.fmt.parseUnsigned(u16, iterator.rest(), 10);

    print ("TCP address [{s}] port [{}]\n", .{real_tcp_address, real_tcp_port});

    server.* = net.StreamServer.init(.{.reuse_address = true});
    var socket_addr = try net.Address.parseIp(real_tcp_address, real_tcp_port);
    try server.listen(socket_addr);

    const newfd = server.sockfd orelse return error.SocketLOL; // TODO
    return newfd;
}

fn create_service() !void {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var ctx = try ipc.Context.init(allocator);
    defer ctx.deinit(); // There. Can't leak. Isn't Zig wonderful?

    // SERVER SIDE: creating a service.
    var service_name = std.process.getEnvVarOwned(allocator, "IPC_SERVICE_NAME") catch |err| switch(err) {
        error.EnvironmentVariableNotFound => blk: {
            print ("no IPC_SERVICE_NAME envvar: TCPd will be named 'tcp'\n", .{});
            break :blk try allocator.dupe(u8, "tcp");
        },
        else => { return err; },
    };
    defer allocator.free(service_name);

    _ = try ctx.server_init(service_name);

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

    var server: net.StreamServer = undefined;
    var serverfd = try init_tcp_server(allocator, &server);
    try ctx.add_external (serverfd);

    var some_event: ipc.Event = undefined;
    var previous_event: ipc.Event.Type = ipc.Event.Type.ERROR;
    ctx.timer = 1000; // 1 second
    var count: u32 = 0;
    while(! S.should_quit) {
        some_event = try ctx.wait_event();

        // For clarity in the output.
        if (some_event.t != .TIMER and previous_event == .TIMER ) { print("\n", .{}); }
        previous_event = some_event.t;

        switch (some_event.t) {
            .TIMER => {
                print ("\rTimer! ({})", .{count});
                count += 1;
            },

            .CONNECTION => {
                print ("New connection: {} so far!\n", .{ctx.pollfd.items.len});
            },

            .DISCONNECTION => {
                print ("User {} disconnected, {} remaining.\n"
                    , .{some_event.origin, ctx.pollfd.items.len});
            },

            .EXTERNAL => {
                print ("Message received from a non IPC socket.\n", .{});
                var client = try server.accept(); // net.StreamServer.Connection
                errdefer client.stream.close();
                // Receiving a new client from the EXTERNAL socket.
                // New client = new switch from a distant TCP connection to a
                // local libipc service.

                var buffer: [10000]u8 = undefined;
                var size = try client.stream.read(&buffer);
                var service_to_contact = buffer[0..size];

                if (service_to_contact.len == 0) {
                    print("Error, no service provided, closing the connection.\n", .{});
                    client.stream.close();
                    continue;
                }

                print ("Ask to connect to service [{s}].\n", .{service_to_contact});
                var servicefd = ctx.connect_service (service_to_contact) catch |err| {
                    print("Error while connecting to the service {s}: {}.\n"
                         , .{service_to_contact, err});
                    print ("Closing the connection.\n", .{});
                    client.stream.close();
                    continue;
                };
                errdefer ctx.close_fd (servicefd) catch {};

                print ("Send a message to inform remote TCPd that the connection is established.\n", .{});
                _ = try client.stream.write("ok");

                print ("Add current client as external connection (for now).\n", .{});
                try ctx.add_external (client.stream.handle);

                print ("Message sent, switching.\n", .{});
                try ctx.add_switch(client.stream.handle, servicefd);

                print ("DONE.\n", .{});

                // Some protocols will require to change the default functions
                // to read and to write on the client socket.
                // Function to call: ctx.set_switch_callbacks(clientfd, infn, outfn);
            },

            .SWITCH_RX => {
                print ("Message has been received (SWITCH fd {}).\n", .{some_event.origin});
                // if (some_event.m) |m| {
                //     var hexbuf: [4000]u8 = undefined;
                //     var hexfbs = std.io.fixedBufferStream(&hexbuf);
                //     var hexwriter = hexfbs.writer();
                //     try hexdump.hexdump(hexwriter, "Received", m.payload);
                //     print("{s}\n", .{hexfbs.getWritten()});
                // }
                // else {
                //     print ("Message received without actually a message?! {}", .{some_event});
                // }
            },

            .SWITCH_TX => {
                print ("Message has been sent (SWITCH fd {}).\n", .{some_event.origin});
            },

            .MESSAGE_RX => {
                print ("Client asking for a service through TCPd.\n", .{});
                errdefer ctx.close (some_event.index) catch {};
                if (some_event.m) |m| {
                    defer m.deinit(); // Do not forget to free the message payload.

                    print ("URI to contact {s}\n", .{m.payload});
                    var uri = URI.read(m.payload);
                    print ("proto [{s}] address [{s}] path [{s}]\n"
                        , .{uri.protocol, uri.address, uri.path});

                    var iterator = std.mem.split(u8, uri.address, ":");
                    var real_tcp_address = iterator.first();
                    var real_tcp_port = try std.fmt.parseUnsigned(u16, iterator.rest(), 10);

                    var socket_addr = try net.Address.parseIp(real_tcp_address, real_tcp_port);
                    var stream = try net.tcpConnectToAddress(socket_addr);
                    errdefer stream.close();

                    print ("Writing URI PATH: {s}\n", .{uri.path});
                    _ = try stream.write(uri.path);

                    print ("Writing URI PATH - written, waiting for the final 'ok'.\n", .{});
                    var buffer: [10000]u8 = undefined;
                    var size = try stream.read(&buffer);
                    if (! std.mem.eql(u8, buffer[0..size], "ok")) {
                        print ("didn't receive 'ok', let's kill the connection\n", .{});
                        stream.close();
                        try ctx.close(some_event.index);
                        continue;
                    }
                    print ("Final 'ok' received, sending 'ok' to IPCd.\n", .{});

                    // Connection is established, inform IPCd.
                    var response = try Message.init(some_event.origin, allocator, "ok");
                    defer response.deinit();
                    try ctx.write(response);

                    print ("Add current client as external connection (for now).\n", .{});
                    try ctx.add_external (stream.handle);

                    print ("Finally, add switching\n", .{});
                    try ctx.add_switch(some_event.origin, stream.handle);
                    // Could invoke ctx.set_switch_callbacks but TCP sockets are
                    // working pretty well with default functions.
                }
                else {
                    // TCPd was contacted without providing a message, nothing to do.
                    var response = try Message.init(some_event.origin, allocator, "no");
                    defer response.deinit();
                    try ctx.write(response);
                    try ctx.close(some_event.index);
                }
            },

            .MESSAGE_TX => {
                print ("Message sent.\n", .{});
            },

            .ERROR => {
                print ("A problem occured, event: {}, let's suicide.\n", .{some_event});
                break;
            },
        }
    }

    print ("Goodbye\n", .{});
}

pub fn main() !u8 {
    try create_service();
    return 0;
}
