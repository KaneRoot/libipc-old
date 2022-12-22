all:

ZIGC = zig
ZIGOPTS = -OReleaseSmall $(ZIGUSROPTS)

c: src/send-msg.zig
	$(ZIGC) build-exe $(ZIGOPTS) $^

s: src/receive-msg.zig
	$(ZIGC) build-exe $(ZIGOPTS) $^

m: src/main.zig
	$(ZIGC) build-exe $(ZIGOPTS) $^

t: src/main.zig
	$(ZIGC) test $(ZIGOPTS) $^

d: src/main.zig
	$(ZIGC) build-exe $(ZIGOPTS) -freference-trace $^

o: $(SRC)
	$(ZIGC) test $(ZIGOPTS) $^
