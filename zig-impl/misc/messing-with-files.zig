const std = @import("std");
const testing = std.testing;
const net = std.net;
const fmt = std.fmt;
const mem = std.mem;
const print = std.debug.print;


fn print_current_dir() !void {
    const buffer_size = 10000;
    var buffer: [buffer_size]u8 = undefined;
    var fba = std.heap.FixedBufferAllocator.init(&buffer);
    var allocator = fba.allocator();

    print("Print current directory\n", .{});
    var current_dir = try std.fs.cwd().openIterableDir(".", .{});
    var walker = try current_dir.walk(allocator);
    while (try walker.next()) |entry| {
        print("content: {s}\n", .{entry.basename});
    }
    walker.deinit();
    print("DONE\n", .{});
}

fn add_line_in_file() !void {
    var cwd = std.fs.cwd();
    var f = try cwd.createFile("some-file.log", .{.read = true});
    // defer f.close(); // closed in add_line_from_fd

    var writer = f.writer();

    try writer.print("hello\n", .{});

    try add_line_from_fd(f.handle);
}

fn add_line_from_fd(fd: i32) !void {
    // var f = std.fs.File {.handle = fd};
    // defer f.close();

    _ = try std.os.write(fd, "hello this is another line\n");
    std.os.close(fd);
}

pub fn main() !u8 {
    // var path = "/tmp/.TEST_USOCK";
    // print("Opening the file '{s}'.\n", .{path});
    //try print_current_dir();
    try add_line_in_file();
    return 0;
}
