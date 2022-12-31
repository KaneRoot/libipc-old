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

TO_CLEAN != ls misc/*.zig | sed 's/.zig$\//' | sed 's_misc/__'
TO_CLEAN += ipcd
TO_CLEAN += *.o
clean:
	@-rm $(TO_CLEAN) 2>/dev/null

mrproper: clean
	@-rm -r docs zig-cache zig-out 2>/dev/null

ACCESS_LOGS ?= ./access.log
servedoc:
	darkhttpd docs/ --addr 127.0.0.1 --port 35000 --log $(ACCESS_LOGS)

# You can add your specific instructions there.
-include makefile.user
