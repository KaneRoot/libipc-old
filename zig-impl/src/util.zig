const std = @import("std");
// const hexdump = @import("./hexdump.zig");
const testing = std.testing;


pub fn print_eq(expected: anytype, obj: anytype) !void {
    var buffer: [4096]u8 = undefined;
    var fbs = std.io.fixedBufferStream(&buffer);
    var writer = fbs.writer();

    try writer.print("{}", .{obj});
    // print("print_eq, expected: {s}\n", .{expected});
    // print("print_eq: {s}\n", .{fbs.getWritten()});

    // typing workaround
    var secbuffer: [4096]u8 = undefined;
    var secfbs = std.io.fixedBufferStream(&secbuffer);
    var secwriter = secfbs.writer();

    try secwriter.print("{s}", .{expected});

    try testing.expectEqualSlices(u8, secfbs.getWritten(), fbs.getWritten());
}
