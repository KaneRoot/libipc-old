#!/bin/sh

nontapmsg() {
	echo $*
}

for i in *.c
do
	f=`echo $i | sed "s/.c$//"`
	nontapmsg "compiling $f"
	gcc $f.c -Wall -Wextra -Wno-unused-parameter -g -o $f.bin -I../src -L../ -lipc
done
