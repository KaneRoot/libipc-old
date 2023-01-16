ZIGC=zig

CC=gcc
CFLAGS=-Wall -Wextra
LDFLAGS=-I build/ -L build/ -lipc

all: zigcompilation compilation

ifeq ($(SRC),)
test-src:
	@echo SRC must be set via command line.
	@exit 1
else
test-src:
endif

list-obj-files: test-src
	@# List all .o included in a .a archive.
	ar t $(SRC)
list-symbols: test-src
	@# List all symbols in a .so.
	nm -D $(SRC)
list-symbols-alt: test-src
	@# Alternative: grep .text section in an objdump output.
	objdump -T $(SRC) | grep text

zigcompilation: build.zig src/*.zig
	$(ZIGC) build

compilation: src/main.c
	@echo the following compilation will produce errors despite actually working
	$(CC) -o app -o main build/libipc.so $(CFLAGS) $^ $(LDFLAGS)

run:
	LD_LIBRARY_PATH=build ./main

valgrind:
	LD_LIBRARY_PATH=build valgrind --suppressions=./suppress-stuff.suppr --gen-suppressions=all -v --leak-check=full ./main
