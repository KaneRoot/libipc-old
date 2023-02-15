# functions

	var received_fd = @as(i32, cmsg.dataPtr().*);

	std.mem.copy(u8, buffer, &msg_buffer);

	@ptrCast(*std.x.os.Socket.Message, &m)

	os.exit(0xff);

	var network_envvar = std.process.getEnvVarOwned(fba, "IPC_NETWORK") catch |err| switch(err) {
	    // error{ OutOfMemory, EnvironmentVariableNotFound, InvalidUtf8 } (ErrorSet)
	    .EnvironmentVariableNotFound => { return; }, // no need to contact IPCd
	    else => { return err; },
	};


	const log = std.log.scoped(.libipc_context);
	log.err("context.deinit(): IndexOutOfBounds", .{});
	log.debug("stuff(): IndexOutOfBounds", .{});

# Functions done

	receive_fd
	send_fd

# switch

An example of `catch |err| switch(err)`.

# Test stuff

	zig test src/main.zig

# Documentation

	zig build-exe -femit-docs -fno-emit-bin src/main.zig

	ACCESS_LOGS ?= ./access.log
	servedoc:
		darkhttpd docs/ --addr 127.0.0.1 --port 35000 --log $(ACCESS_LOGS)

### Frustration

Searching for a type, this type depends on a sub-type, which depends on the OS, which ultimately... cannot be documented automatically.

Example:

	std.fs.File.Mode => const Mode: "mode_t" = os.mode_t;
	os.mode_t => const mode_t: "mode_t" = system.mode_t;

### anytype

    // create a server path for the UNIX socket based on the service name
    pub fn server_path(self: *Self, service_name: []const u8, writer: anytype) !void {
        try writer.print("{s}/{s}", .{ self.rundir, service_name });
    }

From
    var buffer: [1000]u8 = undefined;
    var fbs = std.io.fixedBufferStream(&buffer);
    var writer = fbs.writer();
    try ctx.server_path("simple-context-test", writer);
    var path = fbs.getWritten();
To
    var buffer: [1000]u8 = undefined;
    var path = try std.fmt.bufPrint(&buffer, "{s}/{s}", .{ ctx.rundir, "simple-context-test" });


# Errors

Double returning type => no need for specific return structures.

# Timer

	const Timer = std.time.Timer;
	var timer = try Timer.start();
	var duration = timer.read() / 1000000; // ns -> ms

var value = db.get(key) orelse return error.notHere;

# There is still room for improvement.

When the result actually is a single value (anonymous hash).
	src/switch.zig:125:37: error: binary operator `|` has whitespace on one side, but not the other.
	        self.db.fetchSwapRemove(fd) |k,v|{
