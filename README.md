
# libipc

libipc - Simple, easy-to-use IPC library

See the introductory [man page](man/libipc.7.md).

# Compilation

`make`


# logging system

Logs are in one of the following directories: `$XDG_DATA_HOME/ipc/` or `$HOME/.local/share/ipc/`.
The log file can be indicated with the `IPC_LOGFILE` environment variable, too.

To remove logs: `make LDFLAGS=-DIPC_WITHOUT_ERRORS`
