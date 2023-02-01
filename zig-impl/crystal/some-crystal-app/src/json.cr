require "json"

class IPC::JSON
	include ::JSON::Serializable

	class_property type = -1
	property     id   : ::JSON::Any?

	def type
		@@type
	end

	macro message(id, type, &block)
		class {{id}} < ::IPC::JSON
			include ::JSON::Serializable

			@@type = {{type}}

			{{yield}}
		end
	end
end

class IPC
	def schedule(fd : Int32, message : IPC::JSON)
		schedule fd, message.type.to_u8, message.to_json
	end
end

# CAUTION: Only use this method on an Array(IPC::JSON.class)
class Array(T)
	def parse_ipc_json(message : IPCMessage::TypedMessage) : IPC::JSON?
		message_type = find &.type.==(message.type)

		payload = String.new message.payload

		if message_type.nil?
			raise "invalid message type (#{message.type})"
		end

		message_type.from_json payload
	end
end
