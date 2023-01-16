const std = @import("std");
const fmt = std.fmt;
const heap = std.heap;
const testing = std.testing;

const print = std.debug.print;

pub const Context = struct {
    rundir: [] u8,
    const Self = @This();

    pub fn init() !Self {
        var rundir = try std.heap.c_allocator.dupeZ(u8, "/tmp/libipc-run/");
        return Self { .rundir = rundir };
    }

    pub fn deinit(self: *Self) void {
        std.heap.c_allocator.free(self.rundir);
    }
};

export fn ipc_context_init (ptr: **Context) callconv(.C) i32 {
    ptr.* = std.heap.c_allocator.create(Context) catch return 1;

    ptr.*.* = Context.init() catch |err| {
        print ("libipc: error while init context: {}\n", .{err});
        return 1;
    };
    return 0;
}

export fn ipc_context_deinit (ctx: *Context) callconv(.C) void {
    ctx.deinit();
}
