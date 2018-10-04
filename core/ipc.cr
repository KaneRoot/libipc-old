
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
	fun ipc_application_close(Service*)
	fun ipc_application_read(Service*, Message*)
	fun ipc_application_write(Service*, Message*)
end

class IPC::Service
	# FIXME: getter only as long as proper bindings are unfinished
	getter service = LibIPC::Service.new

	def initialize(name : String)
		if LibIPC.ipc_server_init(ARGV.size, ARGV.map(&.to_unsafe).to_unsafe, LibC.environ, pointerof(@service), name) < 0
			# FIXME: throw exception.
			puts "error: ipc_server_init < 0"
			exit 1
		end
	end

	def finalize
		# FIXME: throw exception
		if LibIPC.ipc_server_close(pointerof(@service)) < 0
			puts "error: ipc_server_close < 0"
			exit 1
		end
	end
end

args = ["lala", "--oh-noes"].map &.bytes.to_unsafe

service = IPC::Service.new "pongd"

client = LibIPC::Client.new


client_array = LibIPC::ClientArray.new

pp! client
struct_service = service.service
if LibIPC.ipc_server_accept(pointerof(struct_service), pointerof(client))
	puts "error: ipc_server_accept < 0"
end
pp! client

p LibIPC.ipc_server_close_client pointerof(client)

