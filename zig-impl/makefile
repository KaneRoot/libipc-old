all:

ZIGMAKEDOC   = -femit-docs -fno-emit-bin
ZIGOPTIM    ?= Debug
ZIGBUSEOPTS ?= -O$(ZIGOPTIM) -freference-trace
ZIGUSROPTS  ?=
ZIGC        ?= zig
ZIGOPTS     ?= $(ZIGBUSEOPTS) $(ZIGUSROPTS)

ipcd: src/ipcd.zig
	$(ZIGC) build-exe $(ZIGOPTS) $^

test-libipc: src/main.zig
	$(ZIGC) test $(ZIGOPTS) $^

test-ipcd: src/ipcd.zig
	$(ZIGC) test $(ZIGOPTS) $^

doc: src/ipcd.zig
	$(ZIGC) build-exe $(ZIGOPTS) $(ZIGMAKEDOC) $^


# You can add your specific instructions there.
-include makefile.user
