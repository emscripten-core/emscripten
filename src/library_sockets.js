/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if PROXY_POSIX_SOCKETS
#error "not compatible with PROXY_POSIX_SOCKETS"
#endif

var SocketsLibrary = {
  $getSocketFromFD__deps: ['$SOCKFS', '$FS'],
  $getSocketFromFD: function(fd) {
    var socket = SOCKFS.getSocket(fd);
    if (!socket) throw new FS.ErrnoError({{{ cDefine('EBADF') }}});
#if SYSCALL_DEBUG
    err('    (socket: "' + socket.path + '")');
#endif
    return socket;
  },
  /** @param {boolean=} allowNull */
  $getSocketAddress__deps: ['$readSockaddr', '$FS', '$DNS'],
  $getSocketAddress__docs: '/** @param {boolean=} allowNull */',
  $getSocketAddress: function(addrp, addrlen, allowNull) {
    if (allowNull && addrp === 0) return null;
    var info = readSockaddr(addrp, addrlen);
    if (info.errno) throw new FS.ErrnoError(info.errno);
    info.addr = DNS.lookup_addr(info.addr) || info.addr;
#if SYSCALL_DEBUG
    err('    (socketaddress: "' + [info.addr, info.port] + '")');
#endif
    return info;
  },
  __syscall_socket__deps: ['$SOCKFS'],
  __syscall_socket: function(domain, type, protocol) {
    var sock = SOCKFS.createSocket(domain, type, protocol);
#if ASSERTIONS
    assert(sock.stream.fd < 64); // XXX ? select() assumes socket fd values are in 0..63
#endif
    return sock.stream.fd;
  },
  __syscall_getsockname__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_getsockname: function(fd, addr, addrlen) {
    err("__syscall_getsockname " + fd);
    var sock = getSocketFromFD(fd);
    // TODO: sock.saddr should never be undefined, see TODO in websocket_sock_ops.getname
    var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(sock.saddr || '0.0.0.0'), sock.sport, addrlen);
#if ASSERTIONS
    assert(!errno);
#endif
    return 0;
  },
  __syscall_getpeername__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_getpeername: function(fd, addr, addrlen) {
    var sock = getSocketFromFD(fd);
    if (!sock.daddr) {
      return -{{{ cDefine('ENOTCONN') }}}; // The socket is not connected.
    }
    var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(sock.daddr), sock.dport, addrlen);
#if ASSERTIONS
    assert(!errno);
