all:

ZIGC = zig
ZIGOPTS = -OReleaseSmall

c: src/send-msg.zig
	$(ZIGC) build-exe $(ZIGOPTS) $^

s: src/receive-msg.zig
	$(ZIGC) build-exe $(ZIGOPTS) $^

m: src/main.zig
	$(ZIGC) build-exe $(ZIGOPTS) $^

d: src/main.zig
	$(ZIGC) build-exe $(ZIGOPTS) -freference-trace $^
