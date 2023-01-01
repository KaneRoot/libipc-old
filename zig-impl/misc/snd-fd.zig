const std = @import("std");
const testing = std.testing;
const net = std.net;
const fmt = std.fmt;
const mem = std.mem;
const os = std.os;
const print = std.debug.print;

const Cmsghdr = @import("./cmsghdr.zig").Cmsghdr;

fn disconnect(stream: net.Stream) void { stream.close(); }

fn connect(path: []const u8) !net.Stream {
    return try net.connectUnixSocket(path);
}

const SCM_RIGHTS: c_int = 1;

fn send_msg(sock: os.socket_t, msg: []const u8, fd: os.fd_t) void {
    var iov = [_]os.iovec_const{
        .{
            .iov_base = msg.ptr,
            .iov_len = msg.len,
        },
    };

    var cmsg = Cmsghdr(os.fd_t).init(.{
        .level = os.SOL.SOCKET,
        .@"type" = SCM_RIGHTS,
        .data = fd,
    });

    const len = os.sendmsg(sock, .{
        .name = undefined,
        .namelen = 0,
        .iov = &iov,
        .iovlen = iov.len,
        .control = &cmsg,
        .controllen = @sizeOf(@TypeOf(cmsg)),
        .flags = 0,
        }, 0) catch |err| {
        print("error sendmsg failed with {s}", .{@errorName(err)});
        return;
    };

    if (len != msg.len) {
        // we don't have much choice but to exit here
        // log.err(@src(), "expected sendmsg to return {} but got {}", .{msg.len, len});
        print("expected sendmsg to return {} but got {}", .{msg.len, len});
        os.exit(0xff);
    }
}

//    const buffer_size = 10000;
//    var buffer: [buffer_size]u8 = undefined;
//    var fba = std.heap.fixedBufferAllocator(&buffer);

fn add_line_in_file() !void {
    var cwd = std.fs.cwd();
    var f = try cwd.createFile("some-file.log", .{.read = true});
    defer f.close();

    var writer = f.writer();
    try writer.print("hello\n", .{});
}
pub fn main() !u8 {
    var path = "/tmp/.TEST_USOCK";

    print("Connection to {s}...\n", .{path});
    var stream = try connect(path);

    print("Connected! Opening a file...\n", .{});
    var file = try std.fs.cwd().createFile("some-file.log", .{.read = true});
    defer file.close();

    print("File opened! Writing some data into it...\n", .{});
    var writer = file.writer();
    try writer.print("hello this is the first process\n", .{});

    print("Data written! Sending its fd...\n", .{});
    send_msg(stream.handle, "hello", file.handle);
    print("Sent fd! Disconnection...\n", .{});
    disconnect(stream);
    print("Disconnected!\n", .{});
    return 0;
}
