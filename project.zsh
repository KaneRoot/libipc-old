
package=ipc
version=0.1.0

CFLAGS="-Wall -Wextra -g"

targets=(libipc src/ipc.h man/libipc.7)

type[libipc]=library
sources[libipc]="$(ls src/*.c)"
cflags[libipc]="-std=c11"

type[src/ipc.h]=header

type[man/libipc.7]=scdocman

dist=(Makefile project.zsh src/ipc.h man/*.scd)

