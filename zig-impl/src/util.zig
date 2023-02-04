const std = @import("std");
const hexdump = @import("./hexdump.zig");
const testing = std.testing;

const log = std.log.scoped(.libipc_util);
const Message = @import("./message.zig").Message;

/// A VERY LIGHT and INCOMPLETE way of decoding URI.
/// DO NOT USE IT UNLESS YOU KNOW WHAT TO EXPECT.
pub const URI = struct {
    protocol: []const u8,
    address:  []const u8,
    path:     []const u8,

    const Self = @This();

    pub fn read(uri_to_decode: []const u8) Self {

        var protocolit = std.mem.split(u8, uri_to_decode, "://");
        var protocol = protocolit.first();

        var addressit = std.mem.split(u8, protocolit.next().?, "/");
        var address = addressit.first();

        var path = addressit.rest();

        return Self { .protocol = protocol
            , .address = address
            , .path = path };
    }
};

test "URI simple decoding" {
    var uri = URI.read("tcp://127.0.0.1:8000/some-path");
    try testing.expectEqualSlices(u8, uri.protocol, "tcp");
    try testing.expectEqualSlices(u8, uri.address, "127.0.0.1:8000");
    try testing.expectEqualSlices(u8, uri.path, "some-path");
}

pub fn print_buffer (header: []const u8, buffer: []const u8) void {
    var hexbuf: [4000]u8 = undefined;
    var hexfbs = std.io.fixedBufferStream(&hexbuf);
    var hexwriter = hexfbs.writer();
    hexdump.hexdump(hexwriter, header, buffer) catch unreachable;
    log.debug("{s}", .{hexfbs.getWritten()});
}

pub fn print_message (header: []const u8, m: Message) void {
   print_buffer (header, m.payload);
}

pub fn print_eq(expected: anytype, obj: anytype) !void {
    var buffer: [4096]u8 = undefined;
    var fbs = std.io.fixedBufferStream(&buffer);
    var writer = fbs.writer();

    try writer.print("{}", .{obj});

    // typing workaround
    var secbuffer: [4096]u8 = undefined;
    var secfbs = std.io.fixedBufferStream(&secbuffer);
    var secwriter = secfbs.writer();

    try secwriter.print("{s}", .{expected});

    try testing.expectEqualSlices(u8, secfbs.getWritten(), fbs.getWritten());
}
