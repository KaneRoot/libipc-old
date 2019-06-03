#!/bin/sh

nontapmsg() {
	echo $*
}

for i in *.c
do
	f=`echo $i | sed "s/.c$//"`
	nontapmsg "compiling $f"
	# gcc $f.c ./lib/*.o -o $f -I../src -I ./lib/ -L../ -L./lib/ -lipc -Wall -g -Wextra
	gcc -Wall -g -Wextra $f.c -o $f -I../src -L../ ../src/ipc.h -lipc
done
