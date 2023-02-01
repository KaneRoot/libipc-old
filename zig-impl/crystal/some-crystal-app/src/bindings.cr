@[Link("ipc")]
lib LibIPC
	enum EventType
		Error         # Self explanatory.
		Connection    # New user.
		Disconnection # User disconnected.
		MessageRx     # Message received.
		MessageTx     # Message sent.
		Timer         # Timeout in the poll(2) function.
		External      # Message received from a non IPC socket.
		SwitchRx      # Switch subsystem: message received.
		SwitchTx      # Switch subsystem: message send.
	end

	fun init   = ipc_context_init   (Void**) : LibC::Int
	fun deinit = ipc_context_deinit (Void**) : Void

	fun service_init    = ipc_service_init   (Void*, LibC::Int*, LibC::Char*, LibC::UInt16T) : LibC::Int
	fun connect_service = ipc_connect_service(Void*, LibC::Int*, LibC::Char*, LibC::UInt16T) : LibC::Int

	# Context EventType index fd buffer buflen
	fun wait = ipc_wait_event(Void*, UInt8*, LibC::UInt64T*, LibC::Int*, UInt8*, LibC::UInt64T*) : LibC::Int

	# Sending a message NOW.
	# WARNING: doesn't wait the fd to become available.
	fun write = ipc_write(Void*, LibC::Int, UInt8*, LibC::UInt64T) : LibC::Int
	# Sending a message (will wait the fd to become available for IO operations).
	fun schedule = ipc_schedule(Void*, LibC::Int, UInt8*, LibC::UInt64T) : LibC::Int

	fun read = ipc_read_fd (Void*, LibC::Int, UInt8*, LibC::UInt64T*);

	fun timer = ipc_context_timer (Void*, LibC::Int)

	# Closing connections.
	fun close     = ipc_close(Void*, LibC::UInt64T) : LibC::Int
	fun close_fd  = ipc_close_fd(Void*, LibC::Int) : LibC::Int
	fun close_all = ipc_close_all(Void*) : LibC::Int
end
