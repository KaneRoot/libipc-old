@[Link("ipc")]
lib LibIPC
#	enum EventType
#		Error         # Self explanatory.
#		Connection    # New user.
#		Disconnection # User disconnected.
#		MessageRx     # Message received.
#		MessageTx     # Message sent.
#		Timer         # Timeout in the poll(2) function.
#		External      # Message received from a non IPC socket.
#		SwitchRx      # Switch subsystem: message received.
#		SwitchTx      # Switch subsystem: message send.
#	end

	fun init   = ipc_context_init   (Pointer(Pointer(Void))) : LibC::Int
	fun deinit = ipc_context_deinit (Pointer(Void)) : Void

#	# Connection functions.
#	# Context is allocated, ipcd is requested and the connection/initialisation is performed.
#	fun service_init    = ipc_service_init   (Pointer(Void), LibC::Int*, LibC::Char*, LibC::UInt16) : LibC::Int
#	fun connect_service = ipc_connect_service(Pointer(Void), LibC::Int*, LibC::Char*, LibC::UInt16) : LibC::Int
#
#	# Closing connections.
#	fun ipc_close(Pointer(Void), index : LibC::UInt64T) : LibC::Int
#	fun ipc_close_all(Pointer(Void)) : LibC::Int
#
#	# Loop function.
#	fun ipc_wait_event(Ctx*, Event*, LibC::Int*) : LibC::Int
#
#	# Adding and removing file discriptors to read.
#	fun ipc_add(Ctx*, Connection*, Pollfd*) : LibC::Int
#	fun ipc_del(Ctx*, LibC::UInt) : LibC::Int
#	fun ipc_add_fd(Ctx*, LibC::Int) : LibC::Int
#	fun ipc_del_fd(Ctx*, LibC::Int) : LibC::Int
#
#	# Sending a message (will wait the fd to become available for IO operations).
#	fun ipc_write(Ctx*, Message*) : LibC::Int
#	fun ipc_schedule(Pointer(Void), ) : LibC::Int
#
#	# Sending a message NOW.
#	# WARNING: unbuffered send do not wait the fd to become available.
#	fun ipc_write_fd(Int32, Message*) : LibC::Int
end

# TODO: Write documentation for `Some::Crystal::App`
module Some::Crystal::App
  VERSION = "0.1.0"

  # TODO: Put your code here
  ctx : Pointer(Void) = Pointer(Void).null
  LibIPC.init (pointerof(ctx))
  LibIPC.deinit (ctx)
end
