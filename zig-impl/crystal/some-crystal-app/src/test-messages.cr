require "./main.cr"
require "./json.cr"
require "baguette-crystal-base"

# Context class, so the variables are available everywhere.
class Context
	class_property requests  = [] of IPC::JSON.class
	class_property responses = [] of IPC::JSON.class
end

class IPC::JSON
	def handle
		raise "unimplemented"
	end
end

IPC::JSON.message Message, 10 do
	property content     : String?
	property some_number : Int32?
	def initialize(@content = nil, @some_number = nil)
	end

	def handle
		Baguette::Log.info "message received: #{@content}, number: #{@some_number}"
		if number = @some_number
			::MessageReceived.new number - 1
		else
			::MessageReceived.new
		end
	end
end
Context.requests << Message


IPC::JSON.message Error, 0 do
	property reason : String
	def initialize(@reason)
	end
end
Context.responses << Error

IPC::JSON.message MessageReceived, 20 do
	property minus_one : Int32?
	def initialize(@minus_one = nil)
	end

	def handle
		Baguette::Log.info "<< MessageReceived (#{@minus_one})"
	end
end
Context.responses << MessageReceived

pp! Context.requests
pp! Context.responses

request = ::Message.new "hello this is a request", 30
pp! request
pp! request.to_json
response = request.handle
pp! response
response.handle

received_raw_message = Bytes[0x0a, 0x68, 0x65, 0x6c, 0x6c, 0x6f]
received_message = IPCMessage::TypedMessage.deserialize received_raw_message
pp! received_message
if received_message.nil?
	puts "received message: nil!!!"
else
	s = String.new received_message.payload.not_nil!
	pp! s

	jpayload = "{\"content\":\"hello this is a request\",\"some_number\":30}"
	payload = Bytes.new jpayload.to_unsafe, jpayload.size
	received_message.payload = payload

	begin
		r = Context.requests.parse_ipc_json(received_message)
		pp! r
	rescue e
		puts "error while parsing JSON: #{e}"
	end
end
