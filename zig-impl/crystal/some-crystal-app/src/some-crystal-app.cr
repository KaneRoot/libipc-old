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

# TODO:
module IPCMessage
	class TypedMessage
		@fd      : Int32
		@type    : UInt8
		@payload : Bytes
		def initialize(@fd, @type, string : String)
			@payload = Bytes.new string
		end
		def initialize(@fd, @type, @payload)
		end
	end
end

class IPC
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

	# Closes all connections then remove the structure from memory.
	def deinit
		LibIPC.deinit(pointerof(@context))
	end

	def service_init(name : String) : Int
		fd = uninitialized Int32
		puts "service name: #{name}"
		puts "service name len: #{name.size}"
		if LibIPC.service_init(@context, pointerof(fd), name, name.size) != 0
			raise "oh noes, 'service_init' iz brkn"
		end
		fd
	end

	def connect(name : String) : Int
		fd = uninitialized Int32
		if LibIPC.connect_service(@context, pointerof(fd), name, name.size) != 0
			raise "oh noes, 'connect_service' iz brkn"
		end
		fd
	end

	def write(fd : Int, message : IPCMessage::TypedMessage)
		self.write(fd, message.payload.to_unsafe, message.payload.size)
	end

	def write(fd : Int, string : String)
		self.write(fd, string.to_unsafe, string.size.to_u64)
	end

	def write(fd : Int, buffer : UInt8*, buflen : UInt64)
		if LibIPC.write(@context, fd, buffer, buflen) != 0
			raise "oh noes, 'write' iz brkn"
		end
	end

	def schedule(fd : Int32, string : String)
		self.schedule(fd, string.to_unsafe, string.size.to_u64)
	end

	def schedule(fd : Int32, buffer : Array(UInt8), buflen : Int32)
		self.schedule(fd, buffer.to_unsafe, buflen.to_u64)
	end

	def schedule(fd : Int32, buffer : UInt8*, buflen : UInt64)
		if LibIPC.schedule(@context, fd, buffer, buflen) != 0
			raise "oh noes, 'schedule' iz brkn"
		end
	end

	def close(index : LibC::UInt64T)
		if LibIPC.close(@context, index) != 0
			raise "Oh noes, 'close index' iz brkn"
		end
	end

	def close(fd : LibC::Int)
		if LibIPC.close_fd(@context, fd) != 0
			raise "Oh noes, 'close fd' iz brkn"
		end
	end

	def close_all()
		if LibIPC.close_all(@context) != 0
			raise "Oh noes, 'close all' iz brkn"
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

	def loop(&block : Proc(IPC::Event, Nil))
		::loop do
			yield wait
		end
	end
end
