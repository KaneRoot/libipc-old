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
    print("Sent {} bytes! Waiting a message...\n", .{bytecount});
    try receive_msg(stream);
    print("Received a message! Disconnection...\n", .{});
    disconnect(stream);
    print("Disconnected!\n", .{});
    return 0;
}
