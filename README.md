
# libipc

libipc - Simple, easy-to-use IPC library

See the introductory man page in `man/libipc.7`.

> This library is a work in progress, but is already used intensively in production.
> It works, but we provide no warranty.

# Compilation

`make`

# Since 0.7

- `libipc` have callbacks to use along with switching capabilities, making easier to implement proxies for different communication protocols

# Planning for 0.8

For performance improvements within `libipc`:

- `libipc` shouldn't use realloc for each event (new client, new message, etc.) but by batch of a few thousand elements
- `libipc` should use better internal structures, unrequiring the use of loops (over the whole list of messages or connections) for each action
- `libipc` will be rewritten in Zig

# Planning for 0.9

- `libipc` should use `libevent` for performance improvments
- `libipc` should be thread-safe

# Planning for 1.0

- `libipc` should have usable bindings in several languages


# Implementation design

## Memory management

1. Prefer stack over mallocs.
2. Basic functions (such as *usock_*) should not handle memory allocation.
