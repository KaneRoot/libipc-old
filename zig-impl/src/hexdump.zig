const std = @import("std");
const print = std.debug.print;

pub fn hexdump(stream: anytype, header: [] const u8, buffer: [] const u8) std.os.WriteError!void {
    try stream.writeAll("\n");

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
    try stream.print("\n  {d:0>4}:  ", .{ hexb });

    var i: u32 = 0;
    while (i < buffer.len) : (i += 1) {
        try stream.print("{X:0>2} ", .{ buffer[i] });

        if (buffer[i] >= ' ' and buffer[i] <= '~') {
            ascii[(i % 16)] = buffer[i];
        } else {
            ascii[(i % 16)] = '.';
        }

        if ((i + 1) % 8 == 0 or (i + 1) == buffer.len) {
            try stream.writeAll(" ");

            if ((i + 1) % 16 == 0) {
                hexb += 16;

                if ((i + 1) != buffer.len) {
                    try stream.print("{s}\n  {d:0>4}:  ", .{ ascii[0..ascii.len], hexb });
                } else {
                    try stream.print("{s}\n", .{ ascii[0..ascii.len] });
                }
            } else if ((i + 1) == buffer.len) {
                var x: u32 = (i + 1) % 16;

                while (x < 16) : (x += 1) {
                    try stream.writeAll("   ");
                }

                try stream.print(" {s}\n", .{ ascii[0..((i+1) % 16)] });
            }
        }
    }

    try stream.writeAll("\n");
}

// test "simple hexdump test" {
//     print("\n\nPrint hexdump, NO AUTOMATIC VERIFICATION, READ SOURCE CODE\n\n", .{});
// 
//     var buffer = "hello this is a simple text to print";
//     var hexbuf: [2000]u8 = undefined;
//     var hexfbs = std.io.fixedBufferStream(&hexbuf);
//     var hexwriter = hexfbs.writer();
//     try hexdump(hexwriter, "Hello World", buffer);
//     print("{s}\n", .{hexfbs.getWritten()});
// 
// }
