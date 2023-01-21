LDPATH ?= /tmp/libipc/zig-impl/build
SRC ?= ./bin/some-crystal-app

VG_OPTS  = --leak-check=full -v
VG_OPTS += --show-leak-kinds=all
VG_OPTS += --suppressions=valgrind.suppressions 
VG_OPTS += --gen-suppressions=all

build:
	CRYSTAL_LIBRARY_PATH=$(LDPATH) shards build

valgrind:
	LD_LIBRARY_PATH=$(LDPATH) valgrind $(VG_OPTS) $(SRC)

run:
	LD_LIBRARY_PATH=$(LDPATH) $(SRC)
