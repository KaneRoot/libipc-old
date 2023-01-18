all:

ZIGMAKEDOC   = -femit-docs -fno-emit-bin
ZIGOPTIM    ?= Debug
# Linking against libc is almost mandatory, C allocator is used
# for switching (default reception and emission functions).
ZIGBUSEOPTS ?= -O$(ZIGOPTIM) -freference-trace -lc
ZIGUSROPTS  ?=
ZIGC        ?= zig
ZIGOPTS     ?= $(ZIGBUSEOPTS) $(ZIGUSROPTS)

# Debug with valgrind.
ifdef VG_SUPPRESS_WARNINGS
VALGRIND_SUPPRESS_WARNINGS ?= --suppressions=./valgrind.suppr
endif
ifdef VG_GENERATE_SUPPRESSION
VALGRIND_GEN_SUPPRESSION ?= --gen-suppressions=all
endif
ifdef USE_VALGRIND
VALGRIND ?= valgrind $(VALGRIND_SUPPRESS_WARNINGS) $(VALGRIND_GEN_SUPPRESSION) -v --leak-check=full
endif
# Optional parameters (copied here to help with autocompletion).
VG_SUPPRESS_WARNINGS ?=
VG_GENERATE_SUPPRESSION ?=
USE_VALGRIND ?=

ipcd: src/ipcd.zig
	$(ZIGC) build-exe $(ZIGOPTS) $^

tcpd: src/tcpd.zig
	$(ZIGC) build-exe $(ZIGOPTS) $^

test-libipc: src/main.zig
	$(ZIGC) test $(ZIGOPTS) $^

test-ipcd: src/ipcd.zig
	$(ZIGC) test $(ZIGOPTS) $^

doc: src/ipcd.zig
	$(ZIGC) build-exe $(ZIGOPTS) $(ZIGMAKEDOC) $^

TO_CLEAN != ls misc/*.zig | sed 's/.zig$\//' | sed 's_misc/__'
TO_CLEAN += ipcd tcpd pong pongd
TO_CLEAN += *.o
clean:
	@-rm $(TO_CLEAN) 2>/dev/null

mrproper: clean
	@-rm -r docs build zig-cache zig-out 2>/dev/null

ACCESS_LOGS ?= ./access.log
servedoc:
	darkhttpd docs/ --addr 127.0.0.1 --port 35000 --log $(ACCESS_LOGS)

# You can add your specific instructions there.
-include makefile.user
