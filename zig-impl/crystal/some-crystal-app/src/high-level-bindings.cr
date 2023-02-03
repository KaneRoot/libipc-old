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

	def connect(name : String) : Int32
		fd = uninitialized Int32
		if LibIPC.connect_service(@context, pointerof(fd), name, name.size) != 0
			raise "oh noes, 'connect_service' iz brkn"
		end
		fd
	end

	def timer(value : LibC::Int)
		LibIPC.timer(@context, value)
	end

	def write(fd : Int, string : String)
		self.write(fd, string.to_unsafe, string.size.to_u64)
	end

	def write(fd : Int, buffer : UInt8*, buflen : UInt64)
		if LibIPC.write(@context, fd, buffer, buflen) != 0
			raise "oh noes, 'write' iz brkn"
		end
	end

	def write(fd : Int32, buffer : Bytes)
		self.write(fd, buffer.to_unsafe, buffer.size.to_u64)
	end

	def read(fd : Int32) : Slice(UInt8)
		buffer : Bytes = Bytes.new 2000000
		size = buffer.size.to_u64
		LibIPC.read(@context, fd, buffer.to_unsafe, pointerof(size))
		buffer[0..size]
	end

	def schedule(fd : Int32, string : String)
		self.schedule(fd, string.to_unsafe, string.size.to_u64)
	end

	def schedule(fd : Int32, buffer : Array(UInt8), buflen : Int32)
		self.schedule(fd, buffer.to_unsafe, buflen.to_u64)
	end

	def schedule(fd : Int32, buffer : Bytes)
		self.schedule(fd, buffer.to_unsafe, buffer.size.to_u64)
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

	def close
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
