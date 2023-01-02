const std = @import("std");
const testing = std.testing;
const net = std.net;
const fmt = std.fmt;
const os = std.os;
const mem = std.mem;
const print = std.debug.print;

const Cmsghdr = @import("./cmsghdr.zig").Cmsghdr;

const system = std.os.system;
const socket_t = std.os.socket_t;
const msghdr = system.msghdr;
const builtin = @import("builtin");
const windows = std.os.windows;
const errno   = std.os.errno;
const unexpectedErrno = std.os.unexpectedErrno;
const SendMsgError = std.os.SendMsgError;

pub fn recvmsg(
    /// The file descriptor of the sending socket.
    sockfd: socket_t,
    /// Message header and iovecs
    msg: msghdr,
    flags: u32,
) SendMsgError!usize {
    while (true) {
        var m = msg;
        const rc = system.recvmsg(sockfd, @ptrCast(*std.x.os.Socket.Message, &m), @intCast(c_int, flags));
        if (builtin.os.tag == .windows) {
            if (rc == windows.ws2_32.SOCKET_ERROR) {
                switch (windows.ws2_32.WSAGetLastError()) {
                    .WSAEACCES => return error.AccessDenied,
                    .WSAEADDRNOTAVAIL => return error.AddressNotAvailable,
                    .WSAECONNRESET => return error.ConnectionResetByPeer,
                    .WSAEMSGSIZE => return error.MessageTooBig,
                    .WSAENOBUFS => return error.SystemResources,
                    .WSAENOTSOCK => return error.FileDescriptorNotASocket,
                    .WSAEAFNOSUPPORT => return error.AddressFamilyNotSupported,
                    .WSAEDESTADDRREQ => unreachable, // A destination address is required.
                    .WSAEFAULT => unreachable, // The lpBuffers, lpTo, lpOverlapped, lpNumberOfBytesSent, or lpCompletionRoutine parameters are not part of the user address space, or the lpTo parameter is too small.
                    .WSAEHOSTUNREACH => return error.NetworkUnreachable,
                    // TODO: WSAEINPROGRESS, WSAEINTR
                    .WSAEINVAL => unreachable,
                    .WSAENETDOWN => return error.NetworkSubsystemFailed,
                    .WSAENETRESET => return error.ConnectionResetByPeer,
                    .WSAENETUNREACH => return error.NetworkUnreachable,
                    .WSAENOTCONN => return error.SocketNotConnected,
                    .WSAESHUTDOWN => unreachable, // The socket has been shut down; it is not possible to WSASendTo on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH.
                    .WSAEWOULDBLOCK => return error.WouldBlock,
                    .WSANOTINITIALISED => unreachable, // A successful WSAStartup call must occur before using this function.
                    else => |err| return windows.unexpectedWSAError(err),
                }
            } else {
                return @intCast(usize, rc);
            }
        } else {
            switch (errno(rc)) {
                .SUCCESS => return @intCast(usize, rc),

                .ACCES => return error.AccessDenied,
                .AGAIN => return error.WouldBlock,
                .ALREADY => return error.FastOpenAlreadyInProgress,
                .BADF => unreachable, // always a race condition
                .CONNRESET => return error.ConnectionResetByPeer,
                .DESTADDRREQ => unreachable, // The socket is not connection-mode, and no peer address is set.
                .FAULT => unreachable, // An invalid user space address was specified for an argument.
                .INTR => continue,
                .INVAL => unreachable, // Invalid argument passed.
                .ISCONN => unreachable, // connection-mode socket was connected already but a recipient was specified
                .MSGSIZE => return error.MessageTooBig,
                .NOBUFS => return error.SystemResources,
                .NOMEM => return error.SystemResources,
                .NOTSOCK => unreachable, // The file descriptor sockfd does not refer to a socket.
                .OPNOTSUPP => unreachable, // Some bit in the flags argument is inappropriate for the socket type.
                .PIPE => return error.BrokenPipe,
                .AFNOSUPPORT => return error.AddressFamilyNotSupported,
                .LOOP => return error.SymLinkLoop,
                .NAMETOOLONG => return error.NameTooLong,
                .NOENT => return error.FileNotFound,
                .NOTDIR => return error.NotDir,
                .HOSTUNREACH => return error.NetworkUnreachable,
                .NETUNREACH => return error.NetworkUnreachable,
                .NOTCONN => return error.SocketNotConnected,
                .NETDOWN => return error.NetworkSubsystemFailed,
                else => |err| return unexpectedErrno(err),
            }
        }
    }
}

