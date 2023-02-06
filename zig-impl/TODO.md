### LibIPC not entirely exposed

Some functions are not available in the bindings, mostly functions related to switching.

### MISC

- create the unix socket directory
- close the connection and log when we receive too big messages

### src/exchange-fd.zig

- still very WIP, even though it works as expected
- recvmsg is a very stupid copy of the sendmsg fn, expect errors
- at least one memory error when using Cmsghdr (see below)

	==32374== Syscall param sendmsg(msg.msg_control) points to uninitialised byte(s)
	==32374==    at 0x40554A3: ??? (in /lib/ld-musl-x86_64.so.1)
	==32374==    by 0x40526F9: ??? (in /lib/ld-musl-x86_64.so.1)
	==32374==    by 0x4096B83: ???
	==32374==  Address 0x1ffefff384 is on thread 1's stack
	==32374==  Uninitialised value was created by a client request
	==32374==    at 0x289769: exchange-fd.Cmsghdr(i32).init (exchange-fd.zig:39)
	==32374==    by 0x2808F0: exchange-fd.send_fd (exchange-fd.zig:86)
	==32374==    by 0x27EA97: ipcd.create_service (ipcd.zig:178)
	==32374==    by 0x28117C: ipcd.main (ipcd.zig:224)
	==32374==    by 0x28161E: callMain (start.zig:614)
	==32374==    by 0x28161E: initEventLoopAndCallMain (start.zig:548)
	==32374==    by 0x28161E: callMainWithArgs (start.zig:498)
	==32374==    by 0x28161E: main (start.zig:513)

### makefile

- distribution

### documentation

- manpages for ipcd, tcpd, pong, pongd
