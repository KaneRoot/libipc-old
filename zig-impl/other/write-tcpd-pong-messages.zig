const std = @import("std");
const ipc = @import("./ipc.zig");
const hexdump = @import("./hexdump.zig");
const print = std.debug.print;


pub fn main() !void {
    var allocator = std.heap.c_allocator;

    var buffer = [_]u8{0} ** 10000;
    var fbs = std.io.fixedBufferStream(&buffer);
    var writer = fbs.writer();

    var m = try ipc.Message.init (9, allocator, "hello this is me!");
    defer m.deinit();

    _ = try m.write(writer);

    var msg_bytes = fbs.getWritten();

    // var hexbuf: [4000]u8 = undefined;
    // var hexfbs = std.io.fixedBufferStream(&hexbuf);
    // var hexwriter = hexfbs.writer();
    // try hexdump.hexdump(hexwriter, "What should be written in output", msg_bytes);
    // print("{s}\n", .{hexfbs.getWritten()});

    var out = std.io.getStdOut();
    _ = try out.write("pong");
    std.time.sleep(1_000_000_000); // wait for 1 seconds
    _ = try out.write(msg_bytes);
}
