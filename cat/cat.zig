const std = @import("std");
const stdout = std.io.getStdOut().writer();

pub fn main() anyerror!void {
    const args = try std.process.argsAlloc(std.heap.page_allocator);

    if (args.len <= 1) {
        try print_input();
    }

    for (args[1..]) |f| {
        if (std.mem.eql(u8, f, "-")) { try print_input();  }
        else                         { try print_file (f); }
    }
}

fn print_input() !void {
    try print_all (std.io.getStdIn());
}

fn print_file(dest: []const u8) !void {
    var file = try std.fs.cwd().openFile(dest, .{ .mode = .read_only });
    defer file.close();
    try print_all (file);
}

fn print_all(reader: std.fs.File) !void {
    var buffer: [4096]u8 = undefined;
    while (true) {
        const nbytes = try reader.read(&buffer);
        try stdout.print("{s}", .{buffer[0..nbytes]});
        if (nbytes == 0) break;
    }
}
