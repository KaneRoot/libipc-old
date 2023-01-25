require "./some-crystal-app.cr"

def test_high_level
	ipc = IPC.new
	fd = ipc.connect("pong")
	ipc.write(fd, "hello this is some value")
	event = ipc.wait()

	m = event.message
	if m.nil?
		puts "No message"
	else
		pp! String.new(m.to_unsafe, m.size)
	end
end

def test_loop
	ipc = IPC.new
	fd = ipc.connect("pong")
	ipc.schedule(fd, "hello this is some value")
	ipc.loop do |event|
		case event.type
		when LibIPC::EventType::MessageRx
			m = event.message
			if m.nil?
				puts "No message"
			else
				pp! String.new(m.to_unsafe, m.size)
			end
			exit 0
		when LibIPC::EventType::MessageTx
			puts "A message has been sent"
		else
			puts "Unexpected: #{event.type}"
			exit 1
		end
	end
end

# TODO: Write documentation for `Some::Crystal::App`
module Some::Crystal::App
	VERSION = "0.1.0"

	test_high_level
	test_loop
end
