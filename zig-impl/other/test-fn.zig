const std = @import("std");
const hexdump = @import("./hexdump.zig");
const net = std.net;
const fmt = std.fmt;
const os = std.os;

const print = std.debug.print;

fn say_hello(some_number: i32) void {
    print ("hello number {}\n", .{some_number});
}

fn say_coucou(some_number: i32) void {
    print ("coucou number {}\n", .{some_number});
}

pub fn main() !u8 {
    var fp: *const fn (i32) void = say_hello;
    // print ("currently fp {any}\n", .{fp});
    fp(10);
    fp = say_coucou;
    // print ("currently fp {p}\n", .{fp.?});
    fp(20);
    return 0;
}