#endif
    return 0;
  },
  __syscall_connect__deps: ['$getSocketFromFD', '$getSocketAddress'],
  __syscall_connect: function(fd, addr, addrlen) {
    var sock = getSocketFromFD(fd);
    var info = getSocketAddress(addr, addrlen);
    sock.sock_ops.connect(sock, info.addr, info.port);
    return 0;
  },
  __syscall_shutdown__deps: ['$getSocketFromFD'],
  __syscall_shutdown: function(fd, how) {
    getSocketFromFD(fd);
    return -{{{ cDefine('ENOSYS') }}}; // unsupported feature
  },
  __syscall_accept4__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_accept4: function(fd, addr, addrlen, flags) {
    var sock = getSocketFromFD(fd);
    var newsock = sock.sock_ops.accept(sock);
    if (addr) {
      var errno = writeSockaddr(addr, newsock.family, DNS.lookup_name(newsock.daddr), newsock.dport, addrlen);
#if ASSERTIONS
      assert(!errno);
#endif
    }
    return newsock.stream.fd;
  },
  __syscall_bind__deps: ['$getSocketFromFD', '$getSocketAddress'],
  __syscall_bind: function(fd, addr, addrlen) {
    var sock = getSocketFromFD(fd);
    var info = getSocketAddress(addr, addrlen);
    sock.sock_ops.bind(sock, info.addr, info.port);
    return 0;
  },
  __syscall_listen__deps: ['$getSocketFromFD'],
  __syscall_listen: function(fd, backlog) {
    var sock = getSocketFromFD(fd);
    sock.sock_ops.listen(sock, backlog);
    return 0;
  },
  __syscall_recvfrom__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_recvfrom: function(fd, buf, len, flags, addr, addrlen) {
    var sock = getSocketFromFD(fd);
    var msg = sock.sock_ops.recvmsg(sock, len);
    if (!msg) return 0; // socket is closed
    if (addr) {
      var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(msg.addr), msg.port, addrlen);
#if ASSERTIONS
      assert(!errno);
#endif
    }
    HEAPU8.set(msg.buffer, buf);
    return msg.buffer.byteLength;
  },
  __syscall_sendto__deps: ['$getSocketFromFD', '$getSocketAddress'],
  __syscall_sendto: function(fd, message, length, flags, addr, addr_len) {
    var sock = getSocketFromFD(fd);
    var dest = getSocketAddress(addr, addr_len, true);
    if (!dest) {
      // send, no address provided
      return FS.write(sock.stream, {{{ heapAndOffset('HEAP8', 'message') }}}, length);
    } else {
      // sendto an address
      return sock.sock_ops.sendmsg(sock, {{{ heapAndOffset('HEAP8', 'message') }}}, length, dest.addr, dest.port);
    }
  },
  __syscall_getsockopt__deps: ['$getSocketFromFD'],
  __syscall_getsockopt: function(fd, level, optname, optval, optlen) {
    var sock = getSocketFromFD(fd);
    // Minimal getsockopt aimed at resolving https://github.com/emscripten-core/emscripten/issues/2211
    // so only supports SOL_SOCKET with SO_ERROR.
    if (level === {{{ cDefine('SOL_SOCKET') }}}) {
      if (optname === {{{ cDefine('SO_ERROR') }}}) {
        {{{ makeSetValue('optval', 0, 'sock.error', 'i32') }}};
        {{{ makeSetValue('optlen', 0, 4, 'i32') }}};
        sock.error = null; // Clear the error (The SO_ERROR option obtains and then clears this field).
        return 0;
      }
    }
    return -{{{ cDefine('ENOPROTOOPT') }}}; // The option is unknown at the level indicated.
  },
  __syscall_sendmsg__deps: ['$getSocketFromFD', '$readSockaddr', '$DNS'],
  __syscall_sendmsg: function(fd, message, flags) {
    var sock = getSocketFromFD(fd);
    var iov = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iov, '*') }}};
    var num = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iovlen, 'i32') }}};
    // read the address and port to send to
    var addr, port;
    var name = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_name, '*') }}};
    var namelen = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_namelen, 'i32') }}};
    if (name) {
      var info = readSockaddr(name, namelen);
      if (info.errno) return -info.errno;
      port = info.port;
      addr = DNS.lookup_addr(info.addr) || info.addr;
    }
    // concatenate scatter-gather arrays into one message buffer
    var total = 0;
    for (var i = 0; i < num; i++) {
      total += {{{ makeGetValue('iov', '(' + C_STRUCTS.iovec.__size__ + ' * i) + ' + C_STRUCTS.iovec.iov_len, 'i32') }}};
    }
    var view = new Uint8Array(total);
    var offset = 0;
    for (var i = 0; i < num; i++) {
      var iovbase = {{{ makeGetValue('iov', '(' + C_STRUCTS.iovec.__size__ + ' * i) + ' + C_STRUCTS.iovec.iov_base, 'i8*') }}};
      var iovlen = {{{ makeGetValue('iov', '(' + C_STRUCTS.iovec.__size__ + ' * i) + ' + C_STRUCTS.iovec.iov_len, 'i32') }}};
      for (var j = 0; j < iovlen; j++) {  
        view[offset++] = {{{ makeGetValue('iovbase', 'j', 'i8') }}};
      }
    }
    // write the buffer
    return sock.sock_ops.sendmsg(sock, view, 0, total, addr, port);
  },
  __syscall_recvmsg__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_recvmsg: function(fd, message, flags) {
    var sock = getSocketFromFD(fd);
    var iov = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iov, 'i8*') }}};
    var num = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iovlen, 'i32') }}};
    // get the total amount of data we can read across all arrays
    var total = 0;
    for (var i = 0; i < num; i++) {
      total += {{{ makeGetValue('iov', '(' + C_STRUCTS.iovec.__size__ + ' * i) + ' + C_STRUCTS.iovec.iov_len, 'i32') }}};
    }
    // try to read total data
    var msg = sock.sock_ops.recvmsg(sock, total);
    if (!msg) return 0; // socket is closed

    // TODO honor flags:
    // MSG_OOB
    // Requests out-of-band data. The significance and semantics of out-of-band data are protocol-specific.
    // MSG_PEEK
    // Peeks at the incoming message.
    // MSG_WAITALL
    // Requests that the function block until the full amount of data requested can be returned. The function may return a smaller amount of data if a signal is caught, if the connection is terminated, if MSG_PEEK was specified, or if an error is pending for the socket.

    // write the source address out
    var name = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_name, '*') }}};
    if (name) {
      var errno = writeSockaddr(name, sock.family, DNS.lookup_name(msg.addr), msg.port);
#if ASSERTIONS
      assert(!errno);
#endif
    }
    // write the buffer out to the scatter-gather arrays
    var bytesRead = 0;
    var bytesRemaining = msg.buffer.byteLength;
    for (var i = 0; bytesRemaining > 0 && i < num; i++) {
      var iovbase = {{{ makeGetValue('iov', '(' + C_STRUCTS.iovec.__size__ + ' * i) + ' + C_STRUCTS.iovec.iov_base, 'i8*') }}};
      var iovlen = {{{ makeGetValue('iov', '(' + C_STRUCTS.iovec.__size__ + ' * i) + ' + C_STRUCTS.iovec.iov_len, 'i32') }}};
      if (!iovlen) {
        continue;
      }
      var length = Math.min(iovlen, bytesRemaining);
      var buf = msg.buffer.subarray(bytesRead, bytesRead + length);
      HEAPU8.set(buf, iovbase + bytesRead);
      bytesRead += length;
      bytesRemaining -= length;
    }

    // TODO set msghdr.msg_flags
    // MSG_EOR
    // End of record was received (if supported by the protocol).
    // MSG_OOB
    // Out-of-band data was received.
    // MSG_TRUNC
    // Normal data was truncated.
    // MSG_CTRUNC

    return bytesRead;
  },
}

for (var x in SocketsLibrary) {
  wrapSyscallFunction(x, SocketsLibrary, true);
}

mergeInto(LibraryManager.library, SocketsLibrary);