fn disconnect(stream: *net.StreamServer) void { stream.close(); }

fn server_init() net.StreamServer {
    // no reuse_address and default kernel_backlog
    return net.StreamServer.init(.{});
}

fn waiting_for_connection(stream: *net.StreamServer
  , path: []const u8) !net.StreamServer.Connection {
    var address = try net.Address.initUnix(path);
    try stream.listen(address);
    return stream.accept();
}

fn remove_unix_socket(path: []const u8) void {
    std.fs.deleteFileAbsolute(path) catch |err| switch(err) {
        else => { print("error: {}\n", .{err}); }
    };
}

const SCM_RIGHTS: c_int = 1;

fn send_msg(sock: os.socket_t, msg: []const u8, fd: os.fd_t) void {
    var iov = [_]os.iovec_const{
        .{
            .iov_base = msg.ptr,
            .iov_len = msg.len,
        },
    };

    var cmsg = Cmsghdr(os.fd_t).init(.{
        .level = os.SOL.SOCKET,
        .@"type" = SCM_RIGHTS,
        .data = fd,
    });

    const len = os.sendmsg(sock, .{
        .name = undefined,
        .namelen = 0,
        .iov = &iov,
        .iovlen = iov.len,
        .control = &cmsg,
        .controllen = @sizeOf(@TypeOf(cmsg)),
        .flags = 0,
        }, 0) catch |err| {
        print("error sendmsg failed with {s}", .{@errorName(err)});
        return;
    };

    if (len != msg.len) {
        // we don't have much choice but to exit here
        // log.err(@src(), "expected sendmsg to return {} but got {}", .{msg.len, len});
        print("expected sendmsg to return {} but got {}", .{msg.len, len});
        os.exit(0xff);
    }
}

fn receive_msg(sock: os.socket_t) !os.fd_t {
    var buffer: [100]u8 = undefined;

    var iov = [1]os.iovec{
        .{
            .iov_base = buffer[0..],
            .iov_len = buffer.len,
        },
    };

    var cmsg = Cmsghdr(os.fd_t).init(.{
        .level = os.SOL.SOCKET,
        .@"type" = SCM_RIGHTS,
        .data = undefined,
    });

    var msg: std.os.msghdr = .{
        .name = undefined,
        .namelen = 0,
        .iov = &iov,
        .iovlen = iov.len,
        .control = &cmsg,
        .controllen = @sizeOf(@TypeOf(cmsg)),
        .flags = 0,
        };

    const len = recvmsg(sock, msg, 0) catch |err| {
        print("error sendmsg failed with {s}", .{@errorName(err)});
        return 0;
    };

    print("received {} bytes, fd is {}\n", .{len, @as(i32, cmsg.dataPtr().*)});
    print("iov base {s}\n", .{iov[0].iov_base[0..iov[0].iov_len - 1]});
    return @as(i32, cmsg.dataPtr().*);
}

fn add_line_from_fd(fd: i32) !void {
    // var f = std.fs.File {.handle = fd};
    // defer f.close();
    _ = try std.os.write(fd, "hello this is another line\n");
    std.os.close(fd);
}

pub fn main() !u8 {
    var path = "/tmp/.TEST_USOCK";
    print("Init UNIX server to {s}...\n", .{path});
    var stream = server_init();
    defer stream.deinit();
    print("Waiting for a connection...\n", .{});
    var connection = try waiting_for_connection(&stream, path);
    defer remove_unix_socket(path);
    print("Someone is connected! Receiving a file descriptor...\n", .{});
    var fd = try receive_msg(connection.stream.handle);
    print("FD received, writing a line into the file...\n", .{});
    try add_line_from_fd(fd);
    print("Disconnection...\n", .{});
    disconnect(&stream);
    print("Disconnected!\n", .{});
    return 0;
}
