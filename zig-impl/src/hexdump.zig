const std = @import("std");

pub fn hexdump(stream: anytype, header: [] const u8, buffer: [] const u8) std.os.WriteError!void {
    // Print a header.
    if (header.len > 0) {
        var hdr: [64] u8 = undefined;
        var offset: usize = (hdr.len / 2) - ((header.len / 2) - 1);

        std.mem.set(u8, hdr[0..hdr.len], ' ');
        std.mem.copy(u8, hdr[offset..hdr.len], header);

        try stream.writeAll(hdr[0..hdr.len]);
        try stream.writeAll("\n");
    }

    var hexb: u32 = 0;
    var ascii: [16] u8 = undefined;
    // First line, first left side (simple number).
    try stream.print("\n  {d:0>4}:  ", .{ hexb });

    // Loop on all values in the buffer (i from 0 to buffer.len).
    var i: u32 = 0;
    while (i < buffer.len) : (i += 1) {
        // Print actual hexadecimal value.
        try stream.print("{X:0>2} ", .{ buffer[i] });

        // What to print (simple ascii text, right side).
        if (buffer[i] >= ' ' and buffer[i] <= '~') {
            ascii[(i % 16)] = buffer[i];
        } else {
            ascii[(i % 16)] = '.';
        }

        // Next input is a multiple of 8 = extra space.
        if ((i + 1) % 8 == 0) {
            try stream.writeAll(" ");
        }

        // No next input: print the right amount of spaces.
        if ((i + 1) == buffer.len) {
            // Each line is 16 bytes to print, each byte takes 3 characters.
            var missing_spaces = 3 * (15 - (i%16));
            // Missing an extra space if the current index % 16 is less than 7.
            if ((i%16) < 7) { missing_spaces += 1; }
            while (missing_spaces > 0) : (missing_spaces -= 1) {
                try stream.writeAll(" ");
            }
        }

        // Every 16 bytes: print ascii text and line return.

        // Case 1: it's been 16 bytes AND it's the last byte to print.
        if ((i + 1) % 16 == 0 and (i + 1) == buffer.len) {
            try stream.print("{s}\n", .{ ascii[0..ascii.len] });
        }
        // Case 2: it's been 16 bytes but it's not the end of the buffer.
        else if ((i + 1) % 16 == 0 and (i + 1) != buffer.len) {
            try stream.print("{s}\n", .{ ascii[0..ascii.len] });
            hexb += 16;
            try stream.print("  {d:0>4}:  ", .{ hexb });
        }
        // Case 3: not the end of the 16 bytes row but it's the end of the buffer.
        else if ((i + 1) % 16 != 0 and (i + 1) == buffer.len) {
            try stream.print(" {s}\n", .{ ascii[0..((i+1) % 16)] });
        }
        // Case 4: not the end of the 16 bytes row and not the end of the buffer.
        //         Do nothing.
    }

    try stream.writeAll("\n");
}

const print = std.debug.print;

test "36-byte hexdump test" {
    print("\nPrint hexdump, NO AUTOMATIC VERIFICATION, READ SOURCE CODE\n", .{});

    var buffer = "hello this is a simple text to print";
    var hexbuf: [2000]u8 = undefined;
    var hexfbs = std.io.fixedBufferStream(&hexbuf);
    var hexwriter = hexfbs.writer();
    try hexdump(hexwriter, "Hello World", buffer);
    print("{s}\n", .{hexfbs.getWritten()});
}

test "32-byte hexdump test" {
    print("\nPrint hexdump, NO AUTOMATIC VERIFICATION, READ SOURCE CODE\n", .{});

    var buffer = "THIS IS THE END, MY ONLY...  END";
    var hexbuf: [2000]u8 = undefined;
    var hexfbs = std.io.fixedBufferStream(&hexbuf);
    var hexwriter = hexfbs.writer();
    try hexdump(hexwriter, "Hello World", buffer);
    print("{s}\n", .{hexfbs.getWritten()});
}

test "26-byte hexdump test" {
    print("\nPrint hexdump, NO AUTOMATIC VERIFICATION, READ SOURCE CODE\n", .{});

    var buffer = "hello this is another text";
    var hexbuf: [2000]u8 = undefined;
    var hexfbs = std.io.fixedBufferStream(&hexbuf);
    var hexwriter = hexfbs.writer();
    try hexdump(hexwriter, "Hello World", buffer);
    print("{s}\n", .{hexfbs.getWritten()});
}

test "1-byte hexdump test" {
    print("\nPrint hexdump, NO AUTOMATIC VERIFICATION, READ SOURCE CODE\n", .{});

    var buffer = "h";
    var hexbuf: [2000]u8 = undefined;
    var hexfbs = std.io.fixedBufferStream(&hexbuf);
    var hexwriter = hexfbs.writer();
    try hexdump(hexwriter, "Hello World", buffer);
    print("{s}\n", .{hexfbs.getWritten()});
}

test "0-byte hexdump test" {
    print("\nPrint hexdump, NO AUTOMATIC VERIFICATION, READ SOURCE CODE\n", .{});

    var buffer = "";
    var hexbuf: [2000]u8 = undefined;
    var hexfbs = std.io.fixedBufferStream(&hexbuf);
    var hexwriter = hexfbs.writer();
    try hexdump(hexwriter, "Hello World", buffer);
    print("{s}\n", .{hexfbs.getWritten()});
}
