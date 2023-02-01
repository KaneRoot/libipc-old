
def test_without_wait()
	ctx = Pointer(Void).null
	LibIPC.init (pointerof(ctx))
	fd : Int32 = 0
	LibIPC.connect_service(ctx, pointerof(fd), "pong", 4)
	pp! fd
	LibIPC.write(ctx, fd, "Hello", 5)

	buflen : LibC::UInt64T = 10
	buffer = uninitialized UInt8[10]
	LibIPC.read(ctx, fd, buffer.to_unsafe, pointerof(buflen))
	received = String.new(buffer.to_unsafe, buflen)
	pp! received

	LibIPC.deinit (pointerof(ctx))
end

def test_with_wait()
	ctx = Pointer(Void).null
	LibIPC.init (pointerof(ctx))
	fd : Int32 = 0
	LibIPC.connect_service(ctx, pointerof(fd), "pong", 4)
	LibIPC.write(ctx, fd, "Hello", 5)

	buflen : LibC::UInt64T = 10
	buffer = uninitialized UInt8[10]
	eventtype : UInt8 = 0
	index : LibC::UInt64T = 0

	LibIPC.timer(ctx, 2000) # Wait at most 2 seconds.
	LibIPC.wait(ctx, pointerof(eventtype), pointerof(index), pointerof(fd), buffer.to_unsafe, pointerof(buflen))

	#pp! LibIPC::EventType.new(eventtype), fd, index, buflen
	received = String.new(buffer.to_unsafe, buflen)
	pp! received

	LibIPC.deinit (pointerof(ctx))
end
