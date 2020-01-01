
package=ipc-examples
version=0.5.0

CFLAGS="-Wall -Wextra -g"
LDFLAGS="-I../src -L../ ../src/ipc.h -lipc"

targets=(
	fs-function-tests
	fd-exchange-providing
	fd-exchange-receiving
	fs-experimentations
	pong
	pongd
	pongspam
	simple-tcp-client
	simple-tcpd
	test-ask-for-fd-to-networkd
	test-networkd-provide-fd
	wsserver
)

for i in $targets ; do
	type[$i]=binary
	sources[$i]="$i.c"
	cflags[$i]="-std=c11"
done

dist=(Makefile project.zsh *.c)

