const std = @import("std");
const hexdump = @import("./hexdump.zig");
const net = std.net;
const fmt = std.fmt;
const os = std.os;

const print = std.debug.print;
const ipc = @import("./main.zig");
const Message = ipc.Message;

pub fn main() !u8 {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var ctx = try ipc.Context.init(allocator);
    ctx.deinit(); // There. Can't leak. Isn't Zig wonderful?

    print("Goodbye\n", .{});
    return 0;
}
