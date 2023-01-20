const std = @import("std");
const net = std.net;
const fmt = std.fmt;
const os = std.os;

const print = std.debug.print;
const testing = std.testing;
const print_eq = @import("./util.zig").print_eq;
const URI = @import("./util.zig").URI;

fn connect_tcp(allocator: std.mem.Allocator) !net.Stream {
    var buffer: [1000]u8 = undefined;
    var fbs = std.io.fixedBufferStream(&buffer);
    var writer = fbs.writer();

    var address = std.process.getEnvVarOwned(allocator, "ADDRESS") catch |err| switch(err) {
        error.EnvironmentVariableNotFound => blk: {
            print("no ADDRESS envvar: connecting on 127.0.0.1:9000\n", .{});
            break :blk try allocator.dupe(u8, "127.0.0.1:9000");
        },
        else => { return err; },
    };
    defer allocator.free(address);

    try writer.print("{s}", .{address});
    var tcp_address = fbs.getWritten();

    var iterator = std.mem.split(u8, tcp_address, ":");
    var real_tcp_address = iterator.first();
    var real_tcp_port = try std.fmt.parseUnsigned(u16, iterator.rest(), 10);

    print ("TCP address [{s}] port [{}]\n", .{real_tcp_address, real_tcp_port});

    var socket_addr = try net.Address.parseIp(real_tcp_address, real_tcp_port);
    var stream = try net.tcpConnectToAddress(socket_addr);

    return stream;
}

pub fn main() !u8 {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();
    var stream = try connect_tcp(allocator);
    _ = try stream.write("coucou");
    stream.close();
    return 0;
}
