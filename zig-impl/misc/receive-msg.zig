const std = @import("std");
const testing = std.testing;
const net = std.net;
const fmt = std.fmt;
const mem = std.mem;
const print = std.debug.print;

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
    print("Waiting for a connection...\n", .{});
    var connection = try waiting_for_connection(&stream, path);
    defer remove_unix_socket(path);
    print("Someone is connected! Receiving a message...\n", .{});
    try receive_msg(connection.stream);
    print("Disconnection...\n", .{});
    disconnect(&stream);
    print("Disconnected!\n", .{});
    return 0;
}
