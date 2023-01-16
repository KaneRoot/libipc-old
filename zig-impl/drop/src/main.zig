const std = @import("std");
const fmt = std.fmt;

const print = std.debug.print;

pub const Context = struct {
    rundir: [] u8,
    allocator: std.mem.Allocator,
    const Self = @This();

    pub fn init(allocator: std.mem.Allocator) !Self {
        var rundir = try allocator.dupeZ(u8, "/tmp/libipc-run/");
        return Self { .rundir = rundir, .allocator = allocator };
    }

    pub fn deinit(self: *Self) void {
        self.allocator.free(self.rundir);
    }
};

export fn ipc_context_init (ptr: **Context) callconv(.C) i32 {
    ptr.* = std.heap.c_allocator.create(Context) catch return 1;

    ptr.*.* = Context.init(std.heap.c_allocator) catch |err| {
        print ("libipc: error while init context: {}\n", .{err});
        return 1;
    };
    return 0;
}

export fn ipc_context_deinit (ctx: *Context) callconv(.C) void {
    ctx.deinit();
}
