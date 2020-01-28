
package=ipc-tests
version=0.5.0

variables+=(
	CFLAGS "-Wall -Wextra -g"
	LDFLAGS "-I../src -L../ ../src/ipc.h -lipc"
)

targets=(
	func_01_connection_establishment
	func_01_connection_establishmentd
	func_02_pong
	func_02_pongd
	func_03_multiple-communications-client
	func_03_multiple-communications-server
	func_04_empty_message
	func_05_read-write-loop
	unit_01_service-path
	unit_02_usock-remove
	unit_03_connection-add-remove
)

for i in $targets ; do
	type[$i]=binary
	sources[$i]="$i.c"
	cflags[$i]="-std=c11"
done

dist=(Makefile project.zsh *.c)

