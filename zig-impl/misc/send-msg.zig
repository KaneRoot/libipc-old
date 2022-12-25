const std = @import("std");
const testing = std.testing;
const net = std.net;
const fmt = std.fmt;
const mem = std.mem;
const print = std.debug.print;

fn disconnect(stream: net.Stream) void { stream.close(); }

fn connect(path: []const u8) !net.Stream {
    return try net.connectUnixSocket(path);
}

fn send_msg(stream: net.Stream) !usize {
    var buffer: [1000]u8 = undefined;
    var fbs = std.io.fixedBufferStream(&buffer);
    var writer = fbs.writer();

    try writer.writeByte(2); // DATA

    const message = "hello everyone";
    try writer.writeIntBig(u32, message.len);
    _ = try writer.write(message);

    return stream.write (fbs.getWritten());
}

pub fn main() !u8 {
    var path = "/tmp/.TEST_USOCK";
    print("Connection to {s}...\n", .{path});
    var stream = try connect(path);
    print("Connected! Sending a message...\n", .{});
    const bytecount = try send_msg(stream);
    print("Sent {} bytes! Disconnection...\n", .{bytecount});
    disconnect(stream);
    print("Disconnected!\n", .{});
    return 0;
}
