require "json"

# IPC::JSON is the mother class for all exchanged messages (using JSON).
# IPC::JSON inherited classes have a common 'type' class attribute,
# which enables to find the right IPC::JSON+ class given a TypedMessage's type.

# All transfered messages are typed messages.
# TypedMessage = u8 type (= IPC::JSON+ class type) + JSON content.
# 'JSON content' being a serialized IPC::JSON+ class.

# Conventionally, IPC::JSON+ classes have a 'handle' method to process incoming messages.
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
	# Schedule messages contained into IPC::JSON+.
	def schedule(fd : Int32, message : IPC::JSON)
		typed_msg = IPCMessage::TypedMessage.new message.type.to_u8, message.to_json
		schedule fd, typed_msg
	end
	def write(fd : Int32, message : IPC::JSON)
		typed_msg = IPCMessage::TypedMessage.new message.type.to_u8, message.to_json
		write fd, typed_msg
	end
end

# CAUTION: only use this method on an Array(IPC::JSON.class).
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
