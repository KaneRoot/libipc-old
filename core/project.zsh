
package=ipc
version=0.0.1

targets=(libipc)
type[libipc]=library
sources[libipc]="$(ls *.c)"

dist=(Makefile project.zsh error.h ipc.h event.h)

