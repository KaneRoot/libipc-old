const std = @import("std");
const net = std.net;
const print = std.debug.print;

const receive_fd = @import("./exchange-fd.zig").receive_fd;

fn disconnect(stream: *net.StreamServer) void { stream.close(); }

fn server_init() net.StreamServer {
    // no reuse_address and default kernel_backlog
    return net.StreamServer.init(.{});
}

fn waiting_for_connection(stream: *net.StreamServer
  , path: []const u8) !net.StreamServer.Connection {
    var address = try net.Address.initUnix(path);
    try stream.listen(address);
    return stream.accept();
}

fn remove_unix_socket(path: []const u8) void {
    std.fs.deleteFileAbsolute(path) catch |err| switch(err) {
        else => { print("error: {}\n", .{err}); }
    };
}

fn add_line_from_fd(fd: i32) !void {
    _ = try std.os.write(fd, "SECOND LINE\n");
    std.os.close(fd);
}

pub fn main() !u8 {
    var path = "/tmp/TEST_EXCHANGE_FD";

    print("Init UNIX server to {s}...\n", .{path});
    var stream = server_init();
    defer stream.deinit();

    print("Waiting for a connection...\n", .{});
    var connection = try waiting_for_connection(&stream, path);
    defer remove_unix_socket(path);

    print("Someone is connected! Receiving a file descriptor...\n", .{});
    var msgbuffer: [1500]u8 = undefined;
    var msgsize: usize = 0;
    var fd = try receive_fd(connection.stream.handle, msgbuffer[0..], &msgsize);
    print("received fd: {}, payload: {s}\n", .{fd, msgbuffer[0..msgsize - 1]});

    print("FD received, writing a line into the file...\n", .{});
    try add_line_from_fd(fd);

    print("Disconnection...\n", .{});
    disconnect(&stream);

    print("Disconnected!\n", .{});
    return 0;
}
