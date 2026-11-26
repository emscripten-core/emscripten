/**
 * @license
 * Copyright 2026 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// TCP over WebTransport (-sWEBTRANSPORT_SOCKETS). 
//
// Each connected stream socket is one WebTransport session, carrying a single 
// bidirectional stream. That stream is a reliable, ordered byte stream, like TCP. 
// Closing our write side (shutdown(SHUT_WR) or close()) sends a FIN on the 
// stream, and the end of the stream's readable side is the peer's FIN.
//
// Unlike a WebSocket, a WebTransport stream has flow control in both
// directions, and this backend passes it on:
//
// - Sending: data handed to the stream's writer counts as buffered until its
//   write() resolves. A socket accepts at most WEBTRANSPORT_SEND_BUFFER_SIZE
//   buffered bytes: send() writes up to the remaining headroom and fails with
//   EAGAIN when there is none, and poll() only reports POLLOUT while there is
//   headroom.
//
// - Receiving: the stream is only read while fewer than
//   WEBTRANSPORT_RECV_BUFFER_SIZE received bytes wait to be recv()'d. Once the
//   application stops reading, QUIC flow control slows the peer down, instead
//   of received data piling up without limit.
//
// Only non-blocking sockets are supported: JS can't wait for the network inside
// a call, so a blocking connect(), send() or recv() could only pretend to. They
// fail with EOPNOTSUPP instead (with a warning in ASSERTIONS builds), and the
// application waits with poll(), select() or epoll.
//
// listen() and accept() are not supported, nor are datagram sockets.

var WebTransportSockFSLibrary = {
  ...defineInternalConsts({
    $WEBTRANSPORT_STATE_CONNECTING: 1,
    $WEBTRANSPORT_STATE_CONNECTED: 2,
    $WEBTRANSPORT_STATE_CLOSED: 3,
    // How long close() waits for the peer to finish the stream before closing
    // the session, so that data still in flight isn't discarded.
    $WEBTRANSPORT_LINGER_MS: 30000,
  }),

  $webTransportSockHelpers: {
    url(addr, port) {
      // The replace is needed because the compiler replaces '//' comments with
      // '#' comments without checking context (see the WebSocket backend).
      var url = '{{{ WEBTRANSPORT_URL }}}'.replace('#', '//');
#if expectToReceiveOnModule('webtransport')
      url = Module['webtransport']?.['url'] ?? url;
#endif
      if (typeof url == 'function') {
        return url(addr, port);
      }
      // A URL that is only a scheme is completed with the address and port.
      return url === 'https://' ? `${url}${addr.includes(':') ? `[${addr}]` : addr}:${port}/` : url;
    },
    // Emits a readiness event, unless the socket has been closed: its fd may
    // already belong to another socket.
    emit(sock, event, param = sock.stream.fd) {
#if SOCKET_DEBUG
      dbg(`webtransport: ${event} fd = ${sock.stream.fd}${sock.closed ? ' (closed, not emitted)' : ''}`);
#endif
      if (!sock.closed) {
        SOCKFS.emit(event, param);
      }
    },
    fail(sock, errno, message) {
      if (sock.state === WEBTRANSPORT_STATE_CLOSED) {
        return;
      }
      sock.state = WEBTRANSPORT_STATE_CLOSED;
      sock.error = errno;
      sock.readClosed = true;
      webTransportSockHelpers.emit(sock, 'error', [sock.stream.fd, errno, message]);
    },
    // Refuses an operation on a blocking socket, which would have to wait.
    checkNonBlocking(sock) {
      if (!(sock.stream.flags & O_NONBLOCK)) {
#if ASSERTIONS
        warnOnce('WEBTRANSPORT_SOCKETS only supports non-blocking sockets: set O_NONBLOCK (or SOCK_NONBLOCK) and wait with poll(), select() or epoll. Failing with EOPNOTSUPP.');
#endif
        throw new FS.ErrnoError(EOPNOTSUPP);
      }
    },
    sendHeadroom(sock) {
      return {{{ WEBTRANSPORT_SEND_BUFFER_SIZE }}} - sock.sendBuffered;
    },
    write(sock, data) {
      sock.writer.write(data).then(() => {
        var wasFull = webTransportSockHelpers.sendHeadroom(sock) <= 0;
        sock.sendBuffered -= data.length;
        if (wasFull && sock.state === WEBTRANSPORT_STATE_CONNECTED) {
          webTransportSockHelpers.emit(sock, 'open'); // writable again
        }
      }, (e) => webTransportSockHelpers.fail(sock, ECONNRESET, e?.message ?? 'stream write failed'));
    },
    // Reads the stream while there is room in the receive queue. Called again
    // by recvmsg once the application has made room.
    pump(sock) {
      if (sock.reading || sock.readClosed || !sock.reader ||
          sock.recvBuffered >= {{{ WEBTRANSPORT_RECV_BUFFER_SIZE }}}) {
        return;
      }
      sock.reading = true;
      sock.reader.read().then(({value, done}) => {
#if SOCKET_DEBUG
        dbg(`webtransport: read ${done ? 'end of stream' : `${value.length} bytes`}`);
#endif
        sock.reading = false;
        if (sock.readClosed) {
          return; // shut down for reading in the meantime: discard
        }

        if (done) {
          sock.readClosed = true; // the peer's FIN: recv() returns 0
        } else {
          // An empty chunk isn't queued: recv() returning 0 bytes means EOF.
          if (value.length) {
            sock.recv_queue.push({ data: value });
            sock.recvBuffered += value.length;
          }
          webTransportSockHelpers.pump(sock);
        }
        webTransportSockHelpers.emit(sock, 'message');
      }, (e) => {
        sock.reading = false;
        if (sock.state === WEBTRANSPORT_STATE_CLOSED) {
          // The session already ended, without an error or with one that has
          // been recorded: fail() has nothing to add, but recv() must still
          // see the end of the stream rather than EAGAIN forever.
          if (!sock.readClosed) {
            sock.readClosed = true;
            webTransportSockHelpers.emit(sock, 'message');
          }
          return;
        }
        webTransportSockHelpers.fail(sock, ECONNRESET, e?.message ?? 'stream read failed');
      });
    },
    connected(sock, stream) {
      sock.writer = stream.writable.getWriter();
      sock.reader = stream.readable.getReader();
      sock.state = WEBTRANSPORT_STATE_CONNECTED;

      webTransportSockHelpers.emit(sock, 'open');
      webTransportSockHelpers.pump(sock);
    },
    // Closes the session once our data has been delivered and the peer has
    // finished the stream, or after a timeout: closing the session right away
    // would discard data still in flight in either direction.
    release(sock) {
      var session = sock.session;
      sock.session = null;
      if (!session) {
        return;
      }
      var closeSession = () => {
        clearTimeout(timer);
        try {
          session.close();
        } catch (e) {
        }
      };
      var timer = setTimeout(closeSession, WEBTRANSPORT_LINGER_MS);
      timer.unref?.();
      // Our FIN, after the data written before it: settles once the peer has
      // received all of it, or the stream has failed. This has to be waited
      // for even if the peer has finished its side already.
      var flushed = sock.writer &&
        (sock.writeShutdown ? sock.writer.closed : sock.writer.close()).catch(() => {});
      // Discard what the peer still sends until it finishes the stream. A read
      // that pump still has outstanding is answered first, and discarded too.
      var drained;
      if (sock.reader && (!sock.readClosed || sock.reading)) {
        sock.readClosed = true;
        drained = new Promise((resolve) => {
          var drain = () => sock.reader.read().then(({done}) => done ? resolve() : drain(), resolve);
          drain();
        });
      }
      Promise.all([flushed, drained]).then(closeSession);
    },
  },

  $webTransportSockOps: {
    poll(sock) {
      var mask = 0;
      if (sock.recv_queue.length || sock.readClosed || sock.error) {
        mask |= POLLRDNORM | POLLIN;
      }
      if (sock.error) {
        // A pending error (e.g. a refused connect) is POLLERR|POLLHUP, plus
        // writable so that SO_ERROR can be read.
        mask |= POLLOUT | POLLERR | POLLHUP;
      } else if (sock.state === WEBTRANSPORT_STATE_CONNECTED &&
                 webTransportSockHelpers.sendHeadroom(sock) > 0) {
        mask |= POLLOUT;
      }
      if (sock.readClosed) {
        mask |= POLLRDHUP;
      }
      if (sock.state === WEBTRANSPORT_STATE_CLOSED || (sock.readClosed && sock.writeShutdown)) {
        mask |= POLLHUP;
      }
      return mask;
    },
    ioctl(sock, request, arg) {
      switch (request) {
        case FIONREAD:
          var bytes = sock.recvBuffered || 0;
          {{{ makeSetValue('arg', '0', 'bytes', 'i32') }}};
          return 0;
        case FIONBIO:
          var on = {{{ makeGetValue('arg', '0', 'i32') }}};
          if (on) sock.stream.flags |= O_NONBLOCK;
          else sock.stream.flags &= ~O_NONBLOCK;
          return 0;
        default:
          return EINVAL;
      }
    },
    close(sock) {
      sock.closed = true;
      sock.state = WEBTRANSPORT_STATE_CLOSED;
      webTransportSockHelpers.release(sock);
      
      return 0;
    },
    // how: SHUT_RD 0, SHUT_WR 1, SHUT_RDWR 2 (musl sys/socket.h).
    shutdown(sock, how) {
      if (!sock.reader) { // never connected, or still connecting
        throw new FS.ErrnoError(ENOTCONN);
      }
      if (!how || how === 2) {
        // No more reads: recv() returns EOF, and the stream isn't read anymore.
        sock.readClosed = true;
        sock.recv_queue = [];
        sock.recvBuffered = 0;
      }
      if ((how === 1 || how === 2) && !sock.writeShutdown) {
        // Half-close the write side (sends FIN once the queued data is
        // written); later sends fail with EPIPE.
        sock.writeShutdown = true;
        sock.writer.close().catch(() => {});
      }
      webTransportSockHelpers.emit(sock, 'message');
      return 0;
    },
    bind(sock, addr, port) {
      if (sock.saddr !== undefined || sock.sport !== undefined) {
        throw new FS.ErrnoError(EINVAL); // already bound
      }
      // There is no local address to bind to: just record the name.
      sock.saddr = addr;
      sock.sport = port;
    },
    connect(sock, addr, port) {
      webTransportSockHelpers.checkNonBlocking(sock);
      if (sock.session) {
        throw new FS.ErrnoError(sock.state === WEBTRANSPORT_STATE_CONNECTING ? EALREADY : EISCONN);
      }
      if (typeof WebTransport == 'undefined') {
        throw new FS.ErrnoError(ENETUNREACH);
      }
      var session;
      try {
#if expectToReceiveOnModule('webtransport')
        session = new WebTransport(webTransportSockHelpers.url(addr, port), Module['webtransport']?.['options']);
#else
        session = new WebTransport(webTransportSockHelpers.url(addr, port));
#endif
      } catch (e) {
        throw new FS.ErrnoError(EHOSTUNREACH);
      }
      
      sock.session = session;
      sock.daddr = addr;
      sock.dport = port;
      sock.state = WEBTRANSPORT_STATE_CONNECTING;
      sock.sendBuffered = 0;
      sock.recvBuffered = 0;

      session.ready.then(
        () => session.createBidirectionalStream()).then((stream) => {
          if (sock.state === WEBTRANSPORT_STATE_CONNECTING) {
            webTransportSockHelpers.connected(sock, stream);
          }
        }, 
        (e) => {
          webTransportSockHelpers.fail(sock, ECONNREFUSED, e?.message ?? 'connection refused')
        }
      );
      
      session.closed.then(
        () => {
          // The session ended without an error: both directions are done. A read
          // that pump still has outstanding reports the end of the stream itself,
          // after any data before it.
          if (sock.state !== WEBTRANSPORT_STATE_CONNECTED) {
            return;
          }

          sock.state = WEBTRANSPORT_STATE_CLOSED;
          if (!sock.reading) {
            sock.readClosed = true;
          }
          
          webTransportSockHelpers.emit(sock, 'close');
        }, 
        (e) => {
          webTransportSockHelpers.fail(sock, sock.state === WEBTRANSPORT_STATE_CONNECTING ? ECONNREFUSED : ECONNRESET, e?.message ?? 'session closed')
        }
      );
      
      // Connecting is asynchronous: completion shows up as POLLOUT, or as an
      // error in SO_ERROR.
      throw new FS.ErrnoError(EINPROGRESS);
    },
    listen(sock, backlog) {
      throw new FS.ErrnoError(EOPNOTSUPP);
    },
    accept(listensock) {
      throw new FS.ErrnoError(EOPNOTSUPP);
    },
    sendmsg(sock, buffer, offset, length, addr, port) {
      webTransportSockHelpers.checkNonBlocking(sock);

      if (sock.writeShutdown) {
        throw new FS.ErrnoError(EPIPE);
      }

      if (sock.error) {
        var err = sock.error;
        sock.error = null;
        throw new FS.ErrnoError(err);
      }
      
      if (sock.state === WEBTRANSPORT_STATE_CLOSED) {
        throw new FS.ErrnoError(EPIPE);
      }
      
      if (!sock.session) {
        throw new FS.ErrnoError(ENOTCONN);
      }

      // Only up to the send buffer's headroom is accepted. EAGAIN is returned
      // when there is none, or while still connecting: poll() reports POLLOUT
      // once the socket can be written to.
      var headroom = webTransportSockHelpers.sendHeadroom(sock);
      if (sock.state === WEBTRANSPORT_STATE_CONNECTING || headroom <= 0) {
        throw new FS.ErrnoError(EAGAIN);
      }
      length = Math.min(length, headroom);

      // Copy out of (possibly shared) wasm memory: slice() returns a new,
      // unshared buffer that stays stable until the write completes.
      sock.sendBuffered += length;
      webTransportSockHelpers.write(sock, new Uint8Array(buffer.buffer, buffer.byteOffset + offset, length).slice());
      return length;
    },
    recvmsg(sock, length, flags) {
      webTransportSockHelpers.checkNonBlocking(sock);
      // MSG_PEEK returns the data from the head of the queue without consuming
      // it, so a later recv sees the same bytes.
      var peek = flags & MSG_PEEK;
      var queued = sock.recv_queue[0];
      if (!queued) {
        if (sock.error) {
          var err = sock.error;
          sock.error = null;
          throw new FS.ErrnoError(err);
        }
        if (sock.readClosed) {
          return null; // EOF
        }
        if (!sock.session) {
          throw new FS.ErrnoError(ENOTCONN);
        }
        throw new FS.ErrnoError(EAGAIN);
      }
      var q = queued.data;
      var bytesRead = Math.min(length, q.length);
      var res = { buffer: q.subarray(0, bytesRead), addr: sock.daddr, port: sock.dport };
      if (peek) {
        return res;
      }
      if (bytesRead < q.length) {
        queued.data = q.subarray(bytesRead);
      } else {
        sock.recv_queue.shift();
      }
      sock.recvBuffered -= bytesRead;
      webTransportSockHelpers.pump(sock); // there may be room to read again
      return res;
    },
    // There is no TCP connection of our own to tune, so the common options that
    // make no difference here are accepted and ignored, so that portable code
    // doesn't fail on them. QUIC has no Nagle's algorithm: data is always sent
    // right away, as with TCP_NODELAY.
    setsockopt(sock, level, optname, optval, optlen) {
      if ((level === SOL_SOCKET && (optname === 2 /* SO_REUSEADDR */ || optname === 9 /* SO_KEEPALIVE */)) ||
          (level === IPPROTO_TCP && optname === 1 /* TCP_NODELAY */)) {
        return 0;
      }
      return -ENOPROTOOPT;
    },
    getsockopt(sock, level, optname, optval, optlen) {
      var val;
      if (level === SOL_SOCKET && optname === SO_ERROR) {
        val = sock.error || 0;
        sock.error = null; // SO_ERROR reads and clears
      } else if (level === IPPROTO_TCP && optname === 1 /* TCP_NODELAY */) {
        val = 1;
      } else {
        return -ENOPROTOOPT;
      }
      {{{ makeSetValue('optval', 0, 'val', 'i32') }}};
      {{{ makeSetValue('optlen', 0, 4, 'i32') }}};
      return 0;
    },
  },
};

addToLibrary(WebTransportSockFSLibrary);
