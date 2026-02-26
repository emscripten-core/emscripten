/**
 * @license
 * Copyright 2026 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 *
 * Direct Sockets API backend for POSIX socket syscalls.
 * Uses TCPSocket, TCPServerSocket, and UDPSocket from the Direct Sockets API
 * (WICG Direct Sockets specification) to provide real TCP/UDP networking
 * in Isolated Web Apps without needing a proxy server.
 */

#if DIRECT_SOCKETS

var DirectSocketsLibrary = {

  $DIRECT_SOCKETS__deps: ['$readSockaddr', '$writeSockaddr', '$DNS', '$inetNtop4', '$inetNtop6'],
  $DIRECT_SOCKETS: {
    // fd -> socket state mapping
    // Each entry: {
    //   family: AF_INET | AF_INET6,
    //   type: SOCK_STREAM | SOCK_DGRAM,
    //   protocol: number,
    //   // Connection state:
    //   state: 'created' | 'bound' | 'listening' | 'connecting' | 'connected' | 'closed',
    //   // Deferred socket options (applied at connect/bind time):
    //   options: { noDelay, keepAliveDelay, sendBufferSize, receiveBufferSize },
    //   // Local/remote address info:
    //   localAddress: string | null,
    //   localPort: number | null,
    //   remoteAddress: string | null,
    //   remotePort: number | null,
    //   // Direct Sockets API objects:
    //   tcpSocket: TCPSocket | null,
    //   tcpServer: TCPServerSocket | null,
    //   udpSocket: UDPSocket | null,
    //   // Stream readers/writers:
    //   reader: ReadableStreamDefaultReader | null,
    //   writer: WritableStreamDefaultWriter | null,
    //   // For TCP server - reader that yields accepted TCPSocket objects:
    //   acceptReader: ReadableStreamDefaultReader | null,
    //   // Buffered data from reads (Direct Sockets gives us chunks, C wants exact sizes):
    //   readBuffer: Uint8Array | null,
    //   readBufferOffset: number,
    //   // Error state:
    //   error: number,
    // }
    sockets: {},
    nextFd: 100,  // Start high to avoid conflicts with stdio/FS fds

    allocateFd() {
      return DIRECT_SOCKETS.nextFd++;
    },

    getSocket(fd) {
      var sock = DIRECT_SOCKETS.sockets[fd];
      if (!sock) return null;
      return sock;
    },

    createSocketState(family, type, protocol) {
      var fd = DIRECT_SOCKETS.allocateFd();
      var sock = {
        fd: fd,
        family: family,
        type: type,
        protocol: protocol,
        state: 'created',
        options: {
          noDelay: false,
          keepAliveDelay: 0,
          sendBufferSize: 0,
          receiveBufferSize: 0,
        },
        localAddress: null,
        localPort: null,
        remoteAddress: null,
        remotePort: null,
        tcpSocket: null,
        tcpServer: null,
        udpSocket: null,
        reader: null,
        writer: null,
        acceptReader: null,
        readBuffer: null,
        readBufferOffset: 0,
        error: 0,
      };
      DIRECT_SOCKETS.sockets[fd] = sock;
      return sock;
    },

    // Parse a sockaddr struct from Wasm memory and return {addr, port} as strings.
    parseSockaddr(addrPtr, addrLen) {
      var info = readSockaddr(addrPtr, addrLen);
      if (info.errno) return null;
      // readSockaddr returns addr as a string like "1.2.3.4" and port as a number.
      // DNS.lookup_addr resolves emscripten fake IPs back to hostnames.
      var resolvedAddr = DNS.lookup_addr(info.addr) || info.addr;
      return { family: info.family, addr: resolvedAddr, port: info.port };
    },

    // Build TCPSocketOptions from deferred socket options.
    buildTCPOptions(sock) {
      var opts = {};
      if (sock.options.noDelay) opts.noDelay = true;
      if (sock.options.keepAliveDelay > 0) opts.keepAliveDelay = sock.options.keepAliveDelay;
      if (sock.options.sendBufferSize > 0) opts.sendBufferSize = sock.options.sendBufferSize;
      if (sock.options.receiveBufferSize > 0) opts.receiveBufferSize = sock.options.receiveBufferSize;
      if (sock.family === {{{ cDefs.AF_INET6 }}}) opts.dnsQueryType = 'ipv6';
      return opts;
    },

    buildUDPOptions(sock) {
      var opts = {};
      if (sock.options.sendBufferSize > 0) opts.sendBufferSize = sock.options.sendBufferSize;
      if (sock.options.receiveBufferSize > 0) opts.receiveBufferSize = sock.options.receiveBufferSize;
      if (sock.family === {{{ cDefs.AF_INET6 }}}) opts.dnsQueryType = 'ipv6';
      return opts;
    },

    // Read from Direct Sockets reader, filling internal buffer.
    // Returns a Uint8Array of up to `length` bytes, or null if closed.
    async readFromSocket(sock, length) {
      // First consume any buffered data from a previous over-read.
      if (sock.readBuffer && sock.readBufferOffset < sock.readBuffer.length) {
        var remaining = sock.readBuffer.length - sock.readBufferOffset;
        var toReturn = Math.min(remaining, length);
        var result = sock.readBuffer.slice(sock.readBufferOffset, sock.readBufferOffset + toReturn);
        sock.readBufferOffset += toReturn;
        if (sock.readBufferOffset >= sock.readBuffer.length) {
          sock.readBuffer = null;
          sock.readBufferOffset = 0;
        }
        return result;
      }

      if (!sock.reader) return null;

      try {
        var { value, done } = await sock.reader.read();
        if (done || !value) return null;

        // value is a Uint8Array from the Direct Sockets readable stream.
        if (value.length <= length) {
          return value;
        }
        // Got more data than requested - buffer the remainder.
        var result = value.slice(0, length);
        sock.readBuffer = value;
        sock.readBufferOffset = length;
        return result;
      } catch (e) {
#if SOCKET_DEBUG
        dbg(`direct_sockets: read error: ${e}`);
#endif
        sock.error = {{{ cDefs.EIO }}};
        return null;
      }
    },

    // Write to Direct Sockets writer.
    async writeToSocket(sock, data) {
      if (!sock.writer) return -{{{ cDefs.ENOTCONN }}};
      try {
        await sock.writer.write(data);
        return data.length;
      } catch (e) {
#if SOCKET_DEBUG
        dbg(`direct_sockets: write error: ${e}`);
#endif
        sock.error = {{{ cDefs.EIO }}};
        return -{{{ cDefs.EPIPE }}};
      }
    },
  },

  // ---------------------------------------------------------------------------
  // Syscall implementations
  // ---------------------------------------------------------------------------

  __syscall_socket__deps: ['$DIRECT_SOCKETS'],
  __syscall_socket: (domain, type, protocol) => {
    // Strip flags that don't apply in single-process context
    type &= ~({{{ cDefs.SOCK_CLOEXEC | cDefs.SOCK_NONBLOCK }}});

    // Validate family
    if (domain !== {{{ cDefs.AF_INET }}} && domain !== {{{ cDefs.AF_INET6 }}}) {
      return -{{{ cDefs.EAFNOSUPPORT }}};
    }

    // Validate type
    if (type !== {{{ cDefs.SOCK_STREAM }}} && type !== {{{ cDefs.SOCK_DGRAM }}}) {
      return -{{{ cDefs.EINVAL }}};
    }

    // Validate protocol vs type
    if (type === {{{ cDefs.SOCK_STREAM }}} && protocol !== 0 && protocol !== {{{ cDefs.IPPROTO_TCP }}}) {
      return -{{{ cDefs.EPROTONOSUPPORT }}};
    }
    if (type === {{{ cDefs.SOCK_DGRAM }}} && protocol !== 0 && protocol !== {{{ cDefs.IPPROTO_UDP }}}) {
      return -{{{ cDefs.EPROTONOSUPPORT }}};
    }

    var sock = DIRECT_SOCKETS.createSocketState(domain, type, protocol);

#if SOCKET_DEBUG
    dbg(`direct_sockets: socket(${domain}, ${type}, ${protocol}) -> fd ${sock.fd}`);
#endif

    return sock.fd;
  },

  __syscall_connect__deps: ['$DIRECT_SOCKETS'],
  __syscall_connect__async: true,
  __syscall_connect: async (fd, addr, addrlen) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};
    if (sock.state === 'connected' || sock.state === 'connecting') return -{{{ cDefs.EISCONN }}};

    var dest = DIRECT_SOCKETS.parseSockaddr(addr, addrlen);
    if (!dest) return -{{{ cDefs.EINVAL }}};

