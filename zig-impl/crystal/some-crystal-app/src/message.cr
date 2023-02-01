# TODO:
module IPCMessage
	class UntypedMessage
		@fd      : Int32
		@payload : Bytes
		def initialize(@fd, string : String)
			@payload = Bytes.new string.to_unsafe, string.size
		end
		def initialize(@fd, @payload)
		end
	end

	class TypedMessage < UntypedMessage
		@type    : UInt8? = nil
		def initialize(fd, @type, string : String)
			super fd, string
		end
		def initialize(fd, @type, payload)
			super fd, payload
		end
		def initialize(fd, payload)
			super fd, payload
		end
	end
end
