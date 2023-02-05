const std = @import("std");
const testing = std.testing;
const os = std.os;
const log = std.log.scoped(.libipc_exchangefd);

const builtin = @import("builtin");
const windows = std.os.windows;
const errno   = std.os.errno;
const system = std.os.system;
const unexpectedErrno = std.os.unexpectedErrno;
const SendMsgError = std.os.SendMsgError;

const SCM_RIGHTS: c_int = 1;

/// This definition enables the use of Zig types with a cmsghdr structure.
/// The oddity of this layout is that the data must be aligned to @sizeOf(usize)
/// rather than its natural alignment.
pub fn Cmsghdr(comptime T: type) type {
    const Header = extern struct {
        len: usize,
        level: c_int,
        @"type": c_int,
    };

    const data_align = @sizeOf(usize);
    const data_offset = std.mem.alignForward(@sizeOf(Header), data_align);

    return extern struct {
        const Self = @This();

        bytes: [data_offset + @sizeOf(T)]u8 align(@alignOf(Header)),

        pub fn init(args: struct {
            level: c_int,
            @"type": c_int,
            data: T,
        }) Self {
            var self: Self = undefined;
            self.headerPtr().* = .{
                .len = data_offset + @sizeOf(T),
                .level = args.level,
                .@"type" = args.@"type",
            };
            self.dataPtr().* = args.data;
            return self;
        }

        // TODO: include this version if we submit a PR to add this to std
        pub fn initNoData(args: struct {
            level: c_int,
            @"type": c_int,
        }) Self {
            var self: Self = undefined;
            self.headerPtr().* = .{
                .len = data_offset + @sizeOf(T),
                .level = args.level,
                .@"type" = args.@"type",
            };
            return self;
        }

        pub fn headerPtr(self: *Self) *Header {
            return @ptrCast(*Header, self);
        }
        pub fn dataPtr(self: *Self) *align(data_align) T {
            return @ptrCast(*T, self.bytes[data_offset..]);
        }
    };
}

test {
    std.testing.refAllDecls(Cmsghdr([3]std.os.fd_t));
}

/// Send a file descriptor and a message through a UNIX socket.
/// TODO: currently voluntarily crashes if data isn't sent properly, should return an error instead.
pub fn send_fd(sockfd: os.socket_t, msg: []const u8, fd: os.fd_t) void {
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

    const len = os.sendmsg(sockfd, .{
        .name = null,
        .namelen = 0,
        .iov = &iov,
        .iovlen = iov.len,
        .control = &cmsg,
        .controllen = @sizeOf(@TypeOf(cmsg)),
        .flags = 0,
        }, 0) catch |err| {
        log.err("error sendmsg failed with {s}", .{@errorName(err)});
        return;
    };

    if (len != msg.len) {
        // We don't have much choice but to exit here.
        log.err("expected sendmsg to return {} but got {}", .{msg.len, len});
        os.exit(0xff);
    }
}

/// WARNING: recvmsg is a WIP.
/// WARNING: errors aren't RECEPTION errors.
/// WARNING: can only work on linux for now (recvmsg is lacking on other systems).
pub fn recvmsg(
    /// The file descriptor of the sending socket.
    sockfd: os.socket_t,
    /// Message header and iovecs
    msg: std.os.msghdr,
    flags: u32,
) SendMsgError!usize {
    while (true) {
        var m = msg;
        const rc = system.recvmsg(sockfd, @ptrCast(*std.os.msghdr, &m), @intCast(c_uint, flags));
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

/// Receive a file descriptor through a UNIX socket.
/// A message can be carried with it, copied into 'buffer'.
/// WARNING: buffer must be at least 1500 bytes.
pub fn receive_fd(sockfd: os.socket_t, buffer: []u8, msg_size: *usize) !os.fd_t {

    var msg_buffer = [_]u8{0} ** 1500;

    var iov = [_]os.iovec{
        .{
              .iov_base = msg_buffer[0..]
            , .iov_len  = msg_buffer.len
        },
    };

    var cmsg = Cmsghdr(os.fd_t).init(.{
        .level = os.SOL.SOCKET,
        .@"type" = SCM_RIGHTS,
        .data = 0,
    });

    var msg: std.os.msghdr = .{
          .name = null
        , .namelen = 0
        , .iov = &iov
        , .iovlen = 1
        , .control = &cmsg
        , .controllen = @sizeOf(@TypeOf(cmsg))
        , .flags = 0
    };

    var msglen = recvmsg(sockfd, msg, 0) catch |err| {
        log.err("error recvmsg failed with {s}", .{@errorName(err)});
        return 0;
    };

    var received_fd = @as(i32, cmsg.dataPtr().*);
    std.mem.copy(u8, buffer, &msg_buffer);
    msg_size.* = msglen;

    return received_fd;
}
