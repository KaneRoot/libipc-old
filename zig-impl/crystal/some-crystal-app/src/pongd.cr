require "./some-crystal-app.cr"

# In 5 messages: quit
count = 5

ipc = IPC.new
fd = ipc.service_init("pong")

ipc.loop do |event|
	case event.type
	when LibIPC::EventType::MessageRx
		m = event.message
		if m.nil?
			puts "No message"
		else
			received = String.new(m.to_unsafe, m.size)
			pp! received
			ipc.schedule event.fd, m, m.size
		end

	when LibIPC::EventType::MessageTx
		puts "A message has been sent"
		count -= 1
		if count == 0
			exit
		end

	when LibIPC::EventType::Connection
		puts "A client just connected #JOY"

	when LibIPC::EventType::Disconnection
		puts "A client just disconnected #SAD"

	else
		puts "Unexpected: #{event.type}"
		exit 1
	end
end
