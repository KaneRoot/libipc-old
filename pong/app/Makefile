CC=gcc
CFLAGS=-Wall -g -Wextra
LDFLAGS= -pthread
CFILES=$(wildcard *.c) # CFILES => recompiles everything on a C file change
EXEC=$(basename $(wildcard *.c))
SOURCES=$(wildcard ../lib/*.c ../../core/*.c)
OBJECTS=$(SOURCES:.c=.o)
TESTS=$(addsuffix .test, $(EXEC))

all: $(SOURCES) $(EXEC)

test:
	rm /tmp/ipc/pongd-0-0
	./pongd.bin

$(EXEC): $(OBJECTS) $(CFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $@.c -o $@.bin

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

$(TESTS):
	valgrind --show-leak-kinds=all --leak-check=full -v --track-origins=yes ./$(basename $@).bin $(PARAMS)

clean:
	@-rm $(OBJECTS)

mrproper: clean
	@-rm *.bin
