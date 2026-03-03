/**
 * @license
 * Copyright 2026 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 *
 * Direct Sockets API backend for POSIX socket syscalls.
 * Uses TCPSocket, TCPServerSocket, and UDPSocket from the Direct Sockets API
 * (https://wicg.github.io/direct-sockets/) to provide real TCP/UDP networking
 * in Isolated Web Apps without needing a proxy server.
 */

#if DIRECT_SOCKETS

var DirectSocketsLibrary = {

  $DIRECT_SOCKETS__deps: ['$readSockaddr', '$writeSockaddr', '$DNS', '$inetNtop4', '$inetNtop6', '$FS'],
  $DIRECT_SOCKETS: {
    // fd -> socket state mapping
    sockets: {},

    // Monotonic counter for unique socket/pipe node names
    nextId: 0,

    // DNS cache: hostname -> {addresses: [...], expires: timestamp}
    dnsCache: {},

    // FS mount point for socket nodes (initialized lazily)
    root: null,

    // Custom stream_ops so that write(fd)/read(fd) route through Direct Sockets.
    // This is the same pattern SOCKFS uses - register sockets as FS streams.
    // IMPORTANT: These must be SYNCHRONOUS because they're called from FS.write/
    // FS.read (JS-to-JS), not directly from WASM, so JSPI can't suspend here.
    stream_ops: {
      read(stream, buffer, offset, length, position) {
        var sock = stream.node.sock;
        if (!sock || (sock.state !== 'connected' && sock.state !== 'bound')) return 0;

        // Synchronous: consume from recvQueue (filled by background reader)
        if (sock.recvQueue.length > 0) {
          var entry = sock.recvQueue[0];
          // UDP datagrams are {data, remoteAddress, remotePort}; TCP chunks are Uint8Array
          var chunk = entry.data ? entry.data : entry;
          var toRead = Math.min(chunk.length, length);
          for (var i = 0; i < toRead; i++) {
            buffer[offset + i] = chunk[i];
          }
          if (toRead >= chunk.length) {
            sock.recvQueue.shift();
          } else {
            // Partial read: for TCP, keep remainder; for UDP, discard (datagram semantics)
            if (entry.data) {
              sock.recvQueue.shift(); // UDP: truncate
            } else {
              sock.recvQueue[0] = chunk.slice(toRead);
            }
          }
          return toRead;
        }

        // Check for read errors before reporting EOF
        if (sock.error) throw new FS.ErrnoError(sock.error);

        // EOF
        if (sock._bgReaderDone) return 0;

        // No data available - signal EAGAIN.
        // OpenSSL handles this: SSL_connect returns SSL_ERROR_WANT_READ,
        // Tor's event loop waits for readability, retries when data arrives.
        throw new FS.ErrnoError({{{ cDefs.EAGAIN }}});
      },
      write(stream, buffer, offset, length, position) {
        var sock = stream.node.sock;
        if (!sock || !sock.writer) {
          throw new FS.ErrnoError({{{ cDefs.ENOTCONN }}});
        }
        // Copy to non-shared buffer (Direct Sockets rejects SharedArrayBuffer views)
        var data = new Uint8Array(length);
        for (var i = 0; i < length; i++) {
          data[i] = buffer[offset + i];
        }
        // Fire-and-forget: queue the write, return byte count immediately.
        // The browser/OS handles TCP buffering and backpressure.
        sock.writer.write(data).catch(function(e) {
          // write error - socket will be cleaned up by close
          sock.error = {{{ cDefs.EIO }}};
        });
        return length;
      },
      poll(stream) {
        var sock = stream.node.sock;
        if (!sock) return 0;
        var mask = 0;
        if (sock.recvQueue.length > 0 || sock._bgReaderDone) mask |= {{{ cDefs.POLLIN | 64 /*POLLRDNORM*/ }}};
        if (sock.writer && sock.state === 'connected') mask |= {{{ cDefs.POLLOUT | 256 /*POLLWRNORM*/ }}};
        if (sock._bgReaderDone && sock.recvQueue.length === 0) mask |= {{{ cDefs.POLLHUP }}};
        if (sock.error) mask |= {{{ cDefs.POLLERR }}};
        return mask;
      },
      close(stream) {
        var sock = stream.node.sock;
        if (sock) {
          delete DIRECT_SOCKETS.sockets[sock.fd];
          sock.stream = null; // prevent _closeSocket from double-closing FS stream
          DIRECT_SOCKETS._closeSocket(sock);
        }
      },
    },

    ensureRoot() {
      if (!DIRECT_SOCKETS.root) {
        DIRECT_SOCKETS.root = FS.createNode(null, '/', {{{ cDefs.S_IFDIR }}} | 511 /* 0777 */, 0);
      }
    },

    getSocket(fd) {
      var sock = DIRECT_SOCKETS.sockets[fd];
      if (!sock) return null;
      return sock;
    },

    createSocketState(family, type, protocol) {
#if ASSERTIONS
      if (typeof globalThis.TCPSocket === 'undefined' &&
          typeof globalThis.UDPSocket === 'undefined') {
        abort('Direct Sockets API is not available. DIRECT_SOCKETS requires a Chrome Isolated Web App (IWA) context. See https://wicg.github.io/direct-sockets/');
      }
#endif
      DIRECT_SOCKETS.ensureRoot();

      // Create an FS node + stream so that write()/read() on this fd
      // routes through our custom stream_ops (same pattern as SOCKFS).
      var name = 'socket[' + (DIRECT_SOCKETS.nextId++) + ']';
      var node = FS.createNode(DIRECT_SOCKETS.root, name, {{{ cDefs.S_IFSOCK }}}, 0);

      var sock = {
        fd: 0, // will be set after FS.createStream
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
        multicastController: null,
        joinedMulticastGroups: [],
        error: 0,
        // Background reader / recv queue fields
        recvQueue: [],
        _bgReaderRunning: false,
        _bgReaderDone: false,
        _waiters: [],
        // Non-blocking mode
        nonBlocking: false,
        flags: 0,
        // FS integration
        stream: null,
      };

      // Attach socket to node (so stream_ops can find it)
      node.sock = sock;

      // Register in FS - this assigns a real fd
      var stream = FS.createStream({
        path: name,
        node: node,
        flags: {{{ cDefs.O_RDWR }}},
        seekable: false,
        stream_ops: DIRECT_SOCKETS.stream_ops,
      });
      sock.stream = stream;
      sock.fd = stream.fd;

      DIRECT_SOCKETS.sockets[sock.fd] = sock;
      return sock;
    },

    // Notify all registered waiters on a socket
    _notifyWaiters(sock) {
      if (sock._waiters && sock._waiters.length > 0) {
        var waiters = sock._waiters.slice();
        sock._waiters = [];
        for (var i = 0; i < waiters.length; i++) {
          waiters[i]();
        }
      }
    },

    // Register a waiter callback on a socket (supports multiple concurrent waiters)
    _addWaiter(sock, cb) {
      if (!sock._waiters) sock._waiters = [];
      sock._waiters.push(cb);
    },

    // Remove a specific waiter callback
    _removeWaiter(sock, cb) {
      if (!sock._waiters) return;
      var idx = sock._waiters.indexOf(cb);
      if (idx >= 0) sock._waiters.splice(idx, 1);
    },

    // Start a background reader loop that pumps data into recvQueue
    startBackgroundReader(sock) {
      if (sock._bgReaderRunning) return;
      sock._bgReaderRunning = true;

      (async () => {
        try {
          while (sock.reader && (sock.state === 'connected' || sock.state === 'bound')) {
            var { value, done } = await sock.reader.read();
            if (done || !value) {
              sock._bgReaderDone = true; break;
            }
            sock.recvQueue.push(value);
            DIRECT_SOCKETS._notifyWaiters(sock);
          }
        } catch (e) {
          // bgReader error - mark done
          sock.error = {{{ cDefs.EIO }}};
          sock._bgReaderDone = true;
        }
        sock._bgReaderRunning = false;
        // Notify on close/error too
        DIRECT_SOCKETS._notifyWaiters(sock);
      })();
    },

    // Parse a sockaddr struct from Wasm memory.
    // Returns {family, addr, port} on success, or {errno} on failure.
    parseSockaddr(addrPtr, addrLen) {
      var info = readSockaddr(addrPtr, addrLen);
      if (info.errno) return { errno: info.errno };
      // readSockaddr returns addr as a string like "1.2.3.4" and port as a number.

      // First check our DoH reverse cache - if this IP was resolved by us,
      // map it back to the hostname so TCPSocket can do its own resolution
      var addr = info.addr;
      var reverseHostname = DIRECT_SOCKETS.dnsCache['_reverse_' + addr];
      if (reverseHostname) {
        return { family: info.family, addr: reverseHostname, port: info.port };
      }

      // Fall back to emscripten's DNS reverse lookup (for fake 172.29.x.x IPs)
      var resolvedAddr = DNS.lookup_addr(addr) || addr;
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

    attachMulticastController(sock, openInfo) {
      var controller = (openInfo && openInfo.multicast) || (sock.udpSocket && sock.udpSocket.multicast) || null;
      sock.multicastController = controller;
      if (controller && Array.isArray(controller.joinedGroups)) {
        sock.joinedMulticastGroups = controller.joinedGroups.slice();
      }
    },

    parseIpMreq(ptr, optlen) {
      // struct ip_mreq { struct in_addr imr_multiaddr; struct in_addr imr_interface; }
      if (!ptr || optlen < 8) return null;
      var groupAddr = inetNtop4(ptr);
      var ifaceAddr = inetNtop4(ptr + 4);
      return { groupAddress: groupAddr, localAddress: ifaceAddr !== '0.0.0.0' ? ifaceAddr : undefined };
    },

    parseIpv6Mreq(ptr, optlen) {
      // struct ipv6_mreq { struct in6_addr ipv6mr_multiaddr; unsigned int ipv6mr_interface; }
      if (!ptr || optlen < 20) return null;
      var groupAddress = inetNtop6(ptr);
      var interfaceIndex = HEAPU32[(ptr + 16) >> 2];
      return { groupAddress: groupAddress, interfaceIndex: interfaceIndex || undefined };
    },

    async joinMulticastGroup(sock, membership) {
      if (!sock.multicastController || !membership || !membership.groupAddress) {
        return -{{{ cDefs.ENOPROTOOPT }}};
      }
      try {
        var result = await sock.multicastController.joinGroup(membership.groupAddress, membership);
        sock.joinedMulticastGroups = Array.isArray(sock.multicastController.joinedGroups)
          ? sock.multicastController.joinedGroups.slice()
          : sock.joinedMulticastGroups;
        return result === undefined ? 0 : 0;
      } catch (e) {
        if (e.name === 'NotAllowedError') return -{{{ cDefs.EACCES }}};
        return -{{{ cDefs.EINVAL }}};
      }
    },

    async leaveMulticastGroup(sock, membership) {
      if (!sock.multicastController || !membership || !membership.groupAddress) {
        return -{{{ cDefs.ENOPROTOOPT }}};
      }
      try {
        var result = await sock.multicastController.leaveGroup(membership.groupAddress, membership);
        sock.joinedMulticastGroups = Array.isArray(sock.multicastController.joinedGroups)
          ? sock.multicastController.joinedGroups.slice()
          : sock.joinedMulticastGroups.filter((g) => g.groupAddress !== membership.groupAddress);
        return result === undefined ? 0 : 0;
      } catch (e) {
        if (e.name === 'NotAllowedError') return -{{{ cDefs.EACCES }}};
        return -{{{ cDefs.EINVAL }}};
      }
    },

    // Read from Direct Sockets, consuming from recvQueue first (sync path),
    // then falling back to blocking await if queue is empty.
    // Returns a Uint8Array of up to `length` bytes, or null if closed.
    async readFromSocket(sock, length) {
      // First consume any buffered data from a previous over-read (legacy readBuffer).
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

      // Consume from recvQueue (filled by background reader)
      if (sock.recvQueue.length > 0) {
        var chunk = sock.recvQueue[0];
        if (chunk.length <= length) {
          sock.recvQueue.shift();
          return chunk;
        }
        // Partial consume
        var result = chunk.slice(0, length);
        sock.recvQueue[0] = chunk.slice(length);
        return result;
      }

      // Queue is empty - check for errors before reporting EOF
      if (sock.error) return -{{{ cDefs.EIO }}};
      if (sock._bgReaderDone) return null;  // EOF

      // Non-blocking: return EAGAIN sentinel
      if (sock.nonBlocking) return 'EAGAIN';

      // Blocking: wait for data to arrive via background reader
      if (!sock._bgReaderRunning && sock.reader) {
        DIRECT_SOCKETS.startBackgroundReader(sock);
      }

      return new Promise(function(resolve) {
        var onData = function() {
          if (sock.recvQueue.length > 0) {
            var chunk = sock.recvQueue[0];
            if (chunk.length <= length) {
              sock.recvQueue.shift();
              resolve(chunk);
            } else {
              var result = chunk.slice(0, length);
              sock.recvQueue[0] = chunk.slice(length);
              resolve(result);
            }
          } else if (sock.error) {
            resolve(-sock.error);  // Return negative errno
          } else {
            // EOF
            resolve(null);
          }
        };
        DIRECT_SOCKETS._addWaiter(sock, onData);
      });
    },

    // Write to Direct Sockets writer.
    async writeToSocket(sock, data) {
      if (!sock.writer) return -{{{ cDefs.ENOTCONN }}};
      try {
        // Direct Sockets streams don't accept SharedArrayBuffer views
        // (from pthreads HEAP). Copy to a non-shared buffer.
        if (data.buffer instanceof SharedArrayBuffer) {
          data = new Uint8Array(data);
        }
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

    // Compute poll revents for a socket
    computeRevents(sock, events) {
      var POLLRDNORM = 64, POLLWRNORM = 256;
      var revents = 0;
      if (events & {{{ cDefs.POLLIN }}}) {
        if (sock.recvQueue.length > 0) {
          revents |= {{{ cDefs.POLLIN }}} | POLLRDNORM;
        } else if (sock._bgReaderDone) {
          revents |= {{{ cDefs.POLLHUP }}};
        }
      }
      if (events & {{{ cDefs.POLLOUT }}}) {
        if ((sock.state === 'connected' || sock.state === 'bound') && sock.writer) {
          revents |= {{{ cDefs.POLLOUT }}} | POLLWRNORM;
        }
      }
      if (sock.error) {
        revents |= {{{ cDefs.POLLERR }}};
      }
      if (sock._bgReaderDone && sock.recvQueue.length === 0) {
        revents |= {{{ cDefs.POLLHUP }}};
      }
      return revents;
    },

    // Async DNS resolution via DNS-over-HTTPS (DoH)
    async resolveDNS(hostname, family) {
      // Check cache first
      var cached = DIRECT_SOCKETS.dnsCache[hostname];
      if (cached && cached.expires > Date.now()) return cached.addresses[0];

      // DoH query via fetch
      var type = (family === {{{ cDefs.AF_INET6 }}}) ? 'AAAA' : 'A';
      var typeNum = (type === 'A') ? 1 : 28;
      try {
        var resp = await fetch(
          'https://dns.google/resolve?name=' + encodeURIComponent(hostname) + '&type=' + type,
          { headers: { 'Accept': 'application/dns-json' } }
        );
        var json = await resp.json();

        if (json.Answer && json.Answer.length > 0) {
          var addresses = json.Answer
            .filter(function(a) { return a.type === typeNum; })
            .map(function(a) { return a.data; });
          if (addresses.length === 0) return null;
          var ttl = Math.max((json.Answer[0].TTL || 300), 60);
          DIRECT_SOCKETS.dnsCache[hostname] = {
            addresses: addresses, expires: Date.now() + ttl * 1000
          };
          return addresses[0];
        }
      } catch (e) {
#if SOCKET_DEBUG
        dbg('direct_sockets: DoH resolution failed for ' + hostname + ': ' + e);
#endif
      }
      return null; // NXDOMAIN
    },
  },

  // ---------------------------------------------------------------------------
  // Pipes - in-memory pipes for pipe()/socketpair()
  // ---------------------------------------------------------------------------

  $DIRECT_SOCKETS_PIPES__deps: ['$DIRECT_SOCKETS', '$FS'],
  $DIRECT_SOCKETS_PIPES: {
    pipes: {},

    // Pipe stream_ops for FS integration (so fd_write/fd_read work on pipe fds)
    stream_ops: {
      read(stream, buffer, offset, length, position) {
        var fd = stream.fd;
        var data = DIRECT_SOCKETS_PIPES.readPipe(fd, length);
        if (!data) throw new FS.ErrnoError({{{ cDefs.EAGAIN }}});
        if (data.length === 0) return 0; // EOF
        for (var i = 0; i < data.length; i++) {
          buffer[offset + i] = data[i];
        }
        return data.length;
      },
      write(stream, buffer, offset, length, position) {
        var fd = stream.fd;
        var data = new Uint8Array(length);
        for (var i = 0; i < length; i++) {
          data[i] = buffer[offset + i];
        }
        var result = DIRECT_SOCKETS_PIPES.writePipe(fd, data);
        if (result < 0) throw new FS.ErrnoError(-result);
        return result;
      },
      poll(stream) {
        var fd = stream.fd;
        return DIRECT_SOCKETS_PIPES.computeRevents(fd, 5 /*POLLIN|POLLOUT*/);
      },
      close(stream) {
        DIRECT_SOCKETS_PIPES.closePipeFd(stream.fd);
      },
    },

    // Allocate an FS-backed fd for a pipe end
    allocatePipeFd(name) {
      DIRECT_SOCKETS.ensureRoot();
      var node = FS.createNode(DIRECT_SOCKETS.root, name, {{{ cDefs.S_IFIFO }}} | 438 /* 0666 */, 0);
      var stream = FS.createStream({
        path: name,
        node: node,
        flags: {{{ cDefs.O_RDWR }}},
        seekable: false,
        stream_ops: DIRECT_SOCKETS_PIPES.stream_ops,
      });
      return stream.fd;
    },

    createPipe() {
      var id = DIRECT_SOCKETS.nextId++;
      var readFd = DIRECT_SOCKETS_PIPES.allocatePipeFd('pipe[r' + id + ']');
      var writeFd = DIRECT_SOCKETS_PIPES.allocatePipeFd('pipe[w' + id + ']');
      var pipe = {
        buffer: [],          // array of Uint8Array chunks
        closed: { read: false, write: false },
        pollNotify: null,    // callback for poll integration
      };
      DIRECT_SOCKETS_PIPES.pipes[readFd] = { pipe: pipe, end: 'read', otherFd: writeFd };
      DIRECT_SOCKETS_PIPES.pipes[writeFd] = { pipe: pipe, end: 'write', otherFd: readFd };
      return { readFd: readFd, writeFd: writeFd };
    },

    getPipe(fd) {
      return DIRECT_SOCKETS_PIPES.pipes[fd] || null;
    },

    closePipeFd(fd) {
      var entry = DIRECT_SOCKETS_PIPES.pipes[fd];
      if (!entry) return false;

      if (entry.writePipe) {
        // Socketpair fd: we're closing our end, so:
        // - our read pipe: mark read closed (we stop reading, partner's writes get broken pipe)
        // - our write pipe: mark write closed (we stop writing, partner's reads get EOF)
        entry.pipe.closed.read = true;
        entry.writePipe.closed.write = true;
        // Notify poll waiters
        if (entry.pipe.pollNotify) {
          var cb = entry.pipe.pollNotify;
          entry.pipe.pollNotify = null;
          cb();
        }
        if (entry.writePipe.pollNotify) {
          var cb = entry.writePipe.pollNotify;
          entry.writePipe.pollNotify = null;
          cb();
        }
      } else {
        entry.pipe.closed[entry.end] = true;
        // Notify poll waiters on the other end
        if (entry.pipe.pollNotify) {
          var cb = entry.pipe.pollNotify;
          entry.pipe.pollNotify = null;
          cb();
        }
      }
      delete DIRECT_SOCKETS_PIPES.pipes[fd];
      return true;
    },

    readPipe(fd, length) {
      var entry = DIRECT_SOCKETS_PIPES.pipes[fd];
      if (!entry) return null;
      // For regular pipes, only the read end can read
      // For socketpair fds, the read pipe is in entry.pipe
      if (entry.end === 'write' && !entry.writePipe) return null;
      var pipe = entry.pipe; // read from this pipe
      if (pipe.buffer.length === 0) {
        if (pipe.closed.write) return new Uint8Array(0); // EOF
        return null; // would block
      }
      var chunk = pipe.buffer[0];
      if (chunk.length <= length) {
        pipe.buffer.shift();
        return chunk;
      }
      var result = chunk.slice(0, length);
      pipe.buffer[0] = chunk.slice(length);
      return result;
    },

    writePipe(fd, data) {
      var entry = DIRECT_SOCKETS_PIPES.pipes[fd];
      if (!entry) return -{{{ cDefs.EBADF }}};
      // For regular pipes, only the write end can write
      // For socketpair fds, write goes to entry.writePipe
      var targetPipe;
      if (entry.writePipe) {
        targetPipe = entry.writePipe; // socketpair: write to the other direction's pipe
      } else if (entry.end === 'write') {
        targetPipe = entry.pipe; // regular pipe write end
      } else {
        return -{{{ cDefs.EBADF }}}; // read end of a regular pipe
      }
      if (targetPipe.closed.read) return -{{{ cDefs.EPIPE }}};
      targetPipe.buffer.push(new Uint8Array(data));
      // Notify poll waiters
      if (targetPipe.pollNotify) {
        var cb = targetPipe.pollNotify;
        targetPipe.pollNotify = null;
        cb();
      }
      return data.length;
    },

    computeRevents(fd, events) {
      var entry = DIRECT_SOCKETS_PIPES.pipes[fd];
      if (!entry) return {{{ cDefs.POLLNVAL }}};
      var POLLRDNORM = 64, POLLWRNORM = 256;
      var revents = 0;

      if (entry.writePipe) {
        // Socketpair fd: can both read and write
        var readPipe = entry.pipe;
        var writePipe = entry.writePipe;
        if (events & {{{ cDefs.POLLIN }}}) {
          if (readPipe.buffer.length > 0) revents |= {{{ cDefs.POLLIN }}} | POLLRDNORM;
        }
        if (readPipe.closed.write && readPipe.buffer.length === 0) revents |= {{{ cDefs.POLLHUP }}};
        if (events & {{{ cDefs.POLLOUT }}}) {
          if (!writePipe.closed.read) revents |= {{{ cDefs.POLLOUT }}} | POLLWRNORM;
        }
      } else if (entry.end === 'read') {
        var pipe = entry.pipe;
        if (events & {{{ cDefs.POLLIN }}}) {
          if (pipe.buffer.length > 0) revents |= {{{ cDefs.POLLIN }}} | POLLRDNORM;
        }
        if (pipe.closed.write && pipe.buffer.length === 0) revents |= {{{ cDefs.POLLHUP }}};
      } else {
        var pipe = entry.pipe;
        if (events & {{{ cDefs.POLLOUT }}}) {
          if (!pipe.closed.read) revents |= {{{ cDefs.POLLOUT }}} | POLLWRNORM;
        }
        if (pipe.closed.read) revents |= {{{ cDefs.POLLERR }}};
      }
      return revents;
    },
  },

  // ---------------------------------------------------------------------------
  // Syscall implementations
  // ---------------------------------------------------------------------------

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

  __syscall_connect__async: true,
  __syscall_connect: async (fd, addr, addrlen) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};
    if (sock.state === 'connected' || sock.state === 'connecting') return -{{{ cDefs.EISCONN }}};
    if (sock.state !== 'created' && sock.state !== 'bound') return -{{{ cDefs.EINVAL }}};

    var dest = DIRECT_SOCKETS.parseSockaddr(addr, addrlen);
    if (dest.errno) return -dest.errno;

#if SOCKET_DEBUG
    dbg(`direct_sockets: connect(fd=${fd}, addr=${dest.addr}, port=${dest.port})`);
#endif

    sock.state = 'connecting';

    try {
      if (sock.type === {{{ cDefs.SOCK_STREAM }}}) {
        // TCP connect - pass local endpoint from prior bind() if present
        if (typeof TCPSocket === 'undefined') return -{{{ cDefs.ENOSYS }}};
        var opts = DIRECT_SOCKETS.buildTCPOptions(sock);
        if (sock.localAddress && sock.localAddress !== '0.0.0.0') {
          opts.localAddress = sock.localAddress;
        }
        if (sock.localPort) {
          opts.localPort = sock.localPort;
        }
        var tcpSocket = new TCPSocket(dest.addr, dest.port, opts);
        var openInfo = await tcpSocket.opened;

        sock.tcpSocket = tcpSocket;
        sock.reader = openInfo.readable.getReader();
        sock.writer = openInfo.writable.getWriter();
        sock.remoteAddress = openInfo.remoteAddress || dest.addr;
        sock.remotePort = openInfo.remotePort || dest.port;
        sock.localAddress = openInfo.localAddress || sock.localAddress || '0.0.0.0';
        sock.localPort = openInfo.localPort || sock.localPort || 0;
        sock.state = 'connected';

        // Start background reader for poll/non-blocking support
        DIRECT_SOCKETS.startBackgroundReader(sock);

      } else {
        // UDP "connect" - creates a connected-mode UDPSocket
        // Close existing UDP socket from prior bind() to avoid leaking
        if (sock.udpSocket) {
          try {
            if (sock.reader) { sock.reader.releaseLock(); sock.reader = null; }
            if (sock.writer) { sock.writer.releaseLock(); sock.writer = null; }
            await sock.udpSocket.close();
          } catch (e) {}
          sock.udpSocket = null;
        }

        if (typeof UDPSocket === 'undefined') return -{{{ cDefs.ENOSYS }}};
        var opts = DIRECT_SOCKETS.buildUDPOptions(sock);
        opts.remoteAddress = dest.addr;
        opts.remotePort = dest.port;
        // Honor local endpoint from prior bind()
        if (sock.localAddress && sock.localAddress !== '0.0.0.0') {
          opts.localAddress = sock.localAddress;
        }
        if (sock.localPort) {
          opts.localPort = sock.localPort;
        }
        var udpSocket = new UDPSocket(opts);
        var openInfo = await udpSocket.opened;

        sock.udpSocket = udpSocket;
        sock.reader = openInfo.readable.getReader();
        sock.writer = openInfo.writable.getWriter();
        DIRECT_SOCKETS.attachMulticastController(sock, openInfo);
        sock.remoteAddress = openInfo.remoteAddress || dest.addr;
        sock.remotePort = openInfo.remotePort || dest.port;
        sock.localAddress = openInfo.localAddress || sock.localAddress || '0.0.0.0';
        sock.localPort = openInfo.localPort || sock.localPort || 0;
        sock.state = 'connected';

        // Start background reader for poll/non-blocking support
        DIRECT_SOCKETS.startBackgroundReader(sock);
      }
    } catch (e) {
#if SOCKET_DEBUG
      dbg(`direct_sockets: connect error: ${e}`);
#endif
      // Restore prior state: if socket was bound, keep it bound (important for UDP)
      sock.state = (sock.localAddress || sock.localPort) ? 'bound' : 'created';
      if (e.name === 'NotAllowedError') return -{{{ cDefs.EACCES }}};
      return -{{{ cDefs.ECONNREFUSED }}};
    }

    return 0;
  },

  __syscall_bind__async: true,
  __syscall_bind: async (fd, addr, addrlen) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};
    if (sock.state !== 'created') return -{{{ cDefs.EINVAL }}};

    var bindAddr = DIRECT_SOCKETS.parseSockaddr(addr, addrlen);
    if (bindAddr.errno) return -bindAddr.errno;

#if SOCKET_DEBUG
    dbg(`direct_sockets: bind(fd=${fd}, addr=${bindAddr.addr}, port=${bindAddr.port})`);
#endif

    // Store the requested bind address. Actual socket creation is deferred
    // to listen() for TCP servers, or done here for UDP bound sockets.
    sock.localAddress = bindAddr.addr;
    sock.localPort = bindAddr.port;

    if (sock.type === {{{ cDefs.SOCK_DGRAM }}}) {
      // UDP: create bound-mode UDPSocket immediately
      if (typeof UDPSocket === 'undefined') return -{{{ cDefs.ENOSYS }}};
      try {
        var opts = DIRECT_SOCKETS.buildUDPOptions(sock);
        opts.localAddress = bindAddr.addr;
        opts.localPort = bindAddr.port;
        var udpSocket = new UDPSocket(opts);
        var openInfo = await udpSocket.opened;

        sock.udpSocket = udpSocket;
        sock.reader = openInfo.readable.getReader();
        sock.writer = openInfo.writable.getWriter();
        DIRECT_SOCKETS.attachMulticastController(sock, openInfo);
        sock.localAddress = openInfo.localAddress || bindAddr.addr;
        sock.localPort = openInfo.localPort || bindAddr.port;
        sock.state = 'bound';

        // Start background reader for poll support
        DIRECT_SOCKETS.startBackgroundReader(sock);
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

  __syscall_listen__async: true,
  __syscall_listen: async (fd, backlog) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};
    if (sock.type !== {{{ cDefs.SOCK_STREAM }}}) return -{{{ cDefs.EOPNOTSUPP }}};
    if (sock.state !== 'bound') return -{{{ cDefs.EINVAL }}};

#if SOCKET_DEBUG
    dbg(`direct_sockets: listen(fd=${fd}, backlog=${backlog})`);
#endif

    if (typeof TCPServerSocket === 'undefined') return -{{{ cDefs.ENOSYS }}};
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

  __syscall_accept4__deps: ['$writeSockaddr', '$DNS'],
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

      // Start background reader for poll/non-blocking support
      DIRECT_SOCKETS.startBackgroundReader(newSock);

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
        if (dest.errno) return -dest.errno;

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

  __syscall_recvfrom__deps: ['$writeSockaddr', '$DNS'],
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
      if (data === 'EAGAIN') return -{{{ cDefs.EAGAIN }}};
      if (typeof data === 'number') return data;  // Error code (negative errno)
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

      // Try recvQueue first (filled by background reader)
      if (sock.recvQueue.length > 0) {
        var message = sock.recvQueue.shift();
        var msgData = message.data || message;
        var copyLen = Math.min(msgData.length, len);
        HEAPU8.set(msgData.subarray(0, copyLen), buf);
        if (addr && message.remoteAddress) {
          var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(message.remoteAddress), message.remotePort, addrlen);
        }
        return copyLen;
      }

      if (sock._bgReaderDone) return 0;
      if (sock.nonBlocking) return -{{{ cDefs.EAGAIN }}};

      // Wait for background reader to deliver data
      await new Promise(function(resolve) {
        DIRECT_SOCKETS._addWaiter(sock, resolve);
      });

      if (sock.recvQueue.length === 0) return 0;

      var message = sock.recvQueue.shift();
      var msgData = message.data || message;
      var copyLen = Math.min(msgData.length, len);
      HEAPU8.set(msgData.subarray(0, copyLen), buf);

      if (addr && message.remoteAddress) {
        var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(message.remoteAddress), message.remotePort, addrlen);
      }

      return copyLen;
    }
  },

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

  __syscall_getsockname__deps: ['$writeSockaddr', '$DNS'],
  __syscall_getsockname: (fd, addr, addrlen) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};

    var localAddr = sock.localAddress || '0.0.0.0';
    var localPort = sock.localPort || 0;

    var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(localAddr), localPort, addrlen);
    return errno ? -errno : 0;
  },

  __syscall_getpeername__deps: ['$writeSockaddr', '$DNS'],
  __syscall_getpeername: (fd, addr, addrlen) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};

    if (!sock.remoteAddress) return -{{{ cDefs.ENOTCONN }}};

    var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(sock.remoteAddress), sock.remotePort, addrlen);
    return errno ? -errno : 0;
  },

  __syscall_setsockopt__async: true,
  __syscall_setsockopt: async (fd, level, optname, optval, optlen) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};

