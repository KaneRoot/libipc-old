CC=gcc
CFLAGS=-Wall -g
LDFLAGS=
CFILES=$(wildcard *.c) # CFILES => recompiles everything on a C file change
EXEC=$(basename $(wildcard *.c))
SOURCES=$(wildcard ../lib/*.c)
OBJECTS=$(SOURCES:.c=.o)
TESTS=$(addsuffix .test, $(EXEC))

all: $(SOURCES) $(EXEC)

$(EXEC): $(OBJECTS) $(CFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $@.c -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	-rm $(OBJECTS)

mrproper: clean
	rm $(EXEC)
