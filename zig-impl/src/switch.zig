const std = @import("std");
const testing = std.testing;
const fmt = std.fmt;

pub const Switches = std.ArrayList(Switch);

const print_eq = @import("./util.zig").print_eq;

// TODO: default callbacks, actual switching.
pub const Switch = struct {
    origin : usize,
    destination : usize,

    // orig_in:  ?fn (origin: usize, m: Message) CBEvent,
    // orig_out: ?fn (origin: usize, m: Message) CBEvent,
    // dest_in:  ?fn (origin: usize, m: Message) CBEvent,
    // dest_out: ?fn (origin: usize, m: Message) CBEvent,

    const Self = @This();

    pub fn init(origin: usize, destination: usize) Self {
        return Self {
           .origin = origin,
           .destination = destination,
        };
    }

    pub fn format(self: Self, comptime _: []const u8, _: fmt.FormatOptions, out_stream: anytype) !void {
        try fmt.format(out_stream
            , "switch {} <-> {}"
            , .{ self.origin, self.destination} );
    }
};

test "Switch - creation and display" {
    const config = .{.safety = true};
    var gpa = std.heap.GeneralPurposeAllocator(config){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var switchdb = Switches.init(allocator);
    defer switchdb.deinit();

    var first = Switch.init(3,8); // origin destination
    var second = Switch.init(2,4); // origin destination
    try switchdb.append(first);
    try switchdb.append(second);

    try print_eq("switch 3 <-> 8", first);
    try print_eq("switch 2 <-> 4", second);

    try testing.expect(2 == switchdb.items.len);
}
