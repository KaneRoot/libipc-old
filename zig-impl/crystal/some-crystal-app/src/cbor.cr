require "cbor"

# IPC::CBOR is the root class for all exchanged messages (using CBOR).
# IPC::CBOR inherited classes have a common 'type' class attribute,
# which enables to find the right IPC::CBOR+ class given a TypedMessage's type.

# All transfered messages are typed messages.
# TypedMessage = u8 type (= IPC::CBOR+ class type) + CBOR content.
# 'CBOR content' being a serialized IPC::CBOR+ class.

# Conventionally, IPC::CBOR+ classes have a 'handle' method to process incoming messages.
class IPC::CBOR
	include ::CBOR::Serializable

	class_property type = -1
	property     id   : ::CBOR::Any?

	def type
		@@type
	end

	macro message(id, type, &block)
		class {{id}} < ::IPC::CBOR
			include ::CBOR::Serializable

			@@type = {{type}}

			{{yield}}
		end
	end
end

class IPC
	# Schedule messages contained into IPC::CBOR+.
	def schedule(fd : Int32, message : IPC::CBOR)
		typed_msg = IPCMessage::TypedMessage.new message.type.to_u8, message.to_cbor
		schedule fd, typed_msg
	end
	def write(fd : Int32, message : IPC::CBOR)
		typed_msg = IPCMessage::TypedMessage.new message.type.to_u8, message.to_cbor
		write fd, typed_msg
	end
end

# CAUTION: Only use this method on an Array(IPC::CBOR.class)
class Array(T)
	def parse_ipc_cbor(message : IPC::Message) : IPC::CBOR?
		message_type = find &.type.==(message.utype)

		if message_type.nil?
			raise "invalid message type (#{message.utype})"
		end

		message_type.from_cbor message.payload
	end
end

# CAUTION: only use this method on an Array(IPC::CBOR.class).
class Array(T)
	def parse_ipc_cbor(message : IPCMessage::TypedMessage) : IPC::CBOR?
		message_type = find &.type.==(message.type)

		payload = message.payload

		if message_type.nil?
			raise "invalid message type (#{message.type})"
		end

		message_type.from_cbor payload
	end
end
