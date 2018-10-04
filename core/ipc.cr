
@[Link("ipc")]
lib LibIPC
	struct Service
		version :  LibC::UInt
		index :    LibC::UInt
		spath :    LibC::Char[4096] # [PATH_MAX]
		fd :       LibC::Int
	end

	struct Client
		version :  LibC::UInt
		index :    LibC::UInt
		fd :       LibC::Int
	end

	struct Message
		type :     LibC::Char
		length :   LibC::UShort
		payload :  LibC::Char*
	end

	struct ClientArray
		clients :  Client**
		size :     LibC::Int
	end

	# FIXME: IPC.initialize:
	#  - throw exceptions on error.
	#  - Make most arguments optional.
	fun ipc_server_init(argc : LibC::Int, argv : LibC::Char**, env : LibC::Char**, service : Service*, sname : LibC::Char*) : LibC::Int
	# FIXME: IPC.(destroy?)
	fun ipc_server_close(Service*) : LibC::Int
	fun ipc_server_close_client(Client*) : LibC::Int

	fun ipc_server_accept(Service*, Client*) : LibC::Int
	fun ipc_server_read(Client*, Message*) : LibC::Int
	fun ipc_server_write(Client*, Message*) : LibC::Int

	fun ipc_server_select(ClientArray*, Service*, ClientArray*) : LibC::Int

	fun ipc_application_connection(LibC::Int, LibC::Char**, LibC::Char**, Service*, LibC::Char*, LibC::Char*, LibC::UInt)
	fun ipc_application_close(Service*) : LibC::Int
	fun ipc_application_read(Service*, Message*) : LibC::Int
	fun ipc_application_write(Service*, Message*) : LibC::Int

	fun ipc_client_add(ClientArray*, Client*) : LibC::Int
	fun ipc_client_del(ClientArray*, Client*) : LibC::Int

	fun ipc_server_client_copy(Client*) : Client*
	fun ipc_server_client_eq(Client*, Client*) : LibC::Int

	fun ipc_server_client_gen(Client*, LibC::UInt, LibC::UInt)
end

class IPC::Exception < ::Exception

end

class IPC::Service
	@closed = false
	# FIXME: getter only as long as proper bindings are unfinished
	getter service = LibIPC::Service.new

	def initialize(name : String)
		if LibIPC.ipc_server_init(ARGV.size, ARGV.map(&.to_unsafe).to_unsafe, LibC.environ, pointerof(@service), name) < 0
			raise Exception.new "ipc_server_init < 0" # FIXME: Add proper descriptions here.
		end

		# Very important as there are filesystem side-effects.
		at_exit { close }
	end

	def close
		return if @closed

		# FIXME: Probably check it’s not been closed already.
		if LibIPC.ipc_server_close(pointerof(@service)) < 0
			raise Exception.new "ipc_server_close < 0"
		end

		@closed = true
	end
	def finalize
		close
	end

	def accept
		::IPC::Client.new pointerof(@service)
	end
end

class IPC::Message
	enum Type
		CLOSE
		CONNECTION
		SYN
		ACK
		DATA
	end

	getter type : UInt8
	getter payload : String

	def initialize(type, length, payload)
		@type = type
		@payload = String.new payload, length
	end
end

class IPC::Client
	@closed = false
	getter client = LibIPC::Client.new

	def initialize(service : LibIPC::Service*)
		if LibIPC.ipc_server_accept(service, pointerof(@client)) < 0
			raise Exception.new "ipc_server_accept < 0"
		end
	end
	def read
		message = LibIPC::Message.new

		if LibIPC.ipc_server_read(pointerof(@client), pointerof(message)) < 0
			raise Exception.new "ipc_server_read < 0"
		end

		Message.new message.type, message.length, message.payload
	end
	def send(type : UInt8, payload : String)
		message = LibIPC::Message.new type: type, length: payload.size, payload: payload.to_unsafe

		if LibIPC.ipc_server_write(pointerof(@client), pointerof(message)) < 0
			raise Exception.new "ipc_server_send < 0"
		end
	end
	def close
		return if @closed

		LibIPC.ipc_server_close_client pointerof(@client)
	end
	def finalize
		close
	end
end

service = IPC::Service.new "pongd"

client_array = LibIPC::ClientArray.new

while client = service.accept
	message = client.read
	client.send 4, message.payload
	message = client.read
	if message.type == IPC::Message::Type::CLOSE
		client.close
	end
end

