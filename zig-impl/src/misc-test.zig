const std = @import("std");
const hexdump = @import("./hexdump.zig");
const testing = std.testing;
const net = std.net;
const fmt = std.fmt;

const print = std.debug.print;
const P = std.ArrayList(std.os.pollfd);

fn create_service() !void {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var p = P.init(allocator);
    defer p.deinit();

    try p.append(.{.fd = 8, .events = 0, .revents = 0});

    for(p.items) |i| { print("fd: {}\n", .{i.fd}); }
}

pub fn main() !u8 {
    try create_service();
    return 0;
}
