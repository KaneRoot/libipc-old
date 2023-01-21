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

	fun init   = ipc_context_init   (Void**) : LibC::Int
	fun deinit = ipc_context_deinit (Void*) : Void

	# Connection functions.
	# Context is allocated, ipcd is requested and the connection/initialisation is performed.
	fun service_init    = ipc_service_init   (Void*, LibC::Int*, LibC::Char*, LibC::UInt16T) : LibC::Int
	fun connect_service = ipc_connect_service(Void*, LibC::Int*, LibC::Char*, LibC::UInt16T) : LibC::Int

	fun wait = ipc_wait_event(Void*, Char*, LibC::UInt64T*, LibC::Int*, Char*, LibC::UInt64T*) : LibC::Int

	# Sending a message NOW.
	# WARNING: unbuffered send do not wait the fd to become available.
	fun write = ipc_write(Void*, LibC::Int, UInt8*, LibC::UInt64T) : LibC::Int
	# Sending a message (will wait the fd to become available for IO operations).
	fun schedule = ipc_schedule(Void*, LibC::Int, UInt8*, LibC::UInt64T) : LibC::Int

	fun read = ipc_read_fd (Void*, LibC::Int, UInt8*, LibC::UInt64T*);

#	# Closing connections.
#	fun ipc_close(Pointer(Void), index : LibC::UInt64T) : LibC::Int
#	fun ipc_close_all(Pointer(Void)) : LibC::Int
end

# TODO: Write documentation for `Some::Crystal::App`
module Some::Crystal::App
  VERSION = "0.1.0"

  # TODO: Put your code here
  ctx : Pointer(Void) = Pointer(Void).null
  LibIPC.init (pointerof(ctx))
  fd : Int32 = 0
  LibIPC.connect_service(ctx, pointerof(fd), "pong", 4)
  pp! fd
  LibIPC.write(ctx, fd, "Hello", 5)

  buflen : LibC::UInt64T = 10
  #buffer = uninitialized UInt8[10]
  buffer = Array(UInt8).new(10, 88)
  #buffer[0] = 'a'
  #buffer[1] = 'b'
  #buffer[2] = 'c'
  #buffer[3] = 'd'
  #buffer[4] = 'e'
  #buffer[5] = 'f'
  #pp! buffer[0]
  #pp! buffer[1]
  #pp! buffer[2]
  #pp! buffer[3]
  #pp! buffer[4]
  #pp! buffer[5]
  LibIPC.read(ctx, fd, buffer.to_unsafe, pointerof(buflen))
  pp! buflen
  pp! buffer.size
  buffer[buflen] = 0
  str = String.new(buffer.to_unsafe)
  pp! str

  #received = String.build do |str|
  #  count = buflen
  #  while count > 0
  #    str << buffer[buflen - count]
  #    count -= 1
  #  end
  #end
  #pp! received

  #received = Slice.new(buffer.to_unsafe, buflen)
  ##puts "Buffer is: [#{received}]"

  LibIPC.deinit (ctx)
end
