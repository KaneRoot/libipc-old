const std = @import("std");
const hexdump = @import("./hexdump.zig");
const net = std.net;
const fmt = std.fmt;

const print = std.debug.print;

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

    // TODO: use these connections
    server: ?net.StreamServer = null,
    client: ?net.StreamServer.Connection = null,

    // more_to_read: bool, // useless for now

    const Self = @This();

    pub fn init(t: Connection.Type, path: ?[] const u8) Self {
        return Self {
           .t            = t,
           .path         = path,
           // .more_to_read = false, // TODO: maybe useless
        };
    }

    pub fn deinit(self: *Self) void {
        if (self.server) |*s| { s.deinit(); }
        // if (self.client) |*c| { c.deinit(); }
    }

    pub fn format(self: Self, comptime _: []const u8, _: fmt.FormatOptions, out_stream: anytype) !void {
        try fmt.format(out_stream, "{}, path {?s}", .{ self.t, self.path});

        if (self.server) |s| {
            try fmt.format(out_stream, "{}" , .{s});
        }
        if (self.client) |c| {
            try fmt.format(out_stream, "{}" , .{c});
        }
    }
};

test "Connection - creation and display" {
    // origin destination
    var path = "/some/path";
    var c1 = Connection.init(Connection.Type.EXTERNAL, path);
    defer c1.deinit();
    var c2 = Connection.init(Connection.Type.IPC     , null);
    defer c2.deinit();
    try print_eq("connection.Connection.Type.EXTERNAL, path /some/path", c1);
    try print_eq("connection.Connection.Type.IPC, path null", c2);
}
