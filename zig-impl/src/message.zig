const std = @import("std");
const testing = std.testing;
const net = std.net;
const fmt = std.fmt;

const print_eq = @import("./util.zig").print_eq;

pub const Messages = std.ArrayList(Message);

pub const Message = struct {

    fd: i32,              // File descriptor concerned about this message.
    payload: []const u8,

    allocator: std.mem.Allocator,  // Memory allocator.

    const Self = @This();

    pub fn init(fd: i32
               , allocator: std.mem.Allocator
               , payload: []const u8) !Self {
        return Message { .fd = fd
            , .allocator = allocator
            , .payload = try allocator.dupe(u8, payload) };
    }

    pub fn deinit(self: Self) void {
        self.allocator.free(self.payload);
    }

    pub fn read(fd: i32, buffer: []const u8, allocator: std.mem.Allocator) !Self {

        var fbs = std.io.fixedBufferStream(buffer);
        var reader = fbs.reader();

        const msg_len = try reader.readIntBig(u32);
        if (msg_len > buffer.len - 4) {
            return error.wrongMessageLength;
        }
        const msg_payload = buffer[4..4+msg_len];

        return try Message.init(fd, allocator, msg_payload);
    }

    pub fn write(self: Self, writer: anytype) !usize {
        try writer.writeIntBig(u32, @truncate(u32, self.payload.len));
        return 4 + try writer.write(self.payload);
    }

    pub fn format(self: Self, comptime _: []const u8, _: fmt.FormatOptions, out_stream: anytype) !void {
        try fmt.format(out_stream, "fd: {}, payload: [{s}]",
            .{self.fd, self.payload} );
    }
};

test "Message - creation and display" {
    // fd payload
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var m = try Message.init(1, allocator, "hello!!");
    defer m.deinit();

    try print_eq("fd: 1, payload: [hello!!]", m);
}

test "Message - read and write" {
    // fd payload
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    // First, create a message.
    var s = "hello!!";
    var first_message = try Message.init(1, allocator, s);
    defer first_message.deinit();

    // Test its content.
    try testing.expect(first_message.fd == 1);
    try testing.expect(first_message.payload.len == 7);
    try testing.expectEqualSlices(u8, first_message.payload, "hello!!");

    // Write it in a buffer, similar to sending it on the network.
    var buffer: [1000]u8 = undefined;
    var fbs = std.io.fixedBufferStream(&buffer);
    var writer = fbs.writer();

    var count = try first_message.write(writer);

    var second_buffer: [2000]u8 = undefined;
    var fba = std.heap.FixedBufferAllocator.init(&second_buffer);
    var second_allocator = fba.allocator();

    // Read the buffer, similar to receiving a message on the network.
    // (8 == random client's fd number)
    var second_message = try Message.read(8, buffer[0..count], second_allocator);
    // var second_message = try Message.read(fbs.getWritten(), second_allocator);
    defer second_message.deinit();

    // Test its content, should be equal to the first.
    try testing.expect(second_message.payload.len == first_message.payload.len);
    try testing.expectEqualSlices(u8, second_message.payload, first_message.payload);
}
