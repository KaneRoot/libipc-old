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
VALGRIND_OPTS=-v --leak-check=full --track-origins=yes
ifdef USE_VALGRIND
VALGRIND ?= valgrind $(VALGRIND_SUPPRESS_WARNINGS) \
	$(VALGRIND_GEN_SUPPRESSION) \
	$(VALGRIND_OPTS)
endif
# Optional parameters (copied here to help with autocompletion).
VG_SUPPRESS_WARNINGS ?=
VG_GENERATE_SUPPRESSION ?=
USE_VALGRIND ?=

TO_CLEAN != ls misc/*.zig | sed 's/.zig$\//' | sed 's_misc/__'
TO_CLEAN += bin/ipcd bin/tcpd bin/pong bin/pongd
TO_CLEAN += bin/*.o
clean:
	@-rm $(TO_CLEAN) 2>/dev/null

mrproper: clean
	@-rm -r docs build zig-cache zig-out 2>/dev/null

doc: src/ipcd.zig
	$(ZIGC) build-exe $(ZIGOPTS) $(ZIGMAKEDOC) $^

ACCESS_LOGS ?= ./access.log
servedoc:
	darkhttpd docs/ --addr 127.0.0.1 --port 35000 --log $(ACCESS_LOGS)

# You can add your specific instructions there.
-include makefile.user
