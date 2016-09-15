CC=gcc
CFLAGS=-Wall -g
LDFLAGS= -pthread
CFILES=$(wildcard *.c) # CFILES => recompiles everything on a C file change
EXEC=$(basename $(wildcard *.c))
SOURCES=$(wildcard ../lib/*.c)
OBJECTS=$(SOURCES:.c=.o)
TESTS=$(addsuffix .test, $(EXEC))

all: tcpdserver tcpdclient

$(EXEC): $(OBJECTS) $(CFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $@.c -o $@

tcpdserver: tcpdserver.c tcpdserver.h
	$(CC) $(CFLAGS) $@.c -o $@

tcpdclient: tcpdclient.c tcpdserver.h
	$(CC) $(CFLAGS) $@.c -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	-rm $(OBJECTS)

mrproper: clean
	rm $(EXEC)