#if SOCKET_DEBUG
    dbg(`direct_sockets: connect(fd=${fd}, addr=${dest.addr}, port=${dest.port})`);
#endif

    sock.state = 'connecting';

    try {
      if (sock.type === {{{ cDefs.SOCK_STREAM }}}) {
        // TCP connect
        var opts = DIRECT_SOCKETS.buildTCPOptions(sock);
        var tcpSocket = new TCPSocket(dest.addr, dest.port, opts);
        var openInfo = await tcpSocket.opened;

        sock.tcpSocket = tcpSocket;
        sock.reader = openInfo.readable.getReader();
        sock.writer = openInfo.writable.getWriter();
        sock.remoteAddress = openInfo.remoteAddress || dest.addr;
        sock.remotePort = openInfo.remotePort || dest.port;
        sock.localAddress = openInfo.localAddress || '0.0.0.0';
        sock.localPort = openInfo.localPort || 0;
        sock.state = 'connected';

      } else {
        // UDP "connect" - creates a connected-mode UDPSocket
        var opts = DIRECT_SOCKETS.buildUDPOptions(sock);
        opts.remoteAddress = dest.addr;
        opts.remotePort = dest.port;
        var udpSocket = new UDPSocket(opts);
        var openInfo = await udpSocket.opened;

        sock.udpSocket = udpSocket;
        sock.reader = openInfo.readable.getReader();
        sock.writer = openInfo.writable.getWriter();
        sock.remoteAddress = openInfo.remoteAddress || dest.addr;
        sock.remotePort = openInfo.remotePort || dest.port;
        sock.localAddress = openInfo.localAddress || '0.0.0.0';
        sock.localPort = openInfo.localPort || 0;
        sock.state = 'connected';
      }
    } catch (e) {
#if SOCKET_DEBUG
      dbg(`direct_sockets: connect error: ${e}`);
#endif
      sock.state = 'created';
      if (e.name === 'NotAllowedError') return -{{{ cDefs.EACCES }}};
      return -{{{ cDefs.ECONNREFUSED }}};
    }

    return 0;
  },

  __syscall_bind__deps: ['$DIRECT_SOCKETS'],
  __syscall_bind__async: true,
  __syscall_bind: async (fd, addr, addrlen) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};
    if (sock.state !== 'created') return -{{{ cDefs.EINVAL }}};

    var bindAddr = DIRECT_SOCKETS.parseSockaddr(addr, addrlen);
    if (!bindAddr) return -{{{ cDefs.EINVAL }}};

