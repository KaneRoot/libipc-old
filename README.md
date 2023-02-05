
# OBSOLETED BY

This project was obsoleted by the [new Zig implementation][zigimpl].
Code is smaller, simpler and safer to use.
Packet format and API are a bit simpler, too.

# libipc

libipc - Simple, easy-to-use IPC library

See the introductory man page in `man/libipc.7`.

See the presentation in [docs/libipc.md](docs/libipc.md).

# Compilation

`make`

# Since 0.7

- `libipc` have callbacks to use along with switching capabilities, making easier to implement proxies for different communication protocols

# Planning for 0.8

For performance improvements within `libipc`:

- `libipc` will be rewritten in Zig -- **DONE!**
- `libipc` shouldn't use realloc for each event (new client, new message, etc.) but by batch of a few thousand elements
- `libipc` should use better internal structures, unrequiring the use of loops (over the whole list of messages or connections) for each action

# Planning for 0.9

- `libipc` should use `epoll/kqueue` for performance improvments
  * new functions will be added to the API
  * **but** we'll keep the same API for applications with no need for threading (way simpler implementation)
- `libipc` should be thread-safe

# Planning for 1.0

- `libipc` should have usable bindings in several languages

[zigimpl]: https://git.baguette.netlib.re/Baguette/libipc
