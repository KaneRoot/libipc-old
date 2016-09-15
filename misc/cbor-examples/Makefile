CC=gcc
CFLAGS=-Wall -g -Wextra
LDFLAGS= -pthread -lcbor
CFILES=$(wildcard *.c) # CFILES => recompiles everything on a C file change
EXEC=$(basename $(wildcard *.c))
SOURCES=$(wildcard ../lib/*.c)
OBJECTS=$(SOURCES:.c=.o)
TESTS=$(addsuffix .test, $(EXEC))

all: $(SOURCES) $(EXEC)

$(EXEC): $(OBJECTS) $(CFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $@.c -lcbor -o $@.bin

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

mrproper:
	rm *.bin
