const msg_type    = @intToEnum(Message.Type, try reader.readByte());
try writer.writeByte(@enumToInt(self.t));

var hexbuf: [4000]u8 = undefined;
var hexfbs = std.io.fixedBufferStream(&hexbuf);
var hexwriter = hexfbs.writer();
try hexdump.hexdump(hexwriter, "Message.read input buffer", buffer);
print("{s}\n", .{hexfbs.getWritten()});