#if SOCKET_DEBUG
    dbg(`direct_sockets: bind(fd=${fd}, addr=${bindAddr.addr}, port=${bindAddr.port})`);
#endif

    // Store the requested bind address. Actual socket creation is deferred
    // to listen() for TCP servers, or done here for UDP bound sockets.
    sock.localAddress = bindAddr.addr;
    sock.localPort = bindAddr.port;

    if (sock.type === {{{ cDefs.SOCK_DGRAM }}}) {
      // UDP: create bound-mode UDPSocket immediately
      try {
        var opts = DIRECT_SOCKETS.buildUDPOptions(sock);
        opts.localAddress = bindAddr.addr;
        opts.localPort = bindAddr.port;
        var udpSocket = new UDPSocket(opts);
        var openInfo = await udpSocket.opened;

        sock.udpSocket = udpSocket;
        sock.reader = openInfo.readable.getReader();
        sock.writer = openInfo.writable.getWriter();
        sock.localAddress = openInfo.localAddress || bindAddr.addr;
        sock.localPort = openInfo.localPort || bindAddr.port;
        sock.state = 'bound';
      } catch (e) {
#if SOCKET_DEBUG
        dbg(`direct_sockets: bind (UDP) error: ${e}`);
#endif
        if (e.name === 'NotAllowedError') return -{{{ cDefs.EACCES }}};
        return -{{{ cDefs.EADDRINUSE }}};
      }
    } else {
      // TCP: just mark as bound, actual server creation happens in listen()
      sock.state = 'bound';
    }

    return 0;
  },

  __syscall_listen__deps: ['$DIRECT_SOCKETS'],
  __syscall_listen__async: true,
  __syscall_listen: async (fd, backlog) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};
    if (sock.type !== {{{ cDefs.SOCK_STREAM }}}) return -{{{ cDefs.EOPNOTSUPP }}};
    if (sock.state !== 'bound') return -{{{ cDefs.EINVAL }}};

