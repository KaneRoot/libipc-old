const std = @import("std");
const hexdump = @import("./hexdump.zig");
const testing = std.testing;
const net = std.net;
const fmt = std.fmt;

const Timer = std.time.Timer;

const print = std.debug.print;
const P = std.ArrayList(std.os.pollfd);

fn arraylist_test() !void {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var p = P.init(allocator);
    defer p.deinit();

    try p.append(.{.fd = 8, .events = 0, .revents = 0});

    for(p.items) |i| { print("fd: {}\n", .{i.fd}); }
}

fn timer_test() !void {
    var timer = try Timer.start();

    var count: u64 = 0;
    while (count < 100000) {
        count += 1;
        print("\rcount = {}", .{count});
    }
    print("\n", .{});

    var duration = timer.read();
    print("took {} us\n", .{duration / 1000});
}

pub fn main() !u8 {
    // try arraylist_test();
    try timer_test();
    return 0;
}
