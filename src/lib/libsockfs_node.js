/**
 * @license
 * Copyright 2026 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// TCP and UDP over node:net / node:dgram (-sNODERAWSOCKETS). This implements
// the same sock_ops contract and SOCKFS.emit readiness callbacks as the
// WebSocket backend, so existing readiness reactors work unchanged.
//
// The code is split in two: nodeSockHelpers holds the node plumbing (module
// loading, handle creation, errno mapping, event wiring) and nodeSockOps is the
// sock_ops interface the rest of emscripten calls (poll/bind/connect/...). The
// interface methods only ever delegate to helpers; helpers never call back into
// the interface.
//
// bind() is eager and synchronous: it produces a role-neutral bound handle and
// records the kernel-assigned name immediately, so getsockname() needs no
// promotion, a conflict surfaces right here as EADDRINUSE, and the handle is
// adopted as-is by listen() (server.listen) or connect() (net.Socket). The bind
// primitive is chosen once per capability: the public, synchronous
// net.BoundSocket when the runtime offers it, else the private tcp_wrap binding
// as a fallback (net.Server's listen is async and cannot report an assigned
// ephemeral port up front, so it can't drive bind on its own). connect() goes
// through net.Socket, adopting the bound handle when one exists so an explicit
// source address/port is honored, and otherwise letting the kernel assign one.
//
// UDP uses the public node:dgram socket when it exposes a synchronous bindSync
// (a recent node addition that ships alongside connectSync), giving the
// bind(:0) + getsockname() and a real connect() that libc needs up front. There
// connect() is a real kernel connect, so the OS filters non-peer datagrams and
// surfaces async errors (e.g. ICMP ECONNREFUSED). Older node has no synchronous
// dgram bind/connect, so it falls back to a low-level udp_wrap handle and a
// connect() emulated in JS (record the peer, filter in udpDeliver). The choice
// is made per socket via useDgram().
//
// Under -pthread with PROXY_TO_PTHREAD, main() and every socket syscall run on
// the same worker, so the node handles, their event loop and the readiness
// callbacks all live on that one thread (a socket is not shared across threads,
// just as in the WebSocket backend). Payloads are copied out of (possibly
// shared) wasm memory before being handed to node, so a SharedArrayBuffer heap
// is safe.

{{{
// sock.state lifecycle, substituted as numbers at build time. A fresh socket
// has an undefined (falsy) state until its first bind/connect.
const SOCK_STATE_CONNECTING = 1;
const SOCK_STATE_CONNECTED = 2;
const SOCK_STATE_BOUND = 3;
const SOCK_STATE_LISTEN = 4;
const SOCK_STATE_CLOSED = 5;
null;
}}}

var NodeSockFSLibrary = {
  // Node plumbing shared by the interface methods below.
  $nodeSockHelpers__deps: ['$SOCKFS', '$ERRNO_CODES'],
  $nodeSockHelpers: {
    // node builtins, resolved once each. getBuiltinModule works in both
    // CommonJS and ESM output, with require as the fallback.
    getNet() {
      return nodeSockHelpers.netModule ??= (process.getBuiltinModule || require)('net');
    },
    getUtil() {
      return nodeSockHelpers.utilModule ??= (process.getBuiltinModule || require)('util');
    },
    getDgram() {
      return nodeSockHelpers.dgramModule ??= (process.getBuiltinModule || require)('dgram');
    },
    // True when node:dgram exposes both synchronous bindSync and connectSync
    // (a recent addition), letting UDP run entirely on the public API. A runtime
    // missing either falls back to the private udp_wrap handle, which provides
    // both, so we never end up on a half-supported public path.
    useDgram() {
      var proto = nodeSockHelpers.getDgram().Socket.prototype;
      return nodeSockHelpers.dgramSync ??= !!(proto.bindSync && proto.connectSync);
    },
    // Queue a received datagram and signal readiness. Shared by both backends.
    udpDeliver(sock, address, port, data) {
      // A connected datagram socket (sock.daddr set) must only see datagrams
      // from its peer; drop anything from another source, matching the kernel
      // filtering a real connect() would do (the udp_wrap fallback has no real
      // connect, so we enforce it here).
      if (sock.daddr !== undefined && (address !== sock.daddr || port !== sock.dport)) {
        return;
      }
      sock.recv_queue.push({ addr: address, port, data });
      SOCKFS.emit('message', sock.stream.fd);
    },
    // Map a node error (its `.code` string) to an emscripten errno. Most node
    // codes are errno names already; a few are node-specific and aliased here.
    nodeErrToErrno(e) {
      var code = e && e.code;
      if (code === 'ERR_SOCKET_DGRAM_NOT_CONNECTED') return {{{ cDefs.ENOTCONN }}};
      if (code === 'ERR_SOCKET_BAD_PORT') return {{{ cDefs.EINVAL }}};
      return (code && ERRNO_CODES[code]) || {{{ cDefs.EIO }}};
    },
    // Map a libuv result code (negative errno, as returned by the low-level
    // handle's bind/getsockname) to an emscripten errno.
    codeToErrno(code) {
      var name = nodeSockHelpers.getUtil().getSystemErrorName(code);
      return (name && ERRNO_CODES[name]) || {{{ cDefs.EINVAL }}};
    },
    // TCP binds eagerly and synchronously, so there is no deferred bind and no
    // lazy handle promotion - the only difference between the two backends is how
    // a bound handle is produced: the public net.BoundSocket when node offers it,
    // else the private tcp_wrap binding. Chosen once, like useDgram().
    useBoundSocket() {
      return nodeSockHelpers.boundSocketOk ??= !!nodeSockHelpers.getNet().BoundSocket;
    },
    // Synchronously bind a TCP socket to addr:port (0 = ephemeral) and record the
    // kernel-assigned name immediately. sock.bound is the resulting role-neutral
    // handle - a net.BoundSocket, or a raw tcp_wrap handle - adopted as-is by
    // listen() (server.listen) and connect() (net.Socket). So getsockname() needs
    // no promotion, a conflict surfaces here as EADDRINUSE (exactly when POSIX
    // bind() would), and close() releases it if unadopted.
    bindHandle(sock, addr, port) {
      var o = sock.opts || {};
      if (nodeSockHelpers.useBoundSocket()) {
        // The constructor binds synchronously and throws a bind conflict
        // (EADDRINUSE etc.) right here; address() on the bound handle is safe.
        // ipv6Only/reusePort are bind-time options, applied here from the cache.
        try {
          var bh = new (nodeSockHelpers.getNet().BoundSocket)({
            host: addr, port, ipv6Only: o.ipv6Only, reusePort: o.reusePort,
          });
        }
        catch (e) { throw new FS.ErrnoError(nodeSockHelpers.nodeErrToErrno(e)); }
        var n = bh.address();
        sock.bound = bh;
        sock.saddr = n.address;
        sock.sport = n.port;
        return;
      }
      var tcp;
      try {
        tcp = process.binding('tcp_wrap');
      } catch (e) {
        throw new FS.ErrnoError({{{ cDefs.EOPNOTSUPP }}});
      }
      var handle = new tcp.TCP(tcp.constants.SOCKET);
      // bind6 for IPv6 literals, honoring IPV6_V6ONLY via the bind flags.
      var code = addr.includes(':')
        ? handle.bind6(addr, port, o.ipv6Only ? 1 /* UV_TCP_IPV6ONLY */ : 0)
        : handle.bind(addr, port);
      if (!code) {
        var name = {};
        code = handle.getsockname(name);
        if (!code) {
          sock.bound = handle;
          sock.saddr = name.address;
          sock.sport = name.port;
          return;
        }
      }
      try { handle.close(); } catch (e) {}
      throw new FS.ErrnoError(nodeSockHelpers.codeToErrno(code));
    },
    // The peer address is already a numeric IP (emscripten resolves names in
    // its own DNS layer), so skip node's async DNS lookup. The family follows
    // the literal: a colon means IPv6.
    noLookup(host, _opts, cb) {
      cb(null, host, host.includes(':') ? 6 : 4);
    },
    // The UDP backing object. With a synchronous dgram bindSync available we use
    // a public node:dgram socket (sock.udpPublic); otherwise we fall back to a
    // private udp_wrap handle, which is the only older-node way to get a
    // synchronous bind() + getsockname(). Either way recv wiring funnels through
    // udpDeliver, so bind/send/recv/poll/close stay backend-agnostic.
    ensureUdpHandle(sock) {
      if (sock.udp) return sock.udp;
      if (nodeSockHelpers.useDgram()) {
        var socket = nodeSockHelpers.getDgram().createSocket(sock.family === {{{ cDefs.AF_INET6 }}} ? 'udp6' : 'udp4');
        socket.on('message', (msg, rinfo) => {
          var data = new Uint8Array(msg.length);
          data.set(msg);
          nodeSockHelpers.udpDeliver(sock, rinfo.address, rinfo.port, data);
        });
        socket.on('error', (e) => {
          sock.error = nodeSockHelpers.nodeErrToErrno(e);
          SOCKFS.emit('error', [sock.stream.fd, sock.error, (e && e.message) || 'udp error']);
        });
        sock.udpPublic = true;
        return sock.udp = socket;
      }
      var udp = process.binding('udp_wrap');
      var handle = new udp.UDP();
      sock.sendWrap = udp.SendWrap;
      handle.onmessage = (nread, _h, buf, rinfo) => {
        if (nread < 0) {
          sock.error = nodeSockHelpers.codeToErrno(nread);
          SOCKFS.emit('error', [sock.stream.fd, sock.error, 'udp error']);
          return;
        }
        var data = new Uint8Array(buf.length);
        data.set(buf);
        nodeSockHelpers.udpDeliver(sock, rinfo.address, rinfo.port, data);
      };
      return sock.udp = handle;
    },
    // Begin receiving exactly once. A udp_wrap handle needs an explicit
    // recvStart after it is bound; a public dgram socket receives automatically
    // once bound, so we only need to ensure a bind. An outgoing socket that
    // never called bind() auto-binds to an ephemeral port here so getsockname
    // reports the assigned local address.
    startUdpRecv(sock) {
      if (!sock.udp || sock.udpReceiving) return;
      if (sock.udpPublic) {
        if (sock.sport === undefined) {
          var a = sock.udp.bindSync({ address: sock.family === {{{ cDefs.AF_INET6 }}} ? '::' : '0.0.0.0', port: 0 });
          sock.saddr = a.address;
          sock.sport = a.port;
        }
      } else {
        sock.udp.recvStart();
        if (sock.sport === undefined) {
          var name = {};
          if (sock.udp.getsockname(name) === 0) {
            sock.saddr = name.address;
            sock.sport = name.port;
          }
        }
      }
      sock.udpReceiving = true;
      // node only honors these once the socket is bound, so (re)apply any
      // options that were set earlier.
      nodeSockHelpers.applyUdpOptions(sock);
    },
    // Apply the buffered datagram options to a bound UDP socket.
    applyUdpOptions(sock) {
      var h = sock.udp;
      var o = sock.opts;
      if (!h || !o || !sock.udpReceiving) return;
      // libuv's multicast TTL/loopback setters apply to whichever family the
      // socket is, so IP_MULTICAST_TTL/IPV6_MULTICAST_HOPS and the two
      // *_MULTICAST_LOOP options funnel through the same handle methods.
      var mcastTtl = o.multicastTtl ?? o.multicastHops;
      var mcastLoop = o.multicastLoop6 ?? o.multicastLoop;
      if (sock.udpPublic) {
        if (o.ttl !== undefined) { try { h.setTTL(o.ttl); } catch (e) {} }
        if (o.broadcast !== undefined) { try { h.setBroadcast(!!o.broadcast); } catch (e) {} }
        if (o.recvBuf !== undefined) { try { h.setRecvBufferSize(o.recvBuf); } catch (e) {} }
        if (o.sendBuf !== undefined) { try { h.setSendBufferSize(o.sendBuf); } catch (e) {} }
        if (mcastTtl !== undefined) { try { h.setMulticastTTL(mcastTtl); } catch (e) {} }
        if (mcastLoop !== undefined) { try { h.setMulticastLoopback(!!mcastLoop); } catch (e) {} }
      } else {
        if (o.ttl !== undefined) { try { h.setTTL(o.ttl); } catch (e) {} }
        if (o.broadcast !== undefined) { try { h.setBroadcast(o.broadcast ? 1 : 0); } catch (e) {} }
        if (o.recvBuf !== undefined) { try { h.bufferSize(o.recvBuf, true, {}); } catch (e) {} }
        if (o.sendBuf !== undefined) { try { h.bufferSize(o.sendBuf, false, {}); } catch (e) {} }
        if (mcastTtl !== undefined) { try { h.setMulticastTTL(mcastTtl); } catch (e) {} }
        if (mcastLoop !== undefined) { try { h.setMulticastLoopback(mcastLoop ? 1 : 0); } catch (e) {} }
      }
    },
    // The live OS buffer size from a bound UDP socket, or undefined.
    udpBufferSize(sock, recv) {
      if (!sock.udp || !sock.udpReceiving) return undefined;
      try {
        if (sock.udpPublic) return recv ? sock.udp.getRecvBufferSize() : sock.udp.getSendBufferSize();
        return sock.udp.bufferSize(0, recv, {});
      } catch (e) {}
    },
    // Replay buffered opts once the socket is live.
    applyOptions(sock) {
      var conn = sock.connection;
      var o = sock.opts;
      if (!conn || !o) return;
      if (o.noDelay !== undefined) {
        try { conn.setNoDelay(!!o.noDelay); } catch (e) {}
      }
      nodeSockHelpers.applyKeepAlive(sock);
    },
    // The keepalive tunables arrive from C in seconds, but node wants
    // milliseconds, so we scale by 1000. A non-positive value keeps node's
    // default for that field.
    applyKeepAlive(sock) {
      var conn = sock.connection;
      var o = sock.opts;
      if (!conn || !o || o.keepAlive === undefined) return;
      try {
        conn.setKeepAlive(
          !!o.keepAlive,
          (o.keepAliveIdle || 0) * 1000,
          (o.keepAliveIntvl || 0) * 1000,
          o.keepAliveCnt || 0);
      } catch (e) {}
    },
    // Forward a connected node socket's events onto sock.
    wireConnection(sock, conn) {
      sock.connection = conn;
      conn.on('data', (buf) => {
        var data = new Uint8Array(buf.length);
        data.set(buf);
        sock.recv_queue.push({ addr: sock.daddr, port: sock.dport, data });
        sock.recv_bytes = (sock.recv_bytes || 0) + data.length;
        // If the peer outruns the reader, pause node and resume in recvmsg.
        if (sock.recv_bytes >= 262144 /* 256 KiB */) {
          try { conn.pause(); } catch (e) {}
          sock.paused = true;
        }
        SOCKFS.emit('message', sock.stream.fd);
      });
      // A peer FIN surfaces as EOF to the reader.
      conn.on('end', () => {
        sock.readClosed = true;
        SOCKFS.emit('message', sock.stream.fd);
      });
      conn.on('close', () => {
        sock.readClosed = true;
        sock.state = {{{ SOCK_STATE_CLOSED }}};
        SOCKFS.emit('close', sock.stream.fd);
      });
      // Backpressure relieved, so we are writable again.
      conn.on('drain', () => {
        sock.writeBlocked = false;
        SOCKFS.emit('open', sock.stream.fd);
      });
      conn.on('error', (e) => {
        sock.error = nodeSockHelpers.nodeErrToErrno(e);
        // Let a failed connect resolve so SO_ERROR can be read.
        if (sock.state === {{{ SOCK_STATE_CONNECTING }}}) sock.state = {{{ SOCK_STATE_CONNECTED }}};
        SOCKFS.emit('error', [sock.stream.fd, sock.error, (e && e.message) || 'socket error']);
      });
    },
  },
  $nodeSockOps__deps: ['$nodeSockHelpers', '$SOCKFS', '$ERRNO_CODES'],
  $nodeSockOps__postset: `
    if (!ENVIRONMENT_IS_NODE) {
      throw new Error('NODERAWSOCKETS is currently only supported on Node.js environment.')
    }`,
  $nodeSockOps: {
    poll(sock) {
      // A listener is readable when a connection is waiting to be accepted.
      if (sock.server) {
        return sock.pending.length ? ({{{ cDefs.POLLRDNORM }}} | {{{ cDefs.POLLIN }}}) : 0;
      }
      // UDP is connectionless: always writable, readable when a datagram waits.
      if (sock.type === {{{ cDefs.SOCK_DGRAM }}}) {
        var dmask = {{{ cDefs.POLLOUT }}};
        if (sock.recv_queue.length || sock.error) dmask |= ({{{ cDefs.POLLRDNORM }}} | {{{ cDefs.POLLIN }}});
        return dmask;
      }
      var mask = 0;
      if (sock.recv_queue.length || sock.readClosed || sock.error) {
        mask |= ({{{ cDefs.POLLRDNORM }}} | {{{ cDefs.POLLIN }}});
      }
      if (sock.error) {
        // A pending socket error (e.g. a refused connect) is Linux's
        // POLLERR|POLLHUP, plus writable so SO_ERROR can be read. POLLOUT|POLLERR
        // also satisfies epoll's is_write_closed() mapping.
        mask |= {{{ cDefs.POLLOUT }}} | {{{ cDefs.POLLERR }}} | {{{ cDefs.POLLHUP }}};
      } else if (sock.connection && sock.state === {{{ SOCK_STATE_CONNECTED }}} && !sock.writeBlocked) {
        mask |= {{{ cDefs.POLLOUT }}};
      }
      // A peer FIN / read-side hangup (recv will see EOF) is POLLRDHUP. POLLHUP
      // means both halves are hung up: either the connection is fully closed, or
      // we locally shut down both directions (shutdown(SHUT_RDWR)), which Linux
      // epoll reports as a hangup even though the node connection is still live.
      if (sock.readClosed) mask |= {{{ cDefs.POLLRDHUP }}};
      if (sock.state === {{{ SOCK_STATE_CLOSED }}} || (sock.readClosed && sock.writeShutdown)) {
        mask |= {{{ cDefs.POLLHUP }}};
      }
      return mask;
    },
    ioctl(sock, request, arg) {
      switch (request) {
        case {{{ cDefs.FIONREAD }}}:
          var bytes = sock.recv_queue.length ? sock.recv_queue[0].data.length : 0;
          {{{ makeSetValue('arg', '0', 'bytes', 'i32') }}};
          return 0;
        case {{{ cDefs.FIONBIO }}}:
          var on = {{{ makeGetValue('arg', '0', 'i32') }}};
          if (on) sock.stream.flags |= {{{ cDefs.O_NONBLOCK }}};
          else sock.stream.flags &= ~{{{ cDefs.O_NONBLOCK }}};
          return 0;
        default:
          return {{{ cDefs.EINVAL }}};
      }
    },
    close(sock) {
      sock.state = {{{ SOCK_STATE_CLOSED }}};
      if (sock.udp) {
        try {
          if (sock.udpPublic) sock.udp.close();
          else { sock.udp.recvStop(); sock.udp.close(); }
        } catch (e) {}
        sock.udp = null;
      }
      if (sock.server) { try { sock.server.close(); } catch (e) {} sock.server = null; }
      if (sock.connection) {
        var conn = sock.connection;
        var linger = sock.opts?.linger;
        if (linger?.onoff && linger.linger === 0 && conn.resetAndDestroy) {
          // SO_LINGER with a zero timeout: abortive close - send RST and
          // discard any unsent data.
          conn.resetAndDestroy();
        } else if (linger?.onoff && linger.linger > 0) {
          // Positive timeout: flush gracefully, but node has no blocking
          // close, so force the connection down once the interval elapses.
          conn.end();
          var timer = setTimeout(() => conn.destroy(), linger.linger * 1000);
          timer.unref?.();
        } else {
          conn.destroy();
        }
        sock.connection = null;
      }
      // A bound handle that was never adopted by listen()/connect() is ours to
      // release; once adopted the server/connection owns it.
      if (sock.bound && !sock.server && !sock.connection) {
        try { sock.bound.close(); } catch (e) {}
      }
      sock.bound = null;
      return 0;
    },
    // how: SHUT_RD 0, SHUT_WR 1, SHUT_RDWR 2 (musl sys/socket.h).
    shutdown(sock, how) {
      if (!sock.connection) throw new FS.ErrnoError({{{ cDefs.ENOTCONN }}});
      if (how === 0 || how === 2) {
        // No more reads: subsequent recv returns EOF.
        sock.readClosed = true;
      }
      if (how === 1 || how === 2) {
        // Half-close the write side (sends FIN); later sends fail with EPIPE.
        sock.writeShutdown = true;
        try { sock.connection.end(); } catch (e) {}
      }
      SOCKFS.emit('message', sock.stream.fd);
      return 0;
    },
    bind(sock, addr, port) {
      if (sock.saddr !== undefined || sock.sport !== undefined) {
        throw new FS.ErrnoError({{{ cDefs.EINVAL }}}); // already bound
      }
      if (sock.type === {{{ cDefs.SOCK_DGRAM }}}) {
        var udp = nodeSockHelpers.ensureUdpHandle(sock);
        if (sock.udpPublic) {
          var a;
          // bindSync throws synchronously (e.g. EADDRINUSE) and returns the
          // bound address, including the OS-assigned port for port 0.
          try { a = udp.bindSync({ address: addr, port }); }
          catch (e) { throw new FS.ErrnoError(nodeSockHelpers.nodeErrToErrno(e)); }
          sock.saddr = a.address;
          sock.sport = a.port;
        } else {
          var ucode = addr.includes(':') ? udp.bind6(addr, port, 0) : udp.bind(addr, port, 0);
          if (ucode) throw new FS.ErrnoError(nodeSockHelpers.codeToErrno(ucode));
          var uname = {};
          ucode = udp.getsockname(uname);
          if (ucode) throw new FS.ErrnoError(nodeSockHelpers.codeToErrno(ucode));
          sock.saddr = uname.address;
          sock.sport = uname.port;
        }
        sock.state = {{{ SOCK_STATE_BOUND }}};
        nodeSockHelpers.startUdpRecv(sock);
        return;
      }
      // TCP binds eagerly and synchronously: the kernel-assigned port (even for
      // a bind(:0)) is known immediately, getsockname() needs no promotion, and a
      // conflict surfaces right here as EADDRINUSE.
      nodeSockHelpers.bindHandle(sock, addr, port);
      sock.state = {{{ SOCK_STATE_BOUND }}};
    },
    connect(sock, addr, port) {
      if (sock.type === {{{ cDefs.SOCK_DGRAM }}}) {
        sock.daddr = addr;
        sock.dport = port;
        var udp = nodeSockHelpers.ensureUdpHandle(sock);
        if (sock.udpPublic) {
          // Real kernel connect: the OS filters non-peer datagrams and reports
          // async errors (e.g. ICMP ECONNREFUSED) on the socket. connectSync
          // binds first if needed and throws synchronously; a re-connect just
          // replaces the peer.
          if (sock.udpConnected) { try { udp.disconnect(); } catch (e) {} }
          try { udp.connectSync(port, addr); }
          catch (e) { throw new FS.ErrnoError(nodeSockHelpers.nodeErrToErrno(e)); }
          sock.udpConnected = true;
          var a = udp.address();
          sock.saddr = a.address;
          sock.sport = a.port;
          sock.udpReceiving = true; // a bound dgram socket already receives
          nodeSockHelpers.applyUdpOptions(sock);
          return;
        }
        // Older node has no synchronous dgram connect, so just record the peer
        // and enforce it in JS (see udpDeliver and sendmsg); replies arrive once
        // the socket is bound (an explicit bind or the auto-bind on first send).
        return;
      }
      if (sock.server) throw new FS.ErrnoError({{{ cDefs.EOPNOTSUPP }}});
      if (sock.connection) {
        throw new FS.ErrnoError(sock.state === {{{ SOCK_STATE_CONNECTING }}} ? {{{ cDefs.EALREADY }}} : {{{ cDefs.EISCONN }}});
      }
      sock.daddr = addr;
      sock.dport = port;
      sock.state = {{{ SOCK_STATE_CONNECTING }}};
      var net = nodeSockHelpers.getNet();
      var conn;
      if (sock.bound) {
        // A prior bind() produced a real, already-bound handle; connect through
        // it so the bound source address/port is honored by the kernel.
        conn = new net.Socket({ handle: sock.bound, pauseOnCreate: true, allowHalfOpen: true });
      } else {
        // Unbound client: let the kernel assign the source address/port.
        conn = new net.Socket({ allowHalfOpen: true });
      }
      conn.once('connect', () => {
        sock.state = {{{ SOCK_STATE_CONNECTED }}};
        sock.saddr = conn.localAddress;
        sock.sport = conn.localPort;
        sock.daddr = conn.remoteAddress || addr;
        sock.dport = conn.remotePort || port;
        try { conn.resume(); } catch (e) {}
        nodeSockHelpers.applyOptions(sock);
        SOCKFS.emit('open', sock.stream.fd);
      });
      nodeSockHelpers.wireConnection(sock, conn);
      conn.connect({ host: addr, port, lookup: nodeSockHelpers.noLookup });
    },
    listen(sock, backlog) {
      if (sock.type !== {{{ cDefs.SOCK_STREAM }}}) throw new FS.ErrnoError({{{ cDefs.EOPNOTSUPP }}}); // not a stream socket
      if (sock.server) throw new FS.ErrnoError({{{ cDefs.EINVAL }}}); // already listening
      if (sock.connection) throw new FS.ErrnoError({{{ cDefs.EINVAL }}}); // a connected socket cannot listen
      // POSIX listen without a prior bind auto-binds an ephemeral port. The bind
      // is eager and synchronous (bindHandle), so the assigned port is known and
      // any conflict surfaces before we listen.
      if (!sock.bound) {
        nodeSockHelpers.bindHandle(sock, '0.0.0.0', 0);
        sock.state = {{{ SOCK_STATE_BOUND }}};
      }
      var server = new (nodeSockHelpers.getNet().Server)({ pauseOnConnect: true, allowHalfOpen: true });
      sock.server = server;
      sock.state = {{{ SOCK_STATE_LISTEN }}};
      server.on('connection', (conn) => {
        var newsock = SOCKFS.createSocket(sock.family, sock.type, sock.protocol);
        newsock.state = {{{ SOCK_STATE_CONNECTED }}};
        newsock.saddr = conn.localAddress;
        newsock.sport = conn.localPort;
        newsock.daddr = conn.remoteAddress;
        newsock.dport = conn.remotePort;
        nodeSockHelpers.wireConnection(newsock, conn);
        try { conn.resume(); } catch (e) {} // paused by pauseOnConnect
        sock.pending.push(newsock);
        SOCKFS.emit('connection', newsock.stream.fd);
        // A queued client makes the listening socket readable (POLLIN).
        sock.stream.node.notifyListeners({{{ cDefs.POLLRDNORM }}} | {{{ cDefs.POLLIN }}});
      });
      server.on('error', (e) => {
        sock.error = nodeSockHelpers.nodeErrToErrno(e);
        SOCKFS.emit('error', [sock.stream.fd, sock.error, (e && e.message) || 'listen error']);
      });
      // listen on the already-bound handle: accept would-blocks until a
      // connection arrives, surfaced through poll/accept.
      server.listen(sock.bound, backlog || 511);
    },
    accept(listensock) {
      if (!listensock.server) throw new FS.ErrnoError({{{ cDefs.EINVAL }}});
      // Surface a real listen error (e.g. late address-in-use) rather than
      // masking it as would-block.
      if (listensock.error) {
        var e = listensock.error;
        listensock.error = null;
        throw new FS.ErrnoError(e);
      }
      if (!listensock.pending.length) throw new FS.ErrnoError({{{ cDefs.EAGAIN }}});
      var newsock = listensock.pending.shift();
      newsock.stream.flags = listensock.stream.flags;
      return newsock;
    },
    sendmsg(sock, buffer, offset, length, addr, port) {
      if (sock.type === {{{ cDefs.SOCK_DGRAM }}}) {
        // A connected datagram socket rejects an explicit destination.
        if (sock.daddr !== undefined && addr !== undefined) {
          throw new FS.ErrnoError({{{ cDefs.EISCONN }}});
        }
        if (addr === undefined || port === undefined) {
          addr = sock.daddr;
          port = sock.dport;
          if (addr === undefined || port === undefined) throw new FS.ErrnoError({{{ cDefs.EDESTADDRREQ }}});
        }
        var handle = nodeSockHelpers.ensureUdpHandle(sock);
        // A public dgram send() would do an async implicit bind, so bind (and
        // start receiving) synchronously up front; udp_wrap auto-binds on send,
        // so it starts receiving afterwards.
        if (sock.udpPublic) nodeSockHelpers.startUdpRecv(sock);
        offset += buffer.byteOffset;
        buffer = buffer.buffer;
        // Copy out of (possibly shared) wasm memory: the datagram must stay
        // stable until the asynchronous send completes.
        var msg = Buffer.from(buffer.slice(offset, offset + length));
        if (sock.udpPublic) {
          // Async errors surface on the 'error' event (read via SO_ERROR). A
          // real-connected socket sends to its kernel peer with no address.
          if (sock.udpConnected) handle.send(msg);
          else handle.send(msg, port, addr);
        } else {
          var code = addr.includes(':')
            ? handle.send6(new sock.sendWrap(), [msg], 1, port, addr, false)
            : handle.send(new sock.sendWrap(), [msg], 1, port, addr, false);
          if (code < 0) throw new FS.ErrnoError(nodeSockHelpers.codeToErrno(code));
          // The send auto-bound an unbound socket, so replies can be received.
          nodeSockHelpers.startUdpRecv(sock);
        }
        return length;
      }
      // Writing after a write-shutdown is a broken pipe, regardless of peer.
      if (sock.writeShutdown) {
        throw new FS.ErrnoError({{{ cDefs.EPIPE }}});
      }
      var conn = sock.connection;
      if (!conn || sock.state === {{{ SOCK_STATE_CLOSED }}}) {
        throw new FS.ErrnoError({{{ cDefs.ENOTCONN }}});
      }
      // Bound node's write buffer to its high-water mark: a non-blocking socket
      // only accepts up to the remaining headroom, would-blocking when there is
      // none, and short-writes the rest (which POSIX send() is allowed to do).
      if (sock.stream.flags & {{{ cDefs.O_NONBLOCK }}}) {
        var headroom = conn.writableHighWaterMark - conn.writableLength;
        if (headroom <= 0) throw new FS.ErrnoError({{{ cDefs.EAGAIN }}});
        if (length > headroom) length = headroom;
      }
      offset += buffer.byteOffset;
      buffer = buffer.buffer;
      var data = new Uint8Array(buffer.slice(offset, offset + length));
      var ok;
      try {
        ok = conn.write(data);
      } catch (e) {
        throw new FS.ErrnoError(nodeSockHelpers.nodeErrToErrno(e));
      }
      if (!ok) sock.writeBlocked = true; // cleared on 'drain', gates poll's POLLOUT
      return length;
    },
    recvmsg(sock, length, flags) {
      // MSG_PEEK returns the data from the head of the queue without consuming
      // it: no shift, no recv_bytes/flow-control adjustment, so a later recv
      // sees the same bytes and poll still reports the socket readable.
      var peek = flags & {{{ cDefs.MSG_PEEK }}};
      if (sock.type === {{{ cDefs.SOCK_DGRAM }}}) {
        var dgram = sock.recv_queue[0];
        if (!dgram) {
          // poll reports the socket readable on a pending error, so surface
          // (and clear) it here rather than spinning on EAGAIN.
          if (sock.error) {
            var derr = sock.error;
            sock.error = null;
            throw new FS.ErrnoError(derr);
          }
          throw new FS.ErrnoError({{{ cDefs.EAGAIN }}});
        }
        // A datagram is atomic: return up to length bytes and drop the rest.
        var dd = dgram.data;
        var res = { buffer: dd.subarray(0, Math.min(length, dd.length)), addr: dgram.addr, port: dgram.port };
        if (!peek) sock.recv_queue.shift();
        return res;
      }
      var queued = sock.recv_queue[0];
      if (!queued) {
        if (sock.readClosed) return null; // EOF
        if (!sock.connection) {
          throw new FS.ErrnoError({{{ cDefs.ENOTCONN }}});
        }
        throw new FS.ErrnoError({{{ cDefs.EAGAIN }}});
      }
      var q = queued.data;
      var bytesRead = Math.min(length, q.length);
      var res = { buffer: q.subarray(0, bytesRead), addr: queued.addr, port: queued.port };
      if (peek) return res;
      sock.recv_queue.shift();
      if (bytesRead < q.length) {
        queued.data = q.subarray(bytesRead);
        sock.recv_queue.unshift(queued);
      }
      sock.recv_bytes = Math.max(0, (sock.recv_bytes || 0) - bytesRead);
      if (sock.paused && sock.recv_bytes < 262144 && sock.connection) {
        sock.paused = false;
        try { sock.connection.resume(); } catch (e) {}
      }
      return res;
    },
    setsockopt(sock, level, optname, optval, optlen) {
      sock.opts ||= {};
      var val = {{{ makeGetValue('optval', 0, 'i32') }}};
      if (level === {{{ cDefs.SOL_SOCKET }}}) {
        switch (optname) {
          case 9: // SO_KEEPALIVE
            sock.opts.keepAlive = !!val;
            nodeSockHelpers.applyKeepAlive(sock);
            return 0;
          case 8: // SO_RCVBUF. Applied to the udp_wrap handle; Node TCP cannot.
            sock.opts.recvBuf = val;
            nodeSockHelpers.applyUdpOptions(sock);
            return 0;
          case 7: // SO_SNDBUF. Applied to the udp_wrap handle; Node TCP cannot.
            sock.opts.sendBuf = val;
            nodeSockHelpers.applyUdpOptions(sock);
            return 0;
          case 6: // SO_BROADCAST (datagram sockets)
            sock.opts.broadcast = !!val;
            nodeSockHelpers.applyUdpOptions(sock);
            return 0;
          case 2: // SO_REUSEADDR. libuv forces SO_REUSEADDR on at bind, so this
            // is effectively always enabled; accept and ignore (getsockopt
            // reports 1). It cannot be turned off.
            return 0;
          case {{{ cDefs.SO_REUSEPORT }}}: // SO_REUSEPORT. Bind-time: cached and
            // passed to the BoundSocket at bind. Set after bind has no effect.
            sock.opts.reusePort = !!val;
            return 0;
          case 13: // SO_LINGER (struct linger: l_onoff, l_linger)
            sock.opts.linger = {
              onoff: val,
              linger: {{{ makeGetValue('optval', 4, 'i32') }}},
            };
            return 0;
        }
      } else if (level === {{{ cDefs.IPPROTO_IP }}}) {
        switch (optname) {
          case 2: // IP_TTL
            sock.opts.ttl = val;
            nodeSockHelpers.applyUdpOptions(sock);
            return 0;
          case 33: // IP_MULTICAST_TTL
            sock.opts.multicastTtl = val;
            nodeSockHelpers.applyUdpOptions(sock);
            return 0;
          case 34: // IP_MULTICAST_LOOP
            sock.opts.multicastLoop = !!val;
            nodeSockHelpers.applyUdpOptions(sock);
            return 0;
        }
      } else if (level === {{{ cDefs.IPPROTO_IPV6 }}}) {
        switch (optname) {
          case {{{ cDefs.IPV6_V6ONLY }}}:
            // Bind-time only: IPV6_V6ONLY cannot change once the socket is bound,
            // so reject a late change (POSIX returns EINVAL). Before any
            // bind/connect/listen we cache it for the BoundSocket constructor.
            if (sock.state) return -{{{ cDefs.EINVAL }}};
            sock.opts.ipv6Only = !!val;
            return 0;
          case 18: // IPV6_MULTICAST_HOPS
            sock.opts.multicastHops = val;
            nodeSockHelpers.applyUdpOptions(sock);
            return 0;
          case 19: // IPV6_MULTICAST_LOOP
            sock.opts.multicastLoop6 = !!val;
            nodeSockHelpers.applyUdpOptions(sock);
            return 0;
        }
      } else if (level === {{{ cDefs.IPPROTO_TCP }}}) {
        switch (optname) {
          case 1: // TCP_NODELAY
            sock.opts.noDelay = !!val;
            if (sock.connection) { try { sock.connection.setNoDelay(!!val); } catch (e) {} }
            return 0;
          case 4: // TCP_KEEPIDLE (seconds)
            sock.opts.keepAliveIdle = val;
            nodeSockHelpers.applyKeepAlive(sock);
            return 0;
          case 5: // TCP_KEEPINTVL (seconds)
            sock.opts.keepAliveIntvl = val;
            nodeSockHelpers.applyKeepAlive(sock);
            return 0;
          case 6: // TCP_KEEPCNT (probe count)
            sock.opts.keepAliveCnt = val;
            nodeSockHelpers.applyKeepAlive(sock);
            return 0;
        }
      }
      // Accept unknown options silently, like a permissive stack.
      return 0;
    },
    getsockopt(sock, level, optname, optval, optlen) {
      sock.opts ||= {};
      var val;
      if (level === {{{ cDefs.SOL_SOCKET }}}) {
        switch (optname) {
          case {{{ cDefs.SO_ERROR }}}:
            {{{ makeSetValue('optval', 0, 'sock.error || 0', 'i32') }}};
            {{{ makeSetValue('optlen', 0, 4, 'i32') }}};
            sock.error = null; // SO_ERROR reads and clears
            return 0;
          case 3: val = sock.type; break; // SO_TYPE
          case 13: { // SO_LINGER (struct linger: l_onoff, l_linger)
            var linger = sock.opts.linger || { onoff: 0, linger: 0 };
            {{{ makeSetValue('optval', 0, 'linger.onoff', 'i32') }}};
            {{{ makeSetValue('optval', 4, 'linger.linger', 'i32') }}};
            {{{ makeSetValue('optlen', 0, 8, 'i32') }}};
            return 0;
          }
          case 9: val = sock.opts.keepAlive ? 1 : 0; break; // SO_KEEPALIVE
          // SO_RCVBUF/SO_SNDBUF: report the live value from the udp_wrap handle
          // when bound, else the stored/default.
          case 8: val = nodeSockHelpers.udpBufferSize(sock, true) ?? (sock.opts.recvBuf || 65536); break;
          case 7: val = nodeSockHelpers.udpBufferSize(sock, false) ?? (sock.opts.sendBuf || 65536); break;
          case 6: val = sock.opts.broadcast ? 1 : 0; break; // SO_BROADCAST
          case 2: val = 1; break; // SO_REUSEADDR: libuv forces it on at bind
          case {{{ cDefs.SO_REUSEPORT }}}: val = sock.opts.reusePort ? 1 : 0; break;
          default: return -{{{ cDefs.ENOPROTOOPT }}};
        }
      } else if (level === {{{ cDefs.IPPROTO_IP }}}) {
        switch (optname) {
          case 2: val = sock.opts.ttl || 64; break; // IP_TTL
          case 33: val = sock.opts.multicastTtl ?? 1; break; // IP_MULTICAST_TTL
          case 34: val = sock.opts.multicastLoop === undefined ? 1 : (sock.opts.multicastLoop ? 1 : 0); break; // IP_MULTICAST_LOOP
          default: return -{{{ cDefs.ENOPROTOOPT }}};
        }
      } else if (level === {{{ cDefs.IPPROTO_IPV6 }}}) {
        switch (optname) {
          case {{{ cDefs.IPV6_V6ONLY }}}: val = sock.opts.ipv6Only ? 1 : 0; break;
          case 18: val = sock.opts.multicastHops ?? 1; break; // IPV6_MULTICAST_HOPS
          case 19: val = sock.opts.multicastLoop6 === undefined ? 1 : (sock.opts.multicastLoop6 ? 1 : 0); break; // IPV6_MULTICAST_LOOP
          default: return -{{{ cDefs.ENOPROTOOPT }}};
        }
      } else if (level === {{{ cDefs.IPPROTO_TCP }}}) {
        switch (optname) {
          // TCP_MAXSEG: node exposes no MSS, so report RFC 879's 536-byte default
          // before the handshake and the (large) loopback-negotiated value once
          // connected. Enough for callers that only compare pre/post-connect MSS.
          case 2: val = (sock.state === {{{ SOCK_STATE_CONNECTED }}}) ? 65483 : 536; break;
          case 1: val = sock.opts.noDelay ? 1 : 0; break;    // TCP_NODELAY
          case 4: val = sock.opts.keepAliveIdle || 0; break; // TCP_KEEPIDLE
          case 5: val = sock.opts.keepAliveIntvl || 0; break;// TCP_KEEPINTVL
          case 6: val = sock.opts.keepAliveCnt || 0; break;  // TCP_KEEPCNT
          default: return -{{{ cDefs.ENOPROTOOPT }}};
        }
      } else {
        return -{{{ cDefs.ENOPROTOOPT }}};
      }
      {{{ makeSetValue('optval', 0, 'val', 'i32') }}};
      {{{ makeSetValue('optlen', 0, 4, 'i32') }}};
      return 0;
    }
  },
};

addToLibrary(NodeSockFSLibrary);
