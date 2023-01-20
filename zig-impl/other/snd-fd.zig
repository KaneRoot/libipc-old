const std = @import("std");
const net = std.net;

const send_fd = @import("./exchange-fd.zig").send_fd;

const print = std.debug.print;

fn disconnect(stream: net.Stream) void { stream.close(); }

fn connect(path: []const u8) !net.Stream {
    return try net.connectUnixSocket(path);
}

fn add_line_in_file() !void {
    var cwd = std.fs.cwd();
    var f = try cwd.createFile("some-file.log", .{.read = true});
    defer f.close();

    var writer = f.writer();
    try writer.print("hello\n", .{});
}

pub fn main() !u8 {
    var path = "/tmp/TEST_EXCHANGE_FD";

    print("Connection to {s}...\n", .{path});
    var stream = try connect(path);

    print("Connected! Opening a file...\n", .{});
    var file = try std.fs.cwd().createFile("some-file.log", .{.read = true});
    defer file.close();

    print("File opened! Writing some data into it...\n", .{});
    var writer = file.writer();
    try writer.print("hello this is the first process\n", .{});

    print("Data written! Sending its fd...\n", .{});
    send_fd(stream.handle, "hello this is the payload", file.handle);
    print("Sent fd! Disconnection...\n", .{});
    disconnect(stream);
    print("Disconnected!\n", .{});
    return 0;
}
