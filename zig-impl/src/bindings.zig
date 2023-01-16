const std = @import("std");
const print = std.debug.print;
const ipc = @import("./main.zig");
const Context = ipc.Context;
const Message = ipc.Message;

export fn ipc_context_init (ptr: **Context) callconv(.C) i32 {
    ptr.* = std.heap.c_allocator.create(Context) catch return -1;

    ptr.*.* = Context.init(std.heap.c_allocator) catch |err| {
        print ("libipc: error while init context: {}\n", .{err});
        return -1;
    };
    return 0;
}

export fn ipc_server_init(ctx: *Context, servicefd: *i32, service_name: [*]const u8, service_name_len: u16) callconv(.C) i32 {
    var streamserver = ctx.server_init (service_name[0..service_name_len]) catch return -1;
    servicefd.* = streamserver.sockfd.?;
    return 0;
}

export fn ipc_connect_service (ctx: *Context, servicefd: *i32, service_name: [*]const u8, service_name_len: u16) i32 {
    var fd = ctx.connect_ipc (service_name[0..service_name_len]) catch return -1;
    servicefd.* = fd;
    return 0;
}

export fn ipc_context_deinit (ctx: *Context) callconv(.C) void {
    ctx.deinit();
}

export fn ipc_write (ctx: *Context, servicefd: i32, mcontent: [*]const u8, mlen: u32) i32 {

    var buffer: [100000]u8 = undefined;
    var fba = std.heap.FixedBufferAllocator.init(&buffer);

    var message = Message.init(servicefd, fba.allocator(), mcontent[0..mlen]) catch return -1;
    ctx.write(message) catch return -1;
    return 0;
}

// pub fn schedule (ctx: *Context, m: Message) !void
// pub fn read_fd (ctx: *Context, fd: i32) !?Message
// pub fn read (ctx: *Context, index: usize) !?Message
// 
// pub fn wait_event(ctx: *Context) !Event
// 
// pub fn close_fd(ctx: *Context, fd: i32) !void
// pub fn close(ctx: *Context, index: usize) !void


// // TODO (specifically in the bindings, not in the structure).
// //change_timer
// 
// // Later.
// // pub fn add_external (ctx: *Context, newfd: i32) !void
// // pub fn add_switch(ctx: *Context, fd1: i32, fd2: i32) !void
// // pub fn set_switch_callbacks(ctx: *Context, fd: i
// 
// // doubt it could really be useful
// //pub fn close_all(ctx: *Context) !void