#if SOCKET_DEBUG
    dbg(`direct_sockets: listen(fd=${fd}, backlog=${backlog})`);
#endif

    try {
      var opts = {};
      if (sock.localPort) opts.localPort = sock.localPort;
      if (backlog > 0) opts.backlog = backlog;

      var tcpServer = new TCPServerSocket(sock.localAddress || '0.0.0.0', opts);
      var openInfo = await tcpServer.opened;

      sock.tcpServer = tcpServer;
      sock.acceptReader = openInfo.readable.getReader();
      sock.localAddress = openInfo.localAddress || sock.localAddress;
      sock.localPort = openInfo.localPort || sock.localPort;
      sock.state = 'listening';
    } catch (e) {
#if SOCKET_DEBUG
      dbg(`direct_sockets: listen error: ${e}`);
#endif
      if (e.name === 'NotAllowedError') return -{{{ cDefs.EACCES }}};
      return -{{{ cDefs.EADDRINUSE }}};
    }

    return 0;
  },

  __syscall_accept4__deps: ['$DIRECT_SOCKETS', '$writeSockaddr', '$DNS'],
  __syscall_accept4__async: true,
  __syscall_accept4: async (fd, addr, addrlen, flags) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};
    if (sock.state !== 'listening') return -{{{ cDefs.EINVAL }}};

#if SOCKET_DEBUG
    dbg(`direct_sockets: accept4(fd=${fd})`);
#endif

    try {
      var { value: acceptedTcpSocket, done } = await sock.acceptReader.read();
      if (done || !acceptedTcpSocket) return -{{{ cDefs.ECONNABORTED }}};

      // acceptedTcpSocket is a TCPSocket instance from the server's readable stream
      var openInfo = await acceptedTcpSocket.opened;

      // Create a new socket state for the accepted connection
      var newSock = DIRECT_SOCKETS.createSocketState(sock.family, sock.type, sock.protocol);
      newSock.tcpSocket = acceptedTcpSocket;
      newSock.reader = openInfo.readable.getReader();
      newSock.writer = openInfo.writable.getWriter();
      newSock.remoteAddress = openInfo.remoteAddress || '0.0.0.0';
      newSock.remotePort = openInfo.remotePort || 0;
      newSock.localAddress = openInfo.localAddress || sock.localAddress;
      newSock.localPort = openInfo.localPort || sock.localPort;
      newSock.state = 'connected';

      // Write peer address back if requested
      if (addr) {
        var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(newSock.remoteAddress), newSock.remotePort, addrlen);
        if (errno) {
#if SOCKET_DEBUG
          dbg(`direct_sockets: accept4 writeSockaddr error: ${errno}`);
#endif
        }
      }

