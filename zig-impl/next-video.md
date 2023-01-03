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

# Functions done

	receive_fd
	send_fd

# Test stuff

	zig test src/main.zig

# Documentation

	zig build-exe -femit-docs -fno-emit-bin src/main.zig

	ACCESS_LOGS ?= ./access.log
	servedoc:
		darkhttpd docs/ --addr 127.0.0.1 --port 35000 --log $(ACCESS_LOGS)
