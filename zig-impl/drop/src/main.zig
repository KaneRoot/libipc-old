const std = @import("std");
const testing = std.testing;

const print = std.debug.print;

const SOMESTRUCT = packed struct {
    somevalue: i32,
    const Self = @This();

    fn update(self: *Self) callconv(.C) i32 {
        self.somevalue += 1;
        return self.somevalue;
    }
};

export fn some_struct_bidouillage_init(ptr: *anyopaque) callconv(.C) void {
    var pointer = @ptrCast(*SOMESTRUCT, @alignCast(@alignOf(SOMESTRUCT),ptr));
    var somestruct = std.heap.c_allocator.create(SOMESTRUCT) catch null;

    print ("hello we just did something\n", .{});
    if (somestruct) |s| {
        s.somevalue = 2;
        print ("just changed a value\n", .{});
        pointer.* = s.*;
    }
    print ("hello again\n", .{});
//    else {
//        pointer.* = null;
//    }
}

export fn some_struct_bidouillage_update(s: *SOMESTRUCT) callconv(.C) i32 {
    return s.update();
}

export fn someipc(a: i32, b: i32) callconv(.C) i32 {
    return a + b;
}

test "basic add functionality" {
    try testing.expect(someipc(3, 7) == 10);
}
