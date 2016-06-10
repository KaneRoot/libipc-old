
package=perfect-os-junk
version=0.0.1

CFLAGS="-O2 -Wall -Wextra -Wshadow -ansi -pedantic -std=c99 -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=199309L"

targets=(libposj)
type[libposj]=library
sources[libposj]="$(echo lib/*.c)"

target="pingpong/pingpong"
targets+=(${target})
sources[${target}]="$(echo pingpong/*.c)"
type[${target}]=binary
depends[${target}]="libposj.a"
ldflags[${target}]="libposj.a -lpthread"

target="pubsub/pubsub"
targets+=(${target})
sources[${target}]="$(ls pubsub/*.c | grep -v test-send)"
type[${target}]=binary
depends[${target}]="libposj.a"
ldflags[${target}]="libposj.a -lpthread"

target="pubsub/pubsub-test-send"
targets+=(${target})
sources[${target}]="pubsub/pubsub-test-send.c"
type[${target}]=binary
depends[${target}]="libposj.a"
ldflags[${target}]="libposj.a -lpthread"

