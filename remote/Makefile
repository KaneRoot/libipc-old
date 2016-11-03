CC=gcc
CFLAGS=-Wall -g
LDFLAGS= -pthread
CFILES=$(wildcard *.c) # CFILES => recompiles everything on a C file change
EXEC=$(basename $(wildcard *.c))
SOURCES=$(wildcard ../lib/communication.c ../lib/process.c)
OBJECTS=$(SOURCES:.c=.o)
TESTS=$(addsuffix .test, $(EXEC))

all: $(SOURCES) $(EXEC)

$(EXEC): $(OBJECTS) $(CFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $@.c -lcbor -o $@.bin

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

$(TESTS):
	valgrind --show-leak-kinds=all --leak-check=full -v --track-origins=yes ./$(basename $@).bin

clean:
	@-rm $(OBJECTS)

mrproper: clean
	@-rm *.bin

cleantmp:
	rm /tmp/ipc/*
