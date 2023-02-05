const std = @import("std");
const log = std.log.scoped(.libipc_bindings);
const ipc = @import("./main.zig");
const Context = ipc.Context;
const Message = ipc.Message;
const CBEventType = ipc.CBEvent.Type;

export fn ipc_context_init (ptr: **Context) callconv(.C) i32 {
    ptr.* = std.heap.c_allocator.create(Context) catch return -1;

    ptr.*.* = Context.init(std.heap.c_allocator) catch |err| {
        log.warn("error while init context: {}\n", .{err});
        return -1;
    };
    return 0;
}

/// Start a libipc service.
export fn ipc_service_init(ctx: *Context, servicefd: *i32, service_name: [*]const u8, service_name_len: u16) callconv(.C) i32 {
    var streamserver = ctx.server_init (service_name[0..service_name_len]) catch return -1;
    servicefd.* = streamserver.sockfd.?;
    return 0;
}

/// Connect to a libipc service, possibly through IPCd.
export fn ipc_connect_service (ctx: *Context, servicefd: *i32, service_name: [*]const u8, service_name_len: u16) callconv(.C) i32 {
    var fd = ctx.connect_ipc (service_name[0..service_name_len]) catch return -1;
    servicefd.* = fd;
    return 0;
}

export fn ipc_context_deinit (ctx: **Context) callconv(.C) void {
    var ptr: *Context = ctx.*;
    ptr.deinit();
    std.heap.c_allocator.destroy(ptr);
}

/// Write a message (no waiting).
export fn ipc_write (ctx: *Context, servicefd: i32, mcontent: [*]const u8, mlen: u32) callconv(.C) i32 {
    // TODO: better default length.
    var buffer = [_]u8{0} ** 100000;
    var fba = std.heap.FixedBufferAllocator.init(&buffer);

    var message = Message.init(servicefd, fba.allocator(), mcontent[0..mlen]) catch return -1;
    ctx.write(message) catch return -1;
    return 0;
}

/// Schedule a message.
/// Use the same allocator as the context.
export fn ipc_schedule (ctx: *Context, servicefd: i32, mcontent: [*]const u8, mlen: u32) callconv(.C) i32 {
    var message = Message.init(servicefd, ctx.allocator, mcontent[0..mlen]) catch return -1;
    ctx.schedule(message) catch return -2;
    return 0;
}

/// Read a message from a file descriptor.
/// Buffer length will be changed to the size of the received message.
export fn ipc_read_fd (ctx: *Context, fd: i32, buffer: [*]u8, buflen: *usize) callconv(.C) i32 {
    var m = ctx.read_fd (fd) catch {return -1;} orelse return -2;
    if (m.payload.len > buflen.*) return -3;
    buflen.* = m.payload.len;

    var fbs = std.io.fixedBufferStream(buffer[0..buflen.*]);
    var writer = fbs.writer();
    _ = writer.write(m.payload) catch return -4;
    m.deinit();

    return 0;
}

/// Read a message.
/// Buffer length will be changed to the size of the received message.
export fn ipc_read (ctx: *Context, index: usize, buffer: [*]u8, buflen: *usize) callconv(.C) i32 {
    var m = ctx.read (index) catch {return -1;} orelse return -2;
    if (m.payload.len > buflen.*) return -3;
    buflen.* = m.payload.len;

    var fbs = std.io.fixedBufferStream(buffer[0..buflen.*]);
    var writer = fbs.writer();
    _ = writer.write(m.payload) catch return -4;
    m.deinit();

    return 0;
}

/// Wait for an event.
/// Buffer length will be changed to the size of the received message.
export fn ipc_wait_event(ctx: *Context, t: *u8, index: *usize, originfd: *i32, buffer: [*]u8, buflen: *usize) callconv(.C) i32 {
    var event = ctx.wait_event() catch return -1;
    t.* = @enumToInt(event.t);
    index.* = event.index;
    originfd.* = event.origin;

    if (event.m) |m| {
        var fbs = std.io.fixedBufferStream(buffer[0..buflen.*]);
        var writer = fbs.writer();
        _ = writer.write(m.payload) catch return -4;
        buflen.* = m.payload.len;
        m.deinit();
    }
    else {
        buflen.* = 0;
    }

    return 0;
}

/// Change the timer (ms).
export fn ipc_context_timer (ctx: *Context, timer: i32) callconv(.C) void {
    ctx.timer = timer;
}

export fn ipc_close_fd (ctx: *Context, fd: i32) callconv(.C) i32 {
    ctx.close_fd (fd) catch return -1;
    return 0;
}

export fn ipc_close (ctx: *Context, index: usize) callconv(.C) i32 {
    ctx.close (index) catch return -1;
    return 0;
}

export fn ipc_close_all (ctx: *Context) callconv(.C) i32 {
    ctx.close_all () catch return -1;
    return 0;
}

/// Add a new file descriptor to listen to.
/// The FD is marked as "external"; it isn't a simple libipc connection.
/// You may want to handle any operation on it by yourself.
export fn ipc_add_external (ctx: *Context, newfd: i32) callconv(.C) i32 {
    ctx.add_external (newfd) catch return -1;
    return 0;
}

export fn ipc_add_switch (ctx: *Context, fd1: i32, fd2: i32) callconv(.C) i32 {
    ctx.add_switch (fd1, fd2) catch return -1;
    return 0;
}

export fn ipc_set_switch_callbacks(ctx: *Context, fd: i32
    , in  : *const fn (origin: i32, mcontent: [*]u8,       mlen: *u32) CBEventType
    , out : *const fn (origin: i32, mcontent: [*]const u8, mlen: u32)  CBEventType) callconv(.C) i32 {
    ctx.set_switch_callbacks (fd, in, out) catch return -1;
    return 0;
}