#if SOCKET_DEBUG
      dbg(`direct_sockets: accept4 -> new fd ${newSock.fd}, remote=${newSock.remoteAddress}:${newSock.remotePort}`);
#endif

      return newSock.fd;
    } catch (e) {
#if SOCKET_DEBUG
      dbg(`direct_sockets: accept4 error: ${e}`);
#endif
      return -{{{ cDefs.ECONNABORTED }}};
    }
  },

  __syscall_sendto__deps: ['$DIRECT_SOCKETS'],
  __syscall_sendto__async: true,
  __syscall_sendto: async (fd, message, length, flags, addr, addr_len) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};

    // Copy data from Wasm memory
    var data = new Uint8Array(HEAPU8.buffer, message, length).slice();

    if (sock.type === {{{ cDefs.SOCK_STREAM }}}) {
      // TCP send - addr is ignored
      if (sock.state !== 'connected') return -{{{ cDefs.ENOTCONN }}};

#if SOCKET_DEBUG
      dbg(`direct_sockets: send(fd=${fd}, length=${length})`);
#endif

      return DIRECT_SOCKETS.writeToSocket(sock, data);

    } else {
      // UDP sendto
      if (addr && addr_len > 0) {
        // sendto with explicit destination (requires bound-mode UDP)
        var dest = DIRECT_SOCKETS.parseSockaddr(addr, addr_len);
        if (!dest) return -{{{ cDefs.EINVAL }}};

#if SOCKET_DEBUG
        dbg(`direct_sockets: sendto(fd=${fd}, length=${length}, dest=${dest.addr}:${dest.port})`);
#endif

        if (sock.state === 'bound' && sock.writer) {
          // Bound-mode UDP: send with per-message addressing
          try {
            await sock.writer.write({
              data: data,
              remoteAddress: dest.addr,
              remotePort: dest.port,
            });
            return length;
          } catch (e) {
#if SOCKET_DEBUG
            dbg(`direct_sockets: sendto error: ${e}`);
#endif
            return -{{{ cDefs.ENETUNREACH }}};
          }
        } else {
          return -{{{ cDefs.EDESTADDRREQ }}};
        }
      } else {
        // send on connected UDP socket
        if (sock.state !== 'connected') return -{{{ cDefs.EDESTADDRREQ }}};

#if SOCKET_DEBUG
        dbg(`direct_sockets: send(fd=${fd}, length=${length}) [connected UDP]`);
#endif

        try {
          await sock.writer.write({ data: data });
          return length;
        } catch (e) {
#if SOCKET_DEBUG
          dbg(`direct_sockets: send error: ${e}`);
#endif
          return -{{{ cDefs.ENETUNREACH }}};
        }
      }
    }
  },

  __syscall_recvfrom__deps: ['$DIRECT_SOCKETS', '$writeSockaddr', '$DNS'],
  __syscall_recvfrom__async: true,
  __syscall_recvfrom: async (fd, buf, len, flags, addr, addrlen) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};

    if (sock.type === {{{ cDefs.SOCK_STREAM }}}) {
      // TCP recv
      if (sock.state !== 'connected') return -{{{ cDefs.ENOTCONN }}};

#if SOCKET_DEBUG
      dbg(`direct_sockets: recv(fd=${fd}, len=${len})`);
#endif

      var data = await DIRECT_SOCKETS.readFromSocket(sock, len);
      if (!data) return 0;  // Connection closed (EOF)

      HEAPU8.set(data, buf);

      if (addr) {
        var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(sock.remoteAddress), sock.remotePort, addrlen);
      }

      return data.length;

    } else {
      // UDP recvfrom
      if (sock.state !== 'connected' && sock.state !== 'bound') return -{{{ cDefs.ENOTCONN }}};

#if SOCKET_DEBUG
      dbg(`direct_sockets: recvfrom(fd=${fd}, len=${len})`);
#endif

      if (!sock.reader) return -{{{ cDefs.ENOTCONN }}};

      try {
        var { value: message, done } = await sock.reader.read();
        if (done || !message) return 0;

        // message is a UDPMessage: { data: Uint8Array, remoteAddress?, remotePort? }
        var msgData = message.data;
        var copyLen = Math.min(msgData.length, len);
        HEAPU8.set(msgData.subarray(0, copyLen), buf);

        if (addr && message.remoteAddress) {
          var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(message.remoteAddress), message.remotePort, addrlen);
        }

        return copyLen;
      } catch (e) {
#if SOCKET_DEBUG
        dbg(`direct_sockets: recvfrom error: ${e}`);
#endif
        return -{{{ cDefs.EIO }}};
      }
    }
  },

  __syscall_shutdown__deps: ['$DIRECT_SOCKETS'],
  __syscall_shutdown__async: true,
  __syscall_shutdown: async (fd, how) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};

