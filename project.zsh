
package=perfect-os-junk
version=0.0.1

targets=(libposj)
type[libposj]=library
sources[libposj]="$(echo lib/*.c)"

for i in *.c; do
	targets+=(${i%.c})
	sources[${i%.c}]="$i $(echo lib/*.c)"
	type[${i%.c}]=binary
done

