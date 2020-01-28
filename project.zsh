
package=ipc
version=0.5.1

variables+=(CFLAGS "-Wall -Wextra -g")

targets=(libipc src/ipc.h src/usocket.h src/message.h src/fs.h src/utils.h man/libipc.7)
type[libipc]=library
sources[libipc]="$(ls src/*.c)"
cflags[libipc]="-std=c11"

type[src/ipc.h]=header
type[src/usocket.h]=header
type[src/message.h]=header
type[src/fs.h]=header
type[src/utils.h]=header

type[man/libipc.7]=scdocman

dist=(Makefile project.zsh src/ipc.h man/*.scd)