#if SOCKET_DEBUG
    dbg(`direct_sockets: shutdown(fd=${fd}, how=${how})`);
#endif

    try {
      // SHUT_RD = 0, SHUT_WR = 1, SHUT_RDWR = 2
      if (how === 0 || how === 2) {
        if (sock.reader) {
          await sock.reader.cancel();
          sock.reader = null;
        }
      }
      if (how === 1 || how === 2) {
        if (sock.writer) {
          await sock.writer.close();
          sock.writer = null;
        }
      }
      if (how === 2) {
        await DIRECT_SOCKETS._closeSocket(sock);
      }
    } catch (e) {
#if SOCKET_DEBUG
      dbg(`direct_sockets: shutdown error: ${e}`);
#endif
    }

    return 0;
  },

  __syscall_getsockname__deps: ['$DIRECT_SOCKETS', '$writeSockaddr', '$DNS'],
  __syscall_getsockname: (fd, addr, addrlen) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};

    var localAddr = sock.localAddress || '0.0.0.0';
    var localPort = sock.localPort || 0;

    var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(localAddr), localPort, addrlen);
    return errno ? -{{{ cDefs.EINVAL }}} : 0;
  },

  __syscall_getpeername__deps: ['$DIRECT_SOCKETS', '$writeSockaddr', '$DNS'],
  __syscall_getpeername: (fd, addr, addrlen) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};

    if (!sock.remoteAddress) return -{{{ cDefs.ENOTCONN }}};

    var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(sock.remoteAddress), sock.remotePort, addrlen);
    return errno ? -{{{ cDefs.EINVAL }}} : 0;
  },

  __syscall_setsockopt__deps: ['$DIRECT_SOCKETS'],
  __syscall_setsockopt: (fd, level, optname, optval, optlen) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};

#if SOCKET_DEBUG
    dbg(`direct_sockets: setsockopt(fd=${fd}, level=${level}, optname=${optname})`);
