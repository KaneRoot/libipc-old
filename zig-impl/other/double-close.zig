const std = @import("std");
const print = std.debug.print;

pub fn main() !u8 {
    print("First close!\n", .{});
    std.os.close(1);
    print("SECOND close!\n", .{});
    std.os.close(1);
    print("Will it be printed?\n", .{});
    return 0;
}
