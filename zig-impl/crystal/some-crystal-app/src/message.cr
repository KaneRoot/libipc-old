# TODO: tests.

# Serialization (and deserialization) doesn't refer to IPC format.
# IPC serialization format: 'length + value'
# IPCMessage serialization: 'value'
# 'Value' is:
# - simply the message payload for UntypedMessage
# - type (u8) + payload for TypedMessage
module IPCMessage
	class UntypedMessage
		property payload : Bytes
		def initialize(string : String)
			@payload = Bytes.new string.to_unsafe, string.size
		end
		def initialize(@payload)
		end

		def self.deserialize(payload : Bytes) : UntypedMessage
			IPCMessage::UntypedMessage.new payload
		end

		def serialize
			@payload
		end
	end

	# WARNING: you can only have up to 256 types.
	class TypedMessage < UntypedMessage
		property type    : UInt8? = nil
		def initialize(@type, string : String)
			super string
		end
		def initialize(@type, payload)
			super payload
		end
		def initialize(payload)
			super payload
		end

		def self.deserialize(bytes : Bytes) : TypedMessage?
			if bytes.size == 0
				nil
			else
				type = bytes[0]
				IPCMessage::TypedMessage.new type, bytes[1..]
			end
		end

		def serialize
			bytes = Bytes.new (1 + @payload.size)
			type = @type
			bytes[0] = type.nil? ? 0.to_u8 : type
			bytes[1..].copy_from @payload
			bytes
		end
	end
end

# Send both typed and untyped messages.
class IPC
	def schedule(fd : Int32, m : (IPCMessage::TypedMessage | IPCMessage::UntypedMessage))
		payload = m.serialize
		schedule fd, payload
	end
	def write(fd : Int32, m : (IPCMessage::TypedMessage | IPCMessage::UntypedMessage))
		payload = m.serialize
		write fd, payload
	end
end