#if SOCKET_DEBUG
    dbg(`direct_sockets: setsockopt(fd=${fd}, level=${level}, optname=${optname})`);
#endif

    // Direct Sockets only supports a few options, and they must be set at
    // construction time. We defer them and apply when connect/bind is called.
    // musl socket option constants (stable ABI):
    var SO_REUSEADDR = 2, SO_TYPE = 3, SO_SNDBUF = 7, SO_RCVBUF = 8;
    var SO_KEEPALIVE = 9, SO_REUSEPORT = 15;
    var TCP_NODELAY = 1, TCP_KEEPIDLE = 4, TCP_KEEPINTVL = 5;
    var IP_MULTICAST_TTL = 33, IP_MULTICAST_LOOP = 34;
    var IP_ADD_MEMBERSHIP = 35, IP_DROP_MEMBERSHIP = 36;
    var IPV6_MULTICAST_LOOP = 18, IPV6_MULTICAST_HOPS = 19;
    var IPV6_JOIN_GROUP = 20, IPV6_LEAVE_GROUP = 21;

    if (level === {{{ cDefs.SOL_SOCKET }}}) {
      switch (optname) {
        case SO_REUSEADDR:
        case SO_REUSEPORT:
          // Silently accept - no equivalent, but harmless
          return 0;
        case SO_SNDBUF:
          sock.options.sendBufferSize = {{{ makeGetValue('optval', 0, 'i32') }}};
          return 0;
        case SO_RCVBUF:
          sock.options.receiveBufferSize = {{{ makeGetValue('optval', 0, 'i32') }}};
          return 0;
        case SO_KEEPALIVE:
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
    } else if (level === {{{ cDefs.IPPROTO_TCP }}}) {
      switch (optname) {
        case TCP_NODELAY:
          sock.options.noDelay = !!{{{ makeGetValue('optval', 0, 'i32') }}};
          return 0;
        case TCP_KEEPIDLE:
        case TCP_KEEPINTVL:
          // Map to keepAliveDelay (in milliseconds)
          sock.options.keepAliveDelay = {{{ makeGetValue('optval', 0, 'i32') }}} * 1000;
          return 0;
        default:
#if SOCKET_DEBUG
          dbg(`direct_sockets: setsockopt ignoring IPPROTO_TCP option ${optname}`);
#endif
          return 0;
      }
    } else if (level === 0 /* IPPROTO_IP */) {
      switch (optname) {
        case IP_MULTICAST_TTL:
          sock.options.multicastTtl = HEAPU8[optval];
          return 0;
        case IP_MULTICAST_LOOP:
          sock.options.multicastLoopback = !!HEAPU8[optval];
          return 0;
        case IP_ADD_MEMBERSHIP:
          return DIRECT_SOCKETS.joinMulticastGroup(sock, DIRECT_SOCKETS.parseIpMreq(optval, optlen));
        case IP_DROP_MEMBERSHIP:
          return DIRECT_SOCKETS.leaveMulticastGroup(sock, DIRECT_SOCKETS.parseIpMreq(optval, optlen));
        default:
          return 0;
      }
    } else if (level === 41 /* IPPROTO_IPV6 */) {
      switch (optname) {
        case IPV6_MULTICAST_LOOP:
          sock.options.multicastLoopback = !!HEAPU8[optval];
          return 0;
        case IPV6_MULTICAST_HOPS:
          sock.options.multicastTtl = HEAPU8[optval];
          return 0;
        case IPV6_JOIN_GROUP:
          return DIRECT_SOCKETS.joinMulticastGroup(sock, DIRECT_SOCKETS.parseIpv6Mreq(optval, optlen));
        case IPV6_LEAVE_GROUP:
          return DIRECT_SOCKETS.leaveMulticastGroup(sock, DIRECT_SOCKETS.parseIpv6Mreq(optval, optlen));
        default:
          return 0;
      }
    }

    // Silently accept unknown levels
    return 0;
  },

  __syscall_getsockopt: (fd, level, optname, optval, optlen) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};

    var SO_TYPE = 3;
    if (level === {{{ cDefs.SOL_SOCKET }}}) {
      if (optname === {{{ cDefs.SO_ERROR }}}) {
        {{{ makeSetValue('optval', 0, 'sock.error', 'i32') }}};
        {{{ makeSetValue('optlen', 0, 4, 'i32') }}};
        sock.error = 0;
        return 0;
      }
      if (optname === SO_TYPE) {
        {{{ makeSetValue('optval', 0, 'sock.type', 'i32') }}};
        {{{ makeSetValue('optlen', 0, 4, 'i32') }}};
        return 0;
      }
    }

    return -{{{ cDefs.ENOPROTOOPT }}};
  },

  // sendmsg/recvmsg: minimal implementations that delegate to sendto/recvfrom
  __syscall_sendmsg__deps: ['__syscall_sendto'],
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
        if (dest.errno) return -dest.errno;
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

  __syscall_recvmsg__deps: ['$writeSockaddr', '$DNS'],
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
      if (data === 'EAGAIN') return -{{{ cDefs.EAGAIN }}};
      if (typeof data === 'number') return data;  // Error code (negative errno)
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
      // UDP: consume from recvQueue (populated by background reader)
      if (!sock.reader && sock.recvQueue.length === 0) return -{{{ cDefs.ENOTCONN }}};

      // Check non-blocking mode
      if (sock.nonBlocking && sock.recvQueue.length === 0) {
        if (sock._bgReaderDone) return 0;
        return -{{{ cDefs.EAGAIN }}};
      }

      // Wait for data if queue is empty
      if (sock.recvQueue.length === 0) {
        if (sock._bgReaderDone) return 0;
        await new Promise(function(resolve) {
          DIRECT_SOCKETS._addWaiter(sock, resolve);
        });
        if (sock.recvQueue.length === 0) return 0;
      }

      var msg = sock.recvQueue.shift();
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
    }
  },

  // ---------------------------------------------------------------------------
  // poll() implementation
  // ---------------------------------------------------------------------------

  __syscall_poll__deps: ['$DIRECT_SOCKETS_PIPES'],
  __syscall_poll__async: true,
  __syscall_poll: async (fds, nfds, timeout) => {
    // struct pollfd { int fd; short events; short revents; }
    // sizeof(pollfd) = 8, offsets: fd=0, events=4, revents=6

    var POLLFD_SIZE = 8;
    var count = 0;

    // Phase 1: synchronous scan
    for (var i = 0; i < nfds; i++) {
      var ptr = fds + i * POLLFD_SIZE;
      var fd = {{{ makeGetValue('ptr', 0, 'i32') }}};
      var events = {{{ makeGetValue('ptr', 4, 'i16') }}};
      var revents = 0;

      // Check pipe fds first
      var pipeEntry = DIRECT_SOCKETS_PIPES.getPipe(fd);
      if (pipeEntry) {
        revents = DIRECT_SOCKETS_PIPES.computeRevents(fd, events);
      } else {
        // Check socket fds
        var sock = DIRECT_SOCKETS.getSocket(fd);
        if (sock) {
          revents = DIRECT_SOCKETS.computeRevents(sock, events);
        }
        // For unknown fds (FS fds like stdin/stdout), leave revents=0
      }

      {{{ makeSetValue('ptr', 6, 'revents', 'i16') }}};
      if (revents) count++;
    }

    // If any events detected or timeout is 0, return immediately
    if (count > 0 || timeout === 0) return count;

    // Phase 2: async wait (timeout > 0 or timeout === -1 for infinite)
    return new Promise(function(resolve) {
      var timer = null;
      var resolved = false;

      var cleanup = function() {
        if (resolved) return;
        resolved = true;
        if (timer) clearTimeout(timer);
        // Remove our waiter from all watched fds
        for (var i = 0; i < nfds; i++) {
          var ptr = fds + i * POLLFD_SIZE;
          var fd = {{{ makeGetValue('ptr', 0, 'i32') }}};
          var sock = DIRECT_SOCKETS.getSocket(fd);
          if (sock) DIRECT_SOCKETS._removeWaiter(sock, onNotify);
          var pipeEntry = DIRECT_SOCKETS_PIPES.getPipe(fd);
          if (pipeEntry) {
            if (pipeEntry.pipe.pollNotify === onNotify) pipeEntry.pipe.pollNotify = null;
            if (pipeEntry.writePipe && pipeEntry.writePipe.pollNotify === onNotify) pipeEntry.writePipe.pollNotify = null;
          }
        }
      };

      var onNotify = function() {
        if (resolved) return;
        // Re-scan all fds
        var newCount = 0;
        for (var i = 0; i < nfds; i++) {
          var ptr = fds + i * POLLFD_SIZE;
          var fd = {{{ makeGetValue('ptr', 0, 'i32') }}};
          var events = {{{ makeGetValue('ptr', 4, 'i16') }}};
          var revents = 0;

          var pipeEntry = DIRECT_SOCKETS_PIPES.getPipe(fd);
          if (pipeEntry) {
            revents = DIRECT_SOCKETS_PIPES.computeRevents(fd, events);
          } else {
            var sock = DIRECT_SOCKETS.getSocket(fd);
            if (sock) {
              revents = DIRECT_SOCKETS.computeRevents(sock, events);
            }
          }

          {{{ makeSetValue('ptr', 6, 'revents', 'i16') }}};
          if (revents) newCount++;
        }
        if (newCount > 0) {
          cleanup();
          resolve(newCount);
        }
        // If no events yet, notifiers are still registered - keep waiting
      };

      // Set timeout
      if (timeout > 0) {
        timer = setTimeout(function() {
          cleanup();
          resolve(0);
        }, timeout);
      }

      // Register poll notifiers on each watched fd (for both POLLIN and POLLOUT)
      for (var i = 0; i < nfds; i++) {
        var ptr = fds + i * POLLFD_SIZE;
        var fd = {{{ makeGetValue('ptr', 0, 'i32') }}};
        var events = {{{ makeGetValue('ptr', 4, 'i16') }}};

        var sock = DIRECT_SOCKETS.getSocket(fd);
        if (sock) {
          DIRECT_SOCKETS._addWaiter(sock, onNotify);
          // Ensure background reader is running for connected sockets
          if ((events & 1 /*POLLIN*/) && sock.state === 'connected' && sock.reader && !sock._bgReaderRunning) {
            DIRECT_SOCKETS.startBackgroundReader(sock);
          }
        }
        var pipeEntry = DIRECT_SOCKETS_PIPES.getPipe(fd);
        if (pipeEntry) {
          pipeEntry.pipe.pollNotify = onNotify;
          if (pipeEntry.writePipe) {
            pipeEntry.writePipe.pollNotify = onNotify;
          }
        }
      }
    });
  },

  // ---------------------------------------------------------------------------
  // pipe2() implementation
  // ---------------------------------------------------------------------------

  __syscall_pipe2__deps: ['$DIRECT_SOCKETS_PIPES'],
  __syscall_pipe2: (fdsPtr, flags) => {
    var result = DIRECT_SOCKETS_PIPES.createPipe();

#if SOCKET_DEBUG
    dbg('direct_sockets: pipe2() -> read=' + result.readFd + ', write=' + result.writeFd);
#endif

    {{{ makeSetValue('fdsPtr', 0, 'result.readFd', 'i32') }}};
    {{{ makeSetValue('fdsPtr', 4, 'result.writeFd', 'i32') }}};
    return 0;
  },

  // ---------------------------------------------------------------------------
  // socketpair() implementation
  // ---------------------------------------------------------------------------

  __syscall_socketpair__deps: ['$DIRECT_SOCKETS_PIPES'],
  __syscall_socketpair: (domain, type, protocol, sv) => {
    // Two cross-connected pipes: fd0's write goes to fd1's read and vice versa
    var fd0 = DIRECT_SOCKETS_PIPES.allocatePipeFd('sockpair[0.' + Object.keys(DIRECT_SOCKETS_PIPES.pipes).length + ']');
    var fd1 = DIRECT_SOCKETS_PIPES.allocatePipeFd('sockpair[1.' + Object.keys(DIRECT_SOCKETS_PIPES.pipes).length + ']');

    // Create pipe objects directly (no intermediate fds needed)
    var spPipe0to1 = {
      buffer: [],
      closed: { read: false, write: false },
      pollNotify: null,
    };
    var spPipe1to0 = {
      buffer: [],
      closed: { read: false, write: false },
      pollNotify: null,
    };

    // fd0 reads from spPipe1to0, writes to spPipe0to1
    // fd1 reads from spPipe0to1, writes to spPipe1to0
    DIRECT_SOCKETS_PIPES.pipes[fd0] = { pipe: spPipe1to0, end: 'read', otherFd: fd1, writePipe: spPipe0to1 };
    DIRECT_SOCKETS_PIPES.pipes[fd1] = { pipe: spPipe0to1, end: 'read', otherFd: fd0, writePipe: spPipe1to0 };

#if SOCKET_DEBUG
    dbg('direct_sockets: socketpair() -> fd0=' + fd0 + ', fd1=' + fd1);
#endif

    {{{ makeSetValue('sv', 0, 'fd0', 'i32') }}};
    {{{ makeSetValue('sv', 4, 'fd1', 'i32') }}};
    return 0;
  },

  // ---------------------------------------------------------------------------
  // fcntl64 - F_GETFL / F_SETFL for O_NONBLOCK support
  // ---------------------------------------------------------------------------

  __syscall_fcntl64__deps: ['$DIRECT_SOCKETS_PIPES'],
  __syscall_fcntl64: (fd, cmd, varargs) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    var pipeEntry = DIRECT_SOCKETS_PIPES.getPipe(fd);

    if (!sock && !pipeEntry) return -{{{ cDefs.EBADF }}};

    if (cmd === {{{ cDefs.F_GETFL }}}) {
      if (sock) {
        return sock.nonBlocking ? {{{ cDefs.O_NONBLOCK }}} : 0;
      }
      return 0;
    }
    if (cmd === {{{ cDefs.F_SETFL }}}) {
      var flags = {{{ makeGetValue('varargs', 0, 'i32') }}};
      if (sock) {
        sock.nonBlocking = !!(flags & {{{ cDefs.O_NONBLOCK }}});
        sock.flags = flags;
      }
      return 0;
    }
    if (cmd === {{{ cDefs.F_GETFD }}}) return 0;
    if (cmd === {{{ cDefs.F_SETFD }}}) return 0;

    return -{{{ cDefs.EINVAL }}};
  },

  // ---------------------------------------------------------------------------
  // ioctl - FIONBIO for non-blocking support
  // ---------------------------------------------------------------------------

  // ioctl for FIONBIO / FIONREAD
  __syscall_ioctl: (fd, op, varargs) => {
    var sock = DIRECT_SOCKETS.getSocket(fd);
    if (!sock) return -{{{ cDefs.EBADF }}};

    if (op === {{{ cDefs.FIONBIO }}}) {
      var val = {{{ makeGetValue('varargs', 0, 'i32') }}};
      var nonblock = {{{ makeGetValue('val', 0, 'i32') }}};
      sock.nonBlocking = !!nonblock;
#if SOCKET_DEBUG
      dbg('direct_sockets: ioctl FIONBIO fd=' + fd + ' nonBlocking=' + sock.nonBlocking);
#endif
      return 0;
    }

    if (op === {{{ cDefs.FIONREAD }}}) {
      var argp = {{{ makeGetValue('varargs', 0, 'i32') }}};
      var avail = 0;
      for (var i = 0; i < sock.recvQueue.length; i++) {
        var entry = sock.recvQueue[i];
        avail += (entry.data ? entry.data.length : entry.length);
      }
      {{{ makeSetValue('argp', 0, 'avail', 'i32') }}};
      return 0;
    }

    // Silently ignore other ioctls
    return 0;
  },

  // ---------------------------------------------------------------------------
  // DNS resolution - async DoH-based getaddrinfo support
  // ---------------------------------------------------------------------------

  _emscripten_lookup_name__deps: ['$DNS', '$inetPton4', '$UTF8ToString'],
  _emscripten_lookup_name__async: true,
  _emscripten_lookup_name: async (name) => {
    var hostname = UTF8ToString(name);

    // Handle special cases that don't need DoH
    // DNS.lookup_name returns a string; inetPton4 converts to packed uint32
    if (hostname === 'localhost' || hostname === '127.0.0.1') {
      return inetPton4(DNS.lookup_name('localhost'));
    }

    // Check if it's already an IP address (dotted decimal)
    if (/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/.test(hostname)) {
      return inetPton4(DNS.lookup_name(hostname));
    }

    // Try DoH resolution for real hostnames
    var realIp = await DIRECT_SOCKETS.resolveDNS(hostname, {{{ cDefs.AF_INET }}});
    if (realIp) {
#if SOCKET_DEBUG
      dbg('direct_sockets: DoH resolved ' + hostname + ' -> ' + realIp);
#endif
      // Convert IP string to packed 32-bit integer (little-endian, same as inetPton4)
      var parts = realIp.split('.');
      var packed = ((parseInt(parts[0])) |
                    (parseInt(parts[1]) << 8) |
                    (parseInt(parts[2]) << 16) |
                    (parseInt(parts[3]) << 24)) >>> 0;

      // Register reverse mapping: real IP -> hostname
      // So parseSockaddr can resolve it back for TCPSocket constructor
      if (DNS.address_map) {
        if (!DNS.address_map.addrs) DNS.address_map.addrs = {};
        if (!DNS.address_map.names) DNS.address_map.names = {};
        DNS.address_map.addrs[hostname] = realIp;
        DNS.address_map.names[realIp] = hostname;
      }

      // Also store in our cache for parseSockaddr
      DIRECT_SOCKETS.dnsCache['_real_' + hostname] = realIp;
      DIRECT_SOCKETS.dnsCache['_reverse_' + realIp] = hostname;

      return packed;
    }

    // Fallback to Emscripten's fake DNS
    return DNS.lookup_name(hostname);
  },

  // Internal helper for closing - not a syscall but used by shutdown and close
  $DIRECT_SOCKETS__postset: `
    DIRECT_SOCKETS._closeSocket = async function(sock) {
      try {
        sock._bgReaderDone = true;
        DIRECT_SOCKETS._notifyWaiters(sock);
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
      sock.recvQueue = [];
      // Close the FS stream if registered
      if (sock.stream) {
        try { FS.closeStream(sock.stream.fd); } catch(e) {}
        sock.stream = null;
      }
    };
  `,
};

autoAddDeps(DirectSocketsLibrary, '$DIRECT_SOCKETS');

for (var x in DirectSocketsLibrary) {
  if (x.startsWith('__syscall_')) {
    wrapSyscallFunction(x, DirectSocketsLibrary, false);
  }
}

addToLibrary(DirectSocketsLibrary);

#endif // DIRECT_SOCKETS
