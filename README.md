
# libipc

libipc - Simple, easy-to-use IPC library

See the introductory [man page](man/libipc.7.md).

# Compilation

`make`


# logging system

Logs are in one of the following directories: `$XDG_DATA_HOME/ipc/` or `$HOME/.local/share/ipc/`.
The log file can be indicated with the `IPC_LOGFILE` environment variable, too.

To remove logs: `make LDFLAGS=-DIPC_WITHOUT_ERRORS`

# Since 0.7

- `libipc` have callbacks to use along with switching capabilities, making easier to implement proxies for different communication protocols

# Planning for 0.8

For performance improvements within `libipc`:

- `libipc` shouldn't use realloc for each event (new client, new message, etc.) but by batch of a few thousand elements
- `libipc` should use better internal structures, unrequiring the use of loops (over the whole list of messages or connections) for each action

# Planning for 0.9

- `libipc` should use `libevent` for performance improvments
- `libipc` should be thread-safe

# Planning for 1.0

- `libipc` *may* be written in Zig
- `libipc` should have usable bindings in several languages


# Implementation design

## Memory management

1. Prefer stack over mallocs.
2. Basic functions (such as *usock_*) should not handle memory allocation.
