const std = @import("std");
const net = std.net;
const fmt = std.fmt;
const os = std.os;

const print = std.debug.print;
const testing = std.testing;
const print_eq = @import("./util.zig").print_eq;
const URI = @import("./util.zig").URI;

fn init_tcp_server(allocator: std.mem.Allocator) !net.StreamServer {
    var buffer: [1000]u8 = undefined;
    var fbs = std.io.fixedBufferStream(&buffer);
    var writer = fbs.writer();

    var address = std.process.getEnvVarOwned(allocator, "ADDRESS") catch |err| switch(err) {
        error.EnvironmentVariableNotFound => blk: {
            print("no ADDRESS envvar: TCPd will listen on 127.0.0.1:9000\n", .{});
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

    var server = net.StreamServer.init(.{.reuse_address = true});
    var socket_addr = try net.Address.parseIp(real_tcp_address, real_tcp_port);
    try server.listen(socket_addr);

    return server;
}

fn accept_read(server: *net.StreamServer) !void {
    var client = try server.accept(); // net.StreamServer.Connection
    var buffer: [1000]u8 = undefined;
    var size = try client.stream.read(&buffer);
    print ("received: {s}\n", .{buffer[0..size]});
    client.stream.close();
}

pub fn main() !u8 {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();
    var streamserver = try init_tcp_server(allocator);
    try accept_read(&streamserver);
    return 0;
}
