all:

ZIGBUSEOPTS ?= -OReleaseSmall
ZIGUSROPTS  ?= -freference-trace
ZIGC        ?= zig
ZIGOPTS     ?= $(ZIGBUSEOPTS) $(ZIGUSROPTS)

c: src/send-msg.zig
	$(ZIGC) build-exe $(ZIGOPTS) $^

s: src/receive-msg.zig
	$(ZIGC) build-exe $(ZIGOPTS) $^

m: src/main.zig
	$(ZIGC) build-exe $(ZIGOPTS) $^

t: src/main.zig
	$(ZIGC) test $(ZIGOPTS) $^

d: src/main.zig
	$(ZIGC) build-exe $(ZIGOPTS) $^

o: $(SRC)
	$(ZIGC) test $(ZIGOPTS) $^
