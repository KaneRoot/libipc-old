
package=ipc
version=0.1.0

targets=(libipc src/ipc.h man/libipc.7)

type[libipc]=library
sources[libipc]="$(ls src/*.c)"

type[src/ipc.h]=header

type[man/libipc.7]=man

dist=(Makefile project.zsh src/ipc.h man/*.md)

