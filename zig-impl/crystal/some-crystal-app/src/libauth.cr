require "./message.cr"

# TODO:
class AuthMessage < IPCMessage::TypedMessage
	def to_s
		content = String.new(@payload.to_unsafe, @payload.size)
		"AUTH MSG: type #{@type} fd #{@fd} bytes #{@payload.size} [#{content}]"
	rescue
		"AUTH MSG: type #{@type} fd #{@fd} bytes #{@payload.size} [#{@payload}]"
	end
end

module Request
	class Auth < AuthMessage
		@type = 1
	end
end
module Response
	class Auth < AuthMessage
		@type = 2
	end
end

puts Request::Auth.new(5, "hello I'm karchnu").to_s
puts Response::Auth.new(3, "hello okay you're in").to_s