#endif

    // Direct Sockets only supports a few options, and they must be set at
    // construction time. We defer them and apply when connect/bind is called.
    // SOL_SOCKET = 1, musl values for socket options:
    // SO_REUSEADDR=2, SO_TYPE=3, SO_ERROR=4, SO_SNDBUF=7, SO_RCVBUF=8,
    // SO_KEEPALIVE=9, SO_REUSEPORT=15
    if (level === 1 /*SOL_SOCKET*/) {
      switch (optname) {
        case 2: // SO_REUSEADDR
        case 15: // SO_REUSEPORT
          // Silently accept - no equivalent, but harmless
          return 0;
        case 7: // SO_SNDBUF
          sock.options.sendBufferSize = {{{ makeGetValue('optval', 0, 'i32') }}};
          return 0;
        case 8: // SO_RCVBUF
          sock.options.receiveBufferSize = {{{ makeGetValue('optval', 0, 'i32') }}};
          return 0;
        case 9: // SO_KEEPALIVE
          // Will be used as keepAliveDelay if enabled - use a default of 60s
          var enabled = {{{ makeGetValue('optval', 0, 'i32') }}};
          if (enabled && sock.options.keepAliveDelay === 0) {
            sock.options.keepAliveDelay = 60000;  // 60 seconds default
          } else if (!enabled) {
            sock.options.keepAliveDelay = 0;
          }
          return 0;
        default:
          // Silently ignore unknown SOL_SOCKET options rather than failing
#if SOCKET_DEBUG
          dbg(`direct_sockets: setsockopt ignoring SOL_SOCKET option ${optname}`);
#endif
          return 0;
      }
    } else if (level === 6 /*IPPROTO_TCP*/) {
      switch (optname) {
        case 1:  // TCP_NODELAY (musl value = 1)
          sock.options.noDelay = !!{{{ makeGetValue('optval', 0, 'i32') }}};
          return 0;
        case 4:  // TCP_KEEPIDLE (musl value = 4)
        case 5:  // TCP_KEEPINTVL (musl value = 5)
          // Map to keepAliveDelay (in milliseconds)
          sock.options.keepAliveDelay = {{{ makeGetValue('optval', 0, 'i32') }}} * 1000;
          return 0;
        default:
#if SOCKET_DEBUG
          dbg(`direct_sockets: setsockopt ignoring IPPROTO_TCP option ${optname}`);
#endif
          return 0;
      }
    }

    // Silently accept unknown levels
    return 0;
  },

  __syscall_getsockopt__deps: ['$DIRECT_SOCKETS'],
  __syscall_getsockopt: (fd, level, optname, optval, optlen) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};

    if (level === 1 /*SOL_SOCKET*/) {
      if (optname === {{{ cDefs.SO_ERROR }}}) {
        {{{ makeSetValue('optval', 0, 'sock.error', 'i32') }}};
        {{{ makeSetValue('optlen', 0, 4, 'i32') }}};
        sock.error = 0;
        return 0;
      }
      if (optname === 3 /*SO_TYPE*/) {
        {{{ makeSetValue('optval', 0, 'sock.type', 'i32') }}};
        {{{ makeSetValue('optlen', 0, 4, 'i32') }}};
        return 0;
      }
    }

    return -{{{ cDefs.ENOPROTOOPT }}};
  },

  // sendmsg/recvmsg: minimal implementations that delegate to sendto/recvfrom
  __syscall_sendmsg__deps: ['$DIRECT_SOCKETS', '__syscall_sendto'],
  __syscall_sendmsg__async: true,
  __syscall_sendmsg: async (fd, message, flags) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};

    var iov = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iov, '*') }}};
    var num = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iovlen, 'i32') }}};

    var name = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_name, '*') }}};
    var namelen = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_namelen, 'i32') }}};

    // Gather all iov buffers into one
    var total = 0;
    for (var i = 0; i < num; i++) {
      total += {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_len}`, 'i32') }}};
    }
    var view = new Uint8Array(total);
    var offset = 0;
    for (var i = 0; i < num; i++) {
      var iovbase = {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_base}`, '*') }}};
      var iovlen = {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_len}`, 'i32') }}};
      view.set(HEAPU8.subarray(iovbase, iovbase + iovlen), offset);
      offset += iovlen;
    }

    // Write through Direct Sockets
    if (sock.type === {{{ cDefs.SOCK_STREAM }}}) {
      if (sock.state !== 'connected') return -{{{ cDefs.ENOTCONN }}};
      return DIRECT_SOCKETS.writeToSocket(sock, view);
    } else {
      if (name && namelen > 0) {
        var dest = DIRECT_SOCKETS.parseSockaddr(name, namelen);
        if (!dest) return -{{{ cDefs.EINVAL }}};
        if (sock.state === 'bound' && sock.writer) {
          try {
            await sock.writer.write({ data: view, remoteAddress: dest.addr, remotePort: dest.port });
            return total;
          } catch (e) {
            return -{{{ cDefs.ENETUNREACH }}};
          }
        }
        return -{{{ cDefs.EDESTADDRREQ }}};
      }
      if (sock.state !== 'connected') return -{{{ cDefs.EDESTADDRREQ }}};
      try {
        await sock.writer.write({ data: view });
        return total;
      } catch (e) {
        return -{{{ cDefs.ENETUNREACH }}};
      }
    }
  },

  __syscall_recvmsg__deps: ['$DIRECT_SOCKETS', '$writeSockaddr', '$DNS'],
  __syscall_recvmsg__async: true,
  __syscall_recvmsg: async (fd, message, flags) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};

    var iov = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iov, '*') }}};
    var num = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iovlen, 'i32') }}};

    // Calculate total recv capacity
    var total = 0;
    for (var i = 0; i < num; i++) {
      total += {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_len}`, 'i32') }}};
    }

    if (sock.type === {{{ cDefs.SOCK_STREAM }}}) {
      if (sock.state !== 'connected') return -{{{ cDefs.ENOTCONN }}};
      var data = await DIRECT_SOCKETS.readFromSocket(sock, total);
      if (!data) return 0;

      // Scatter into iovecs
      var bytesRead = 0;
      var remaining = data.length;
      for (var i = 0; remaining > 0 && i < num; i++) {
        var iovbase = {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_base}`, '*') }}};
        var iovlen = {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_len}`, 'i32') }}};
        var copyLen = Math.min(iovlen, remaining);
        HEAPU8.set(data.subarray(bytesRead, bytesRead + copyLen), iovbase);
        bytesRead += copyLen;
        remaining -= copyLen;
      }

      var msgName = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_name, '*') }}};
      if (msgName && sock.remoteAddress) {
        writeSockaddr(msgName, sock.family, DNS.lookup_name(sock.remoteAddress), sock.remotePort);
      }

      return bytesRead;
    } else {
      // UDP
      if (!sock.reader) return -{{{ cDefs.ENOTCONN }}};
      try {
        var { value: msg, done } = await sock.reader.read();
        if (done || !msg) return 0;

        var msgData = msg.data;
        var bytesRead = 0;
        var remaining = msgData.length;
        for (var i = 0; remaining > 0 && i < num; i++) {
          var iovbase = {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_base}`, '*') }}};
          var iovlen = {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_len}`, 'i32') }}};
          var copyLen = Math.min(iovlen, remaining);
          HEAPU8.set(msgData.subarray(bytesRead, bytesRead + copyLen), iovbase);
          bytesRead += copyLen;
          remaining -= copyLen;
        }

        var msgName = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_name, '*') }}};
        if (msgName && msg.remoteAddress) {
          writeSockaddr(msgName, sock.family, DNS.lookup_name(msg.remoteAddress), msg.remotePort);
        }

        return bytesRead;
      } catch (e) {
        return -{{{ cDefs.EIO }}};
      }
    }
  },

  // Internal helper for closing - not a syscall but used by shutdown and close
  $DIRECT_SOCKETS__postset: `
    DIRECT_SOCKETS._closeSocket = async function(sock) {
      try {
        if (sock.reader) { try { sock.reader.releaseLock(); } catch(e) {} sock.reader = null; }
        if (sock.writer) { try { sock.writer.releaseLock(); } catch(e) {} sock.writer = null; }
        if (sock.acceptReader) { try { sock.acceptReader.releaseLock(); } catch(e) {} sock.acceptReader = null; }
        if (sock.tcpSocket) { try { await sock.tcpSocket.close(); } catch(e) {} sock.tcpSocket = null; }
        if (sock.tcpServer) { try { await sock.tcpServer.close(); } catch(e) {} sock.tcpServer = null; }
        if (sock.udpSocket) { try { await sock.udpSocket.close(); } catch(e) {} sock.udpSocket = null; }
      } catch (e) {}
      sock.state = 'closed';
      sock.readBuffer = null;
      sock.readBufferOffset = 0;
    };
  `,
};

for (var x in DirectSocketsLibrary) {
  if (x.startsWith('__syscall_')) {
    wrapSyscallFunction(x, DirectSocketsLibrary, false);
  }
}

addToLibrary(DirectSocketsLibrary);

#endif // DIRECT_SOCKETS
