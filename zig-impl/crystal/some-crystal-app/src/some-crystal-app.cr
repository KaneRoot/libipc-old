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
	fun ipc_close(Void*, LibC::UInt64T) : LibC::Int
	fun ipc_close_all(Void*) : LibC::Int
end

def test_without_wait()
	ctx = Pointer(Void).null
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

	LibIPC.deinit (pointerof(ctx))
end

class IPC
	#class Message
	#	@fd : Int32
	#	@payload : Bytes
	#	def initialize(@fd, string : String)
	#		@payload = Bytes.new string
	#	end
	#end

	# Reception buffer with a big capacity.
	# Allocated once.
	@reception_buffer = Array(UInt8).new 2_000_000
	@reception_buffer_len : LibC::UInt64T = 2_000_000

	class Event
		property type : LibIPC::EventType
		property index : LibC::UInt64T
		property fd : Int32
		property message : Array(UInt8)? = nil

		def initialize(t : UInt8, @index, @fd, buffer, buflen)
			@type = LibIPC::EventType.new t
			if buflen > 0
				# Array -> Pointer -> Slice -> Array
				@message = buffer.to_unsafe.to_slice(buflen).to_a
			end
		end
	end

	def initialize()
		@context = Pointer(Void).null
		LibIPC.init(pointerof(@context))
		at_exit { deinit }
	end

	def deinit
		LibIPC.deinit(pointerof(@context))
	end

	def connect(name : String) : Int
		fd = uninitialized Int32
		if LibIPC.connect_service(@context, pointerof(fd), name, name.size) != 0
			raise "oh noes"
		end
		fd
	end

	def write(fd : Int, string : String)
		self.write(fd, string.to_unsafe, string.size.to_u64)
	end

	def write(fd : Int, buffer : UInt8*, buflen : UInt64)
		if LibIPC.write(@context, fd, buffer, buflen) != 0
			raise "oh noes"
		end
	end

	def wait() : IPC::Event
		eventtype : UInt8 = 0
		index : LibC::UInt64T = 0
		fd : Int32 = 0
		buflen = @reception_buffer_len
		ret = LibIPC.wait(@context,
			pointerof(eventtype),
			pointerof(index),
			pointerof(fd),
			@reception_buffer.to_unsafe,
			pointerof(buflen))

		if ret != 0
			raise "Oh noes, 'wait' iz brkn"
		end

		Event.new(eventtype, index, fd, @reception_buffer, buflen)
	end
end

def test_with_wait()
	ctx = Pointer(Void).null
	LibIPC.init (pointerof(ctx))
	fd : Int32 = 0
	LibIPC.connect_service(ctx, pointerof(fd), "pong", 4)
	LibIPC.write(ctx, fd, "Hello", 5)

	buflen : LibC::UInt64T = 10
	buffer = uninitialized UInt8[10]
	eventtype : UInt8 = 0
	index : LibC::UInt64T = 0

	LibIPC.timer(ctx, 2000) # Wait at most 2 seconds.
	LibIPC.wait(ctx, pointerof(eventtype), pointerof(index), pointerof(fd), buffer.to_unsafe, pointerof(buflen))

	#pp! LibIPC::EventType.new(eventtype), fd, index, buflen
	received = String.new(buffer.to_unsafe, buflen)
	pp! received

	LibIPC.deinit (pointerof(ctx))
end

def test_high_level
	ipc = IPC.new
	fd = ipc.connect("pong")
	ipc.write(fd, "hello this is some value")
	event = ipc.wait()

	m = event.message
	if m.nil?
		puts "No message"
	else
		pp! String.new(m.to_unsafe, m.size)
	end
end

# TODO: Write documentation for `Some::Crystal::App`
module Some::Crystal::App
	VERSION = "0.1.0"

	test_without_wait
	test_with_wait
	test_high_level
end
