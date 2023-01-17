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
    // TODO: better default length.
    var buffer: [100000]u8 = undefined;
    var fba = std.heap.FixedBufferAllocator.init(&buffer);

    var message = Message.init(servicefd, fba.allocator(), mcontent[0..mlen]) catch return -1;
    ctx.write(message) catch return -1;
    return 0;
}

export fn ipc_read_fd (ctx: *Context, fd: i32, buffer: [*]u8, buflen: *usize) i32 {
    var m = ctx.read_fd (fd) catch {return -1;} orelse return -2;
    if (m.payload.len > buflen.*) return -3;
    buflen.* = m.payload.len;

    var fbs = std.io.fixedBufferStream(buffer[0..buflen.*]);
    var writer = fbs.writer();
    _ = writer.write(m.payload) catch return -4;

    return 0;
}

export fn ipc_schedule (ctx: *Context, servicefd: i32, mcontent: [*]const u8, mlen: u32) i32 {
    var message = Message.init(servicefd, ctx.allocator, mcontent[0..mlen]) catch return -1;
    ctx.schedule(message) catch return -2;
    return 0;
}

export fn ipc_wait_event(ctx: *Context, t: *u8, index: *usize, originfd: *i32, buffer: [*]u8, buflen: *usize) i32 {
    var event = ctx.wait_event() catch return -1;
    t.* = @enumToInt(event.t);
    index.* = event.index;
    originfd.* = event.origin;

    if (event.m) |m| {
        var fbs = std.io.fixedBufferStream(buffer[0..buflen.*]);
        var writer = fbs.writer();
        _ = writer.write(m.payload) catch return -4;
        buflen.* = m.payload.len;
    }
    else {
        buflen.* = 0;
    }

    return 0;
}

// pub fn read (ctx: *Context, index: usize) !?Message
// 
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
