const std = @import("std");
const testing = std.testing;

const SOMESTRUCT = packed struct {
    somevalue: i32,
    const Self = @This();

    fn update(self: *Self) callconv(.C) i32 {
        self.somevalue += 1;
        return self.somevalue;
    }
};

export fn some_struct_bidouillage_init() callconv(.C) SOMESTRUCT {
    return SOMESTRUCT {.somevalue = 2};
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
