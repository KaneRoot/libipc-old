
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

	struct Clients
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

	fun ipc_server_select(Clients*, Service*, Clients*, LibC::Int*) : LibC::Int

	fun ipc_application_connection(LibC::Int, LibC::Char**, LibC::Char**, Service*, LibC::Char*, LibC::Char*, LibC::UInt)
	fun ipc_application_close(Service*) : LibC::Int
	fun ipc_application_read(Service*, Message*) : LibC::Int
	fun ipc_application_write(Service*, Message*) : LibC::Int

	fun ipc_client_add(Clients*, Client*) : LibC::Int
	fun ipc_client_del(Clients*, Client*) : LibC::Int

	fun ipc_server_client_copy(Client*) : Client*
	fun ipc_server_client_eq(Client*, Client*) : LibC::Int

	fun ipc_server_client_gen(Client*, LibC::UInt, LibC::UInt)

	fun ipc_clients_free(Clients*)
end

class IPC::Exception < ::Exception

end

class IPC::Service
	@closed = false
	@clients = LibIPC::Clients.new
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

	def loop(&block)
		active_clients = LibIPC::Clients.new
		anyone_joined = 0_i32
		message = LibIPC::Message.new

		::loop do
			if LibIPC.ipc_server_select(pointerof(@clients), pointerof(@service), pointerof(active_clients), pointerof(anyone_joined)) < 0
				raise Exception.new "ipc_server_select < 0"
			end

			if anyone_joined > 0
				client = LibIPC::Client.new
				if LibIPC.ipc_server_accept(pointerof(@service), pointerof(client)) < 0
					raise Exception.new "ipc_server_accept < 0"
				end

				if LibIPC.ipc_client_add(pointerof(@clients), pointerof(client)) < 0
					raise Exception.new "ipc_client_add < 0"
				end

				yield ::IPC::Event::Connection.new ::IPC::Client.new client
			end

			i = 0
			while i < active_clients.size
				client_pointer = active_clients.clients[i]

				return_value = LibIPC.ipc_server_read(client_pointer, pointerof(message))

				if return_value < 0
					raise Exception.new "ipc_server_read < 0"
				elsif return_value == 1
					LibIPC.ipc_client_del pointerof(@clients), client_pointer

					# FIXME: Should probably not be a new Client. Having unique
					#        variables helps in using Clients as keys.
					yield Event::Disconnection.new Client.new client_pointer.value
				else
					yield Event::Message.new(IPC::Message.new(message.type, message.length, message.payload), Client.new client_pointer.value)
				end

				i += 1
			end

			LibIPC.ipc_clients_free(pointerof(active_clients))
		end

		close
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
	def initialize(@client)
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
			raise Exception.new "ipc_server_write < 0"
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

class IPC::Event
	class Connection
		getter client : ::IPC::Client
		def initialize(@client)
		end
	end

	class Disconnection
		getter client : ::IPC::Client
		def initialize(@client)
		end
	end

	class Message
		getter message : ::IPC::Message
		getter client : ::IPC::Client
		def initialize(@message, @client)
		end
	end
end

IPC::Service.new("pongd").loop do |event|
	if event.is_a? IPC::Event::Connection
		puts "Connection: #{event.client}"
	elsif event.is_a? IPC::Event::Disconnection
		puts "Disconnection: #{event.client}"
	elsif event.is_a? IPC::Event::Message
		next if event.message.type == 0

		puts "Message: #{event.message.payload}"
		event.client.send event.message.type, event.message.payload
	end
end

