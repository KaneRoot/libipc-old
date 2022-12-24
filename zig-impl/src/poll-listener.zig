const std = @import("std");
const hexdump = @import("./hexdump.zig");
const testing = std.testing;
const net = std.net;
const os = std.os;
const fmt = std.fmt;

const print = std.debug.print;

// const config = .{.safety = true};
// var gpa = std.heap.GeneralPurposeAllocator(config){};
// defer _ = gpa.deinit();
// const allocator = gpa.allocator();

fn disconnect(stream: *net.StreamServer) void { stream.close(); }

fn server_init() net.StreamServer {
    // no reuse_address and default kernel_backlog
    return net.StreamServer.init(.{});
}

fn remove_unix_socket(path: []const u8) void {
    std.fs.deleteFileAbsolute(path) catch |err| switch(err) {
        else => { print("error: {}\n", .{err}); }
    };
}

fn waiting_for_connection(stream: *net.StreamServer
  , path: []const u8) !net.StreamServer.Connection {
    var address = try net.Address.initUnix(path);
    try stream.listen(address);

    // const linux = os.linux;
    // var tfd = try os.timerfd_create(linux.CLOCK.MONOTONIC, linux.TFD.CLOEXEC);
    // defer os.close(tfd);
    // // Fire event 10_000_000ns = 10s after the os.timerfd_settime call.
    // var sit: linux.itimerspec = .{ .it_interval = .{ .tv_sec = 0, .tv_nsec = 0 }
    //                              , .it_value    = .{ .tv_sec = 0, .tv_nsec = 10 * (1000 * 1000 * 1000) } };
    // print("before os.timerfd_settime\n", .{});
    // try os.timerfd_settime(tfd, 0, &sit, null);

    // var fds: [2]os.pollfd =
    //    .{ .{ .fd = tfd, .events = os.linux.POLL.IN, .revents = 0 }
    //     , .{ .fd = lfd, .events = os.linux.POLL.IN, .revents = 0 }};
    // var count = try os.poll(&fds, -1); // -1 => infinite waiting

    var waiting_duration: i32 = 10 * 1000; // in ms
    print("waiting for 10 seconds, tops\n", .{});
    var ssockfd = stream.sockfd; // actual listener (server)
    var lfd: os.socket_t = undefined;

    if (ssockfd) |sfd| { lfd = sfd; }
    else               { return error.Undefined; }

    print("Let's wait for an event (either stdin or unix socket)\n", .{});
    var count: usize = undefined;
    while(true) {
        var fds: [2]os.pollfd =
           .{.{ .fd = 0,   .events = os.linux.POLL.IN, .revents = 0 }
           , .{ .fd = lfd, .events = os.linux.POLL.IN, .revents = 0 }};
        print("fds: {any}\n", .{fds});
        count = try os.poll(&fds, waiting_duration);
        if (count == 0) { print("no client, still waiting\n", .{}); continue; }
        print("fds NOW: {any}\n", .{fds});
        break;
    }

    return stream.accept();
}

fn receive_msg(stream: net.Stream) !void {
    var buffer: [1000]u8 = undefined;
    var fbs = std.io.fixedBufferStream(&buffer);
    var reader = fbs.reader();

    _ = try stream.read(buffer[0..]);

    const msg_type    = try reader.readByte();
    const msg_len     = try reader.readIntBig(u32);
    const msg_payload = buffer[5..5+msg_len];
    print ("type: {}, len {}, content: {s}\n"
           , .{msg_type, msg_len, msg_payload});
}

pub fn main() !u8 {
    var path = "/tmp/.TEST_USOCK";
    print("Init UNIX server to {s}...\n", .{path});
    var stream = server_init();
    defer stream.deinit();
    defer remove_unix_socket(path);

    // TODO
    print("Waiting for a connection...\n", .{});
    var connection = try waiting_for_connection(&stream, path);
    print("Someone is connected! Receiving a message...\n", .{});
    try receive_msg(connection.stream);

    print("Disconnection...\n", .{});
    disconnect(&stream);
    print("Disconnected!\n", .{});
    return 0;
}


