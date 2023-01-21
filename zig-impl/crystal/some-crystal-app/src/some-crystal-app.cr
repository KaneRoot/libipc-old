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
	fun deinit = ipc_context_deinit (Void*) : Void

	fun service_init    = ipc_service_init   (Void*, LibC::Int*, LibC::Char*, LibC::UInt16T) : LibC::Int
	fun connect_service = ipc_connect_service(Void*, LibC::Int*, LibC::Char*, LibC::UInt16T) : LibC::Int

    # Context EventType index fd buffer buflen
	fun wait = ipc_wait_event(Void*, UInt8*, LibC::UInt64T*, LibC::Int*, UInt8*, LibC::UInt64T*) : LibC::Int

	# Sending a message NOW.
	# WARNING: unbuffered send do not wait the fd to become available.
	fun write = ipc_write(Void*, LibC::Int, UInt8*, LibC::UInt64T) : LibC::Int
	# Sending a message (will wait the fd to become available for IO operations).
	fun schedule = ipc_schedule(Void*, LibC::Int, UInt8*, LibC::UInt64T) : LibC::Int

	fun read = ipc_read_fd (Void*, LibC::Int, UInt8*, LibC::UInt64T*);

	# Closing connections.
	fun ipc_close(Void*, index : LibC::UInt64T) : LibC::Int
	fun ipc_close_all(Void*) : LibC::Int
end

def test_without_wait()
  ctx : Pointer(Void) = Pointer(Void).null
  LibIPC.init (pointerof(ctx))
  fd : Int32 = 0
  LibIPC.connect_service(ctx, pointerof(fd), "pong", 4)
  pp! fd
  LibIPC.write(ctx, fd, "Hello", 5)

  buflen : LibC::UInt64T = 10
  buffer = uninitialized UInt8[10]
  LibIPC.read(ctx, fd, buffer.to_unsafe, pointerof(buflen))
  received = String.new(buffer.to_unsafe, buflen)
  pp! received

  LibIPC.deinit (ctx)
end

def test_with_wait()
  ctx : Pointer(Void) = Pointer(Void).null
  LibIPC.init (pointerof(ctx))
  fd : Int32 = 0
  LibIPC.connect_service(ctx, pointerof(fd), "pong", 4)
  pp! fd
  LibIPC.write(ctx, fd, "Hello", 5)

  buflen : LibC::UInt64T = 10
  buffer = uninitialized UInt8[10]
  eventtype : UInt8 = 0
  index : LibC::UInt64T = 0

  LibIPC.wait(ctx, pointerof(eventtype), pointerof(index), pointerof(fd), buffer.to_unsafe, pointerof(buflen))

  pp! "After waiting: ", LibIPC::EventType.new(eventtype), fd, index, buflen
  received = String.new(buffer.to_unsafe, buflen)
  pp! received

  LibIPC.deinit (ctx)
end

# TODO: Write documentation for `Some::Crystal::App`
module Some::Crystal::App
  VERSION = "0.1.0"

  test_without_wait

  test_with_wait
end
