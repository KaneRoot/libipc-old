const std = @import("std");

fn print(comptime format: []const u8, args: anytype) void {
    const stdout = std.io.getStdOut().writer();
    nosuspend stdout.print(format, args) catch return;
}

fn print_input() !void {
   const stdin = std.io.getStdIn().reader();
   var buffer: [4096]u8 = undefined;
   while (true) {
       const nbytes = try stdin.read(&buffer);
       print("{s}", .{buffer[0..nbytes]});
       if (nbytes == 0) break;
   }
}

fn fatal(comptime format: []const u8, args: anytype) noreturn {
    std.log.err(format, args);
    std.process.exit(1);
}

const usage = "usage: cat [files]";

pub fn main() anyerror!void {
    const args = try std.process.argsAlloc(std.heap.page_allocator);

    if (args.len <= 1) {
        std.log.info("{s}", .{usage});
        fatal("expected command argument", .{});
    }

    const files = args[1..];
    for (files) |f| {
        if (std.mem.eql(u8, f, "-")) {
            try print_input();
        }
        else {
            try print_file (f);
        }
    }
}

fn print_file(dest: []const u8) !void {
   // open file and defer closing
   var file = try std.fs.cwd().openFile(dest, .{ .mode = .read_only });
   defer file.close();

   // read file content and print everything
   var buffer: [4096]u8 = undefined;
   var nbytes : u64 = 0;
   while (true) {
       nbytes = try file.read(&buffer);
       print("{s}", .{buffer[0..nbytes]});
       if (nbytes == 0) break;
   }
}
