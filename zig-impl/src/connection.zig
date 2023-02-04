const std = @import("std");
const net = std.net;
const fmt = std.fmt;

const print_eq = @import("./util.zig").print_eq;

pub const Connections = std.ArrayList(Connection);

pub const Connection = struct {

    pub const Type = enum {
        IPC,      // Standard connection.
        EXTERNAL, // Non IPC connection (TCP, UDP, etc.).
        SERVER,   // Messages received = new connections.
        SWITCHED, // IO operations should go through registered callbacks.
    };

    t: Connection.Type,
    path: ?[] const u8, // Not always needed.

    const Self = @This();

    pub fn init(t: Connection.Type, path: ?[] const u8) Self {
        return Self {
           .t            = t,
           .path         = path,
        };
    }

    pub fn format(self: Self, comptime _: []const u8, _: fmt.FormatOptions, out_stream: anytype) !void {
        try fmt.format(out_stream, "{}, path {?s}", .{ self.t, self.path});
    }
};

test "Connection - creation and display" {
    // origin destination
    var path = "/some/path";
    var c1 = Connection.init(Connection.Type.EXTERNAL, path);
    var c2 = Connection.init(Connection.Type.IPC     , null);
    try print_eq("connection.Connection.Type.EXTERNAL, path /some/path", c1);
    try print_eq("connection.Connection.Type.IPC, path null", c2);
}
