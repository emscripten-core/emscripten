/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $SOCKFS__postset: () => {
    addAtInit('SOCKFS.root = FS.mount(SOCKFS, {}, null);');
  },
  $SOCKFS__deps: ['$FS',
#if NODERAWSOCKETS
    '$nodeSockOps',
#endif
  ],
  $SOCKFS: {
#if expectToReceiveOnModule('websocket')
    websocketArgs: {},
#endif
    callbacks: {},
    on(event, callback) {
      SOCKFS.callbacks[event] = callback;
    },
    emit(event, param) {
      SOCKFS.callbacks[event]?.(param);
      // Bridge socket readiness into the inode wait-queue (poll/epoll). The
      // 'error' event carries [fd, ...]; the rest carry the fd directly.
      var fd = event === 'error' ? param[0] : param;
      var flags = {
        'message':    {{{ cDefs.POLLRDNORM }}} | {{{ cDefs.POLLIN }}},
        'open':       {{{ cDefs.POLLOUT }}},
        'connection': {{{ cDefs.POLLRDNORM }}} | {{{ cDefs.POLLIN }}},
        'close':      {{{ cDefs.POLLIN }}} | {{{ cDefs.POLLHUP }}},
        'error':      {{{ cDefs.POLLERR }}},
      }[event];
      // 'listen' has no readiness mapping; skip it.
      if (flags) FS.getStream(fd)?.node.notifyListeners(flags);
    },
    mount(mount) {
#if expectToReceiveOnModule('websocket')
      // The incoming Module['websocket'] can be used for configuring 
      // subprotocol/url, etc
      SOCKFS.websocketArgs = {{{ makeModuleReceiveExpr('websocket', '{}') }}};
      // Add the Event registration mechanism to the exported websocket configuration
      // object so we can register network callbacks from native JavaScript too.
      // For more documentation see system/include/emscripten/emscripten.h
      (Module['websocket'] ??= {})['on'] = SOCKFS.on;
#endif

#if SOCKET_DEBUG
      // If debug is enabled register simple default logging callbacks for each Event.
      SOCKFS.on('error', (error) => dbg(`websocket: error ${error}`));
      SOCKFS.on('open', (fd) => dbg(`websocket: open fd = ${fd}`));
      SOCKFS.on('listen', (fd) => dbg(`websocket: listen fd = ${fd}`));
      SOCKFS.on('connection', (fd) => dbg(`websocket: connection fd = ${fd}`));
      SOCKFS.on('message', (fd) => dbg(`websocket: message fd = ${fd}`));
      SOCKFS.on('close', (fd) => dbg(`websocket: close fd = ${fd}`));
#endif

      return FS.createNode(null, '/', {{{ cDefs.S_IFDIR | 0o777 }}}, 0);
    },
    createSocket(family, type, protocol) {
      if (family != {{{ cDefs.AF_INET }}}
#if NODERAWSOCKETS
          // The node:net backend supports IPv6; other backends are IPv4 only.
          && family != {{{ cDefs.AF_INET6 }}}
#endif
         ) {
        throw new FS.ErrnoError({{{ cDefs.EAFNOSUPPORT }}});
      }
      type &= ~{{{ cDefs.SOCK_CLOEXEC | cDefs.SOCK_NONBLOCK }}}; // Some applications may pass it; it makes no sense for a single process.
      // Emscripten only supports SOCK_STREAM and SOCK_DGRAM
      if (type != {{{ cDefs.SOCK_STREAM }}} && type != {{{ cDefs.SOCK_DGRAM }}}) {
        throw new FS.ErrnoError({{{ cDefs.EINVAL }}});
      }
      var streaming = type == {{{ cDefs.SOCK_STREAM }}};
      if (streaming && protocol && protocol != {{{ cDefs.IPPROTO_TCP }}}) {
        throw new FS.ErrnoError({{{ cDefs.EPROTONOSUPPORT }}}); // if SOCK_STREAM, must be tcp or 0.
      }

      // create our internal socket structure
      var sock = {
        family,
        type,
        protocol,
        server: null,
        error: null, // Used in getsockopt for SOL_SOCKET/SO_ERROR test
        peers: {},
        pending: [],
        recv_queue: [],
#if SOCKET_WEBRTC
#elif NODERAWSOCKETS
        sock_ops: nodeSockOps
#else
        sock_ops: SOCKFS.websocket_sock_ops
#endif
      };

      // create the filesystem node to store the socket structure
      var name = SOCKFS.nextname();
      var node = FS.createNode(SOCKFS.root, name, {{{ cDefs.S_IFSOCK }}}, 0);
      node.sock = sock;

      // and the wrapping stream that enables library functions such
      // as read and write to indirectly interact with the socket
      var stream = FS.createStream({
        path: name,
        node,
        flags: {{{ cDefs.O_RDWR }}},
        seekable: false,
        stream_ops: SOCKFS.stream_ops
      });

      // map the new stream to the socket structure (sockets have a 1:1
      // relationship with a stream)
      sock.stream = stream;

      return sock;
    },
    getSocket(fd) {
      var stream = FS.getStream(fd);
      if (!stream || !FS.isSocket(stream.node.mode)) {
        return null;
      }
      return stream.node.sock;
    },
    // node and stream ops are backend agnostic
    stream_ops: {
      poll(stream) {
        var sock = stream.node.sock;
        return sock.sock_ops.poll(sock);
      },
      ioctl(stream, request, varargs) {
        var sock = stream.node.sock;
        return sock.sock_ops.ioctl(sock, request, varargs);
      },
      read(stream, buffer, offset, length, position /* ignored */) {
        var sock = stream.node.sock;
        var msg = sock.sock_ops.recvmsg(sock, length);
        if (!msg) {
          // socket is closed
          return 0;
        }
        buffer.set(msg.buffer, offset);
        return msg.buffer.length;
      },
      write(stream, buffer, offset, length, position /* ignored */) {
        var sock = stream.node.sock;
        return sock.sock_ops.sendmsg(sock, buffer, offset, length);
      },
      close(stream) {
        var sock = stream.node.sock;
        sock.sock_ops.close(sock);
      }
    },
    nextname() {
      if (!SOCKFS.nextname.current) {
        SOCKFS.nextname.current = 0;
      }
      return `socket[${SOCKFS.nextname.current++}]`;
    },
    // backend-specific stream ops
    websocket_sock_ops: {
      //
      // peers are a small wrapper around a WebSocket to help in
      // emulating dgram sockets
      //
      // these functions aren't actually sock_ops members, but we're
      // abusing the namespace to organize them
      //
      createPeer(sock, addr, port) {
        var ws;

        if (typeof addr == 'object') {
          ws = addr;
          addr = null;
          port = null;
        }

        if (ws) {
          // for sockets that've already connected (e.g. we're the server)
          // we can inspect the _socket property for the address
          if (ws._socket) {
            addr = ws._socket.remoteAddress;
            port = ws._socket.remotePort;
          }
          // if we're just now initializing a connection to the remote,
          // inspect the url property
          else {
            var result = /ws[s]?:\/\/([^:]+):(\d+)/.exec(ws.url);
            if (!result) {
              throw new Error('WebSocket URL must be in the format ws(s)://address:port');
            }
            addr = result[1];
            port = parseInt(result[2], 10);
          }
        } else {
          // create the actual websocket object and connect
          try {
            // The default value is 'ws://' the replace is needed because the compiler replaces '//' comments with '#'
            // comments without checking context, so we'd end up with ws:#, the replace swaps the '#' for '//' again.
            var url = '{{{ WEBSOCKET_URL }}}'.replace('#', '//');
            // Make the WebSocket subprotocol (Sec-WebSocket-Protocol) default to binary if no configuration is set.
            var subProtocols = '{{{ WEBSOCKET_SUBPROTOCOL }}}'; // The default value is 'binary'
            // The default WebSocket options
            var opts = undefined;

#if expectToReceiveOnModule('websocket')
            // Fetch runtime WebSocket URL config.
            if (SOCKFS.websocketArgs['url']) {
              url = SOCKFS.websocketArgs['url'];
            }
            // Fetch runtime WebSocket subprotocol config.
            if (SOCKFS.websocketArgs['subprotocol']) {
              subProtocols = SOCKFS.websocketArgs['subprotocol'];
            } else if (SOCKFS.websocketArgs['subprotocol'] === null) {
              subProtocols = 'null'
            }
#endif

            if (url === 'ws://' || url === 'wss://') { // Is the supplied URL config just a prefix, if so complete it.
              var parts = addr.split('/');
              url = url + parts[0] + ":" + port + "/" + parts.slice(1).join('/');
            }

            if (subProtocols !== 'null') {
              // The regex trims the string (removes spaces at the beginning and end), then splits the string by
              // <any space>,<any space> into an Array. Whitespace removal is important for Websockify and ws.
              subProtocols = subProtocols.replace(/^ +| +$/g,"").split(/ *, */);

              opts = subProtocols;
            }

#if SOCKET_DEBUG
            dbg(`websocket: connect: ${url}, ${subProtocols.toString()}`);
#endif
            // If node we use the ws library.
            var WebSocketConstructor;
#if ENVIRONMENT_MAY_BE_NODE
            if (ENVIRONMENT_IS_NODE) {
              WebSocketConstructor = /** @type{(typeof WebSocket)} */(require('ws'));
            } else
#endif // ENVIRONMENT_MAY_BE_NODE
            {
              WebSocketConstructor = WebSocket;
            }
            ws = new WebSocketConstructor(url, opts);
            ws.binaryType = 'arraybuffer';
          } catch (e) {
#if SOCKET_DEBUG
            dbg(`websocket: error connecting: ${e}`);
#endif
            throw new FS.ErrnoError({{{ cDefs.EHOSTUNREACH }}});
          }
        }

#if SOCKET_DEBUG
        dbg(`websocket: adding peer: ${addr}:${port}`);
#endif

        var peer = {
          addr,
          port,
          socket: ws,
          msg_send_queue: []
        };

        SOCKFS.websocket_sock_ops.addPeer(sock, peer);
        SOCKFS.websocket_sock_ops.handlePeerEvents(sock, peer);

        // if this is a bound dgram socket, send the port number first to allow
        // us to override the ephemeral port reported to us by remotePort on the
        // remote end.
        if (sock.type === {{{ cDefs.SOCK_DGRAM }}} && typeof sock.sport != 'undefined') {
#if SOCKET_DEBUG
          dbg(`websocket: queuing port message (port ${sock.sport})`);
#endif
          peer.msg_send_queue.push(new Uint8Array([
              255, 255, 255, 255,
              'p'.charCodeAt(0), 'o'.charCodeAt(0), 'r'.charCodeAt(0), 't'.charCodeAt(0),
              ((sock.sport & 0xff00) >> 8) , (sock.sport & 0xff)
          ]));
        }

        return peer;
      },
      getPeer(sock, addr, port) {
        return sock.peers[addr + ':' + port];
      },
      addPeer(sock, peer) {
        sock.peers[peer.addr + ':' + peer.port] = peer;
      },
      removePeer(sock, peer) {
        delete sock.peers[peer.addr + ':' + peer.port];
      },
      handlePeerEvents(sock, peer) {
        var first = true;

        function handleOpen() {
#if SOCKET_DEBUG
          dbg('websocket: handle open');
#endif

          sock.connecting = false;
          SOCKFS.emit('open', sock.stream.fd);

          try {
            var queued = peer.msg_send_queue.shift();
            while (queued) {
#if SOCKET_DEBUG
              dbg(`websocket: sending queued data (${queued.byteLength} bytes): ${new Uint8Array(queued)}`);
#endif
              peer.socket.send(queued);
              queued = peer.msg_send_queue.shift();
            }
          } catch (e) {
            // not much we can do here in the way of proper error handling as we've already
            // lied and said this data was sent. shut it down.
            peer.socket.close();
          }
        }

        function handleMessage(data) {
          if (typeof data == 'string') {
            var encoder = new TextEncoder(); // should be utf-8
            data = encoder.encode(data); // make a typed array from the string
          } else {
#if ASSERTIONS
            assert(data.byteLength !== undefined); // must receive an ArrayBuffer
#endif
            if (data.byteLength == 0) {
              // An empty ArrayBuffer will emit a pseudo disconnect event
              // as recv/recvmsg will return zero which indicates that a socket
              // has performed a shutdown although the connection has not been disconnected yet.
              return;
            }
            data = new Uint8Array(data); // make a typed array view on the array buffer
          }

#if SOCKET_DEBUG
          dbg(`websocket: handle message (${data.byteLength} bytes): ${data}`);
#endif

          // if this is the port message, override the peer's port with it
          var wasfirst = first;
          first = false;
          if (wasfirst &&
              data.length === 10 &&
              data[0] === 255 && data[1] === 255 && data[2] === 255 && data[3] === 255 &&
              data[4] === 'p'.charCodeAt(0) && data[5] === 'o'.charCodeAt(0) && data[6] === 'r'.charCodeAt(0) && data[7] === 't'.charCodeAt(0)) {
            // update the peer's port and its key in the peer map
            var newport = ((data[8] << 8) | data[9]);
            SOCKFS.websocket_sock_ops.removePeer(sock, peer);
            peer.port = newport;
            SOCKFS.websocket_sock_ops.addPeer(sock, peer);
            return;
          }

          sock.recv_queue.push({ addr: peer.addr, port: peer.port, data: data });
          SOCKFS.emit('message', sock.stream.fd);
        }

#if ENVIRONMENT_MAY_BE_NODE
        if (ENVIRONMENT_IS_NODE) {
           // EventEmitter-style events use by ws library objects in Node.js).
          peer.socket.on('open', handleOpen);
          peer.socket.on('message', (data, isBinary) => {
            if (!isBinary) {
              return;
            }
            handleMessage((new Uint8Array(data)).buffer); // copy from node Buffer -> ArrayBuffer
          });
          peer.socket.on('close', () => SOCKFS.emit('close', sock.stream.fd));
          peer.socket.on('error', (error) =>{
            // Although the ws library may pass errors that may be more descriptive than
            // ECONNREFUSED they are not necessarily the expected error code e.g.
            // ENOTFOUND on getaddrinfo seems to be node.js specific, so using ECONNREFUSED
            // is still probably the most useful thing to do.
            sock.error = {{{ cDefs.ECONNREFUSED }}}; // Used in getsockopt for SOL_SOCKET/SO_ERROR test.
            SOCKFS.emit('error', [sock.stream.fd, sock.error, 'ECONNREFUSED: Connection refused']);
          });
          return;
        }
#endif
        peer.socket.onopen = handleOpen;
        peer.socket.onclose = () => SOCKFS.emit('close', sock.stream.fd);
        peer.socket.onmessage = (event) => handleMessage(event.data);
        peer.socket.onerror = (error) => {
          // The WebSocket spec only allows a 'simple event' to be thrown on error,
          // so we only really know as much as ECONNREFUSED.
          sock.error = {{{ cDefs.ECONNREFUSED }}}; // Used in getsockopt for SOL_SOCKET/SO_ERROR test.
          SOCKFS.emit('error', [sock.stream.fd, sock.error, 'ECONNREFUSED: Connection refused']);
        };
      },

      //
      // actual sock ops
      //
      poll(sock) {
        if (sock.type === {{{ cDefs.SOCK_STREAM }}} && sock.server) {
          // listen sockets should only say they're available for reading
          // if there are pending clients.
          return sock.pending.length ? ({{{ cDefs.POLLRDNORM }}} | {{{ cDefs.POLLIN }}}) : 0;
        }

        var mask = 0;
        var dest = sock.type === {{{ cDefs.SOCK_STREAM }}} ?  // we only care about the socket state for connection-based sockets
          SOCKFS.websocket_sock_ops.getPeer(sock, sock.daddr, sock.dport) :
          null;

        if (sock.recv_queue.length ||
            !dest ||  // connection-less sockets are always ready to read
            (dest && dest.socket.readyState === dest.socket.CLOSING) ||
            (dest && dest.socket.readyState === dest.socket.CLOSED)) {  // let recv return 0 once closed
          mask |= ({{{ cDefs.POLLRDNORM }}} | {{{ cDefs.POLLIN }}});
        }

        if (!dest ||  // connection-less sockets are always ready to write
            (dest && dest.socket.readyState === dest.socket.OPEN)) {
          mask |= {{{ cDefs.POLLOUT }}};
        }

        if ((dest && dest.socket.readyState === dest.socket.CLOSING) ||
            (dest && dest.socket.readyState === dest.socket.CLOSED)) {
          // When an non-blocking connect fails mark the socket as writable.
          // Its up to the calling code to then use getsockopt with SO_ERROR to
          // retrieve the error.
          // See https://man7.org/linux/man-pages/man2/connect.2.html
          if (sock.connecting) {
            mask |= {{{ cDefs.POLLOUT }}};
          } else  {
            // A closed peer is both a full hangup and a read-side hangup.
            mask |= {{{ cDefs.POLLHUP }}} | {{{ cDefs.POLLRDHUP }}};
          }
        }

        return mask;
      },
      ioctl(sock, request, arg) {
        switch (request) {
          case {{{ cDefs.FIONREAD }}}:
            var bytes = 0;
            if (sock.recv_queue.length) {
              bytes = sock.recv_queue[0].data.length;
            }
            {{{ makeSetValue('arg', '0', 'bytes', 'i32') }}};
            return 0;
          case {{{ cDefs.FIONBIO }}}:
            var on = {{{ makeGetValue('arg', '0', 'i32') }}};
            if (on) {
              sock.stream.flags |= {{{ cDefs.O_NONBLOCK }}};
            } else {
              sock.stream.flags &= ~{{{ cDefs.O_NONBLOCK }}};
            }
            return 0;
          default:
            return {{{ cDefs.EINVAL }}};
        }
      },
      close(sock) {
        // if we've spawned a listen server, close it
        if (sock.server) {
          try {
            sock.server.close();
          } catch (e) {
          }
          sock.server = null;
        }
        // close any peer connections
        for (var peer of Object.values(sock.peers)) {
          try {
            peer.socket.close();
          } catch (e) {
          }
          SOCKFS.websocket_sock_ops.removePeer(sock, peer);
        }
        return 0;
      },
      bind(sock, addr, port) {
        if (typeof sock.saddr != 'undefined' || typeof sock.sport != 'undefined') {
          throw new FS.ErrnoError({{{ cDefs.EINVAL }}});  // already bound
        }
        sock.saddr = addr;
        sock.sport = port;
        // in order to emulate dgram sockets, we need to launch a listen server when
        // binding on a connection-less socket
        // note: this is only required on the server side
        if (sock.type === {{{ cDefs.SOCK_DGRAM }}}) {
          // close the existing server if it exists
          if (sock.server) {
            sock.server.close();
            sock.server = null;
          }
          // swallow error operation not supported error that occurs when binding in the
          // browser where this isn't supported
          try {
            sock.sock_ops.listen(sock, 0);
          } catch (e) {
            if (!(e.name === 'ErrnoError')) throw e;
            if (e.errno !== {{{ cDefs.EOPNOTSUPP }}}) throw e;
          }
        }
      },
      connect(sock, addr, port) {
        if (sock.server) {
          throw new FS.ErrnoError({{{ cDefs.EOPNOTSUPP }}});
        }

        // TODO autobind
        // if (!sock.addr && sock.type == {{{ cDefs.SOCK_DGRAM }}}) {
        // }

        // early out if we're already connected / in the middle of connecting
        if (typeof sock.daddr != 'undefined' && typeof sock.dport != 'undefined') {
          var dest = SOCKFS.websocket_sock_ops.getPeer(sock, sock.daddr, sock.dport);
          if (dest) {
            if (dest.socket.readyState === dest.socket.CONNECTING) {
              throw new FS.ErrnoError({{{ cDefs.EALREADY }}});
            } else {
              throw new FS.ErrnoError({{{ cDefs.EISCONN }}});
            }
          }
        }

        // add the socket to our peer list and set our
        // destination address / port to match
        var peer = SOCKFS.websocket_sock_ops.createPeer(sock, addr, port);
        sock.daddr = peer.addr;
        sock.dport = peer.port;

        // because we cannot synchronously block to wait for the WebSocket
        // connection to complete, we return here pretending that the connection
        // was a success.
        sock.connecting = true;
      },
      listen(sock, backlog) {
        if (!ENVIRONMENT_IS_NODE) {
          throw new FS.ErrnoError({{{ cDefs.EOPNOTSUPP }}});
        }
#if ENVIRONMENT_MAY_BE_NODE
        if (sock.server) {
           throw new FS.ErrnoError({{{ cDefs.EINVAL }}});  // already listening
        }
        var WebSocketServer = require('ws').Server;
        var host = sock.saddr;
#if SOCKET_DEBUG
        dbg(`websocket: listen: ${host}:${sock.sport}`);
#endif
        sock.server = new WebSocketServer({
          host,
          port: sock.sport
          // TODO support backlog
        });
        SOCKFS.emit('listen', sock.stream.fd); // Send Event with listen fd.

        sock.server.on('connection', (ws) => {
#if SOCKET_DEBUG
          dbg(`websocket: received connection from: ${ws._socket.remoteAddress}:${ws._socket.remotePort}`);
#endif
          if (sock.type === {{{ cDefs.SOCK_STREAM }}}) {
            var newsock = SOCKFS.createSocket(sock.family, sock.type, sock.protocol);

            // create a peer on the new socket
            var peer = SOCKFS.websocket_sock_ops.createPeer(newsock, ws);
            newsock.daddr = peer.addr;
            newsock.dport = peer.port;

            // push to queue for accept to pick up
            sock.pending.push(newsock);
            SOCKFS.emit('connection', newsock.stream.fd);
            // A queued client makes the listening socket readable (POLLIN).
            sock.stream.node.notifyListeners({{{ cDefs.POLLRDNORM }}} | {{{ cDefs.POLLIN }}});
          } else {
            // create a peer on the listen socket so calling sendto
            // with the listen socket and an address will resolve
            // to the correct client
            SOCKFS.websocket_sock_ops.createPeer(sock, ws);
            SOCKFS.emit('connection', sock.stream.fd);
          }
        });
        sock.server.on('close', () => {
          SOCKFS.emit('close', sock.stream.fd);
          sock.server = null;
        });
        sock.server.on('error', (error) => {
          // Although the ws library may pass errors that may be more descriptive than
          // ECONNREFUSED they are not necessarily the expected error code e.g.
          // ENOTFOUND on getaddrinfo seems to be node.js specific, so using EHOSTUNREACH
          // is still probably the most useful thing to do. This error shouldn't
          // occur in a well written app as errors should get trapped in the compiled
          // app's own getaddrinfo call.
          sock.error = {{{ cDefs.EHOSTUNREACH }}}; // Used in getsockopt for SOL_SOCKET/SO_ERROR test.
          SOCKFS.emit('error', [sock.stream.fd, sock.error, 'EHOSTUNREACH: Host is unreachable']);
          // don't throw
        });
#endif // ENVIRONMENT_MAY_BE_NODE
      },
      accept(listensock) {
        if (!listensock.server || !listensock.pending.length) {
          throw new FS.ErrnoError({{{ cDefs.EINVAL }}});
        }
        var newsock = listensock.pending.shift();
        newsock.stream.flags = listensock.stream.flags;
        return newsock;
      },
      getname(sock, peer) {
        var addr, port;
        if (peer) {
          if (sock.daddr === undefined || sock.dport === undefined) {
            throw new FS.ErrnoError({{{ cDefs.ENOTCONN }}});
          }
          addr = sock.daddr;
          port = sock.dport;
        } else {
          // TODO saddr and sport will be set for bind()'d UDP sockets, but what
          // should we be returning for TCP sockets that've been connect()'d?
          addr = sock.saddr || 0;
          port = sock.sport || 0;
        }
        return { addr, port };
      },
      sendmsg(sock, buffer, offset, length, addr, port) {
        if (sock.type === {{{ cDefs.SOCK_DGRAM }}}) {
          // connection-less sockets will honor the message address,
          // and otherwise fall back to the bound destination address
          if (addr === undefined || port === undefined) {
            addr = sock.daddr;
            port = sock.dport;
          }
          // if there was no address to fall back to, error out
          if (addr === undefined || port === undefined) {
            throw new FS.ErrnoError({{{ cDefs.EDESTADDRREQ }}});
          }
        } else {
          // connection-based sockets will only use the bound
          addr = sock.daddr;
          port = sock.dport;
        }

        // find the peer for the destination address
        var dest = SOCKFS.websocket_sock_ops.getPeer(sock, addr, port);

        // early out if not connected with a connection-based socket
        if (sock.type === {{{ cDefs.SOCK_STREAM }}}) {
          if (!dest || dest.socket.readyState === dest.socket.CLOSING || dest.socket.readyState === dest.socket.CLOSED) {
            throw new FS.ErrnoError({{{ cDefs.ENOTCONN }}});
#if SOCKET_DEBUG
          } else if (dest.socket.readyState === dest.socket.CONNECTING) {
            dbg('socket sendmsg called while socket is still connecting.');
#endif
          }
        }

        // create a copy of the incoming data to send, as the WebSocket API
        // doesn't work entirely with an ArrayBufferView, it'll just send
        // the entire underlying buffer
        if (ArrayBuffer.isView(buffer)) {
          offset += buffer.byteOffset;
          buffer = buffer.buffer;
        }

        var data = buffer.slice(offset, offset + length);
#if PTHREADS
        // WebSockets .send() does not allow passing a SharedArrayBuffer, so
        // clone the SharedArrayBuffer as regular ArrayBuffer before
        // sending.
        if (data instanceof SharedArrayBuffer) {
          data = new Uint8Array(new Uint8Array(data)).buffer;
        }
#endif

        // if we don't have a cached connectionless UDP datagram connection, or
        // the TCP socket is still connecting, queue the message to be sent upon
        // connect, and lie, saying the data was sent now.
        if (!dest || dest.socket.readyState !== dest.socket.OPEN) {
          // if we're not connected, open a new connection
          if (sock.type === {{{ cDefs.SOCK_DGRAM }}}) {
            if (!dest || dest.socket.readyState === dest.socket.CLOSING || dest.socket.readyState === dest.socket.CLOSED) {
              dest = SOCKFS.websocket_sock_ops.createPeer(sock, addr, port);
            }
          }
#if SOCKET_DEBUG
          dbg(`websocket: queuing (${length} bytes): ${new Uint8Array(data)}`);
#endif
          dest.msg_send_queue.push(data);
          return length;
        }

        try {
#if SOCKET_DEBUG
          dbg(`websocket: send (${length} bytes): ${new Uint8Array(data)}`);
#endif
          // send the actual data
          dest.socket.send(data);
          return length;
        } catch (e) {
          throw new FS.ErrnoError({{{ cDefs.EINVAL }}});
        }
      },
      recvmsg(sock, length, flags) {
        // http://pubs.opengroup.org/onlinepubs/7908799/xns/recvmsg.html
        if (sock.type === {{{ cDefs.SOCK_STREAM }}} && sock.server) {
          // tcp servers should not be recv()'ing on the listen socket
          throw new FS.ErrnoError({{{ cDefs.ENOTCONN }}});
        }

        // MSG_PEEK returns the head of the queue without consuming it, so a
        // later recv sees the same bytes and poll still reports it readable.
        var peek = flags & {{{ cDefs.MSG_PEEK }}};
        var queued = sock.recv_queue[0];
        if (!queued) {
          if (sock.type === {{{ cDefs.SOCK_STREAM }}}) {
            var dest = SOCKFS.websocket_sock_ops.getPeer(sock, sock.daddr, sock.dport);

            if (!dest) {
              // if we have a destination address but are not connected, error out
              throw new FS.ErrnoError({{{ cDefs.ENOTCONN }}});
            }
            if (dest.socket.readyState === dest.socket.CLOSING || dest.socket.readyState === dest.socket.CLOSED) {
              // return null if the socket has closed
              return null;
            }
            // else, our socket is in a valid state but truly has nothing available
            throw new FS.ErrnoError({{{ cDefs.EAGAIN }}});
          }
          throw new FS.ErrnoError({{{ cDefs.EAGAIN }}});
        }

        // queued.data will be an ArrayBuffer if it's unadulterated, but if it's
        // requeued TCP data it'll be an ArrayBufferView
        var queuedLength = queued.data.byteLength || queued.data.length;
        var queuedOffset = queued.data.byteOffset || 0;
        var queuedBuffer = queued.data.buffer || queued.data;
        var bytesRead = Math.min(length, queuedLength);
        var res = {
          buffer: new Uint8Array(queuedBuffer, queuedOffset, bytesRead),
          addr: queued.addr,
          port: queued.port
        };

#if SOCKET_DEBUG
        dbg(`websocket: read (${bytesRead} bytes): ${res.buffer}`);
#endif

        if (peek) return res;
        sock.recv_queue.shift();

        // push back any unread data for TCP connections
        if (sock.type === {{{ cDefs.SOCK_STREAM }}} && bytesRead < queuedLength) {
          var bytesRemaining = queuedLength - bytesRead;
#if SOCKET_DEBUG
          dbg(`websocket: read: put back ${bytesRemaining} bytes`);
#endif
          queued.data = new Uint8Array(queuedBuffer, queuedOffset + bytesRead, bytesRemaining);
          sock.recv_queue.unshift(queued);
        }

        return res;
      }
    },
  },

  /*
   * Mechanism to register handlers for the various Socket Events from C code.
   * The registration functions are mostly variations on a theme, so we use this
   * generic handler. Most of the callback functions take a file descriptor as a
   * parameter, which will get passed to them by the emitting call. The error
   * callback also takes an int representing the errno and a char* representing the
   * error message, which we extract from the data passed to _callback and convert
   * to a char* string before calling the registered C callback.
   * Passing a NULL callback function to a emscripten_set_socket_*_callback call
   * will deregister the callback registered for that Event.
   */
  $_setNetworkCallback__deps: ['$withStackSave', '$callUserCallback', '$stringToUTF8OnStack'],
  $_setNetworkCallback: (event, userData, callback) => {
    function _callback(data) {
      callUserCallback(() => {
        if (event === 'error') {
          withStackSave(() => {
            var msg = stringToUTF8OnStack(data[2]);
            {{{ makeDynCall('viipp', 'callback') }}}(data[0], data[1], msg, userData);
          });
        } else {
          {{{ makeDynCall('vip', 'callback') }}}(data, userData);
        }
      });
    };

    // FIXME(sbc): This has no corresponding Pop so will currently keep the
    // runtime alive indefinitely.
    {{{ runtimeKeepalivePush() }}}
    SOCKFS.on(event, callback ? _callback : null);
  },
  emscripten_set_socket_error_callback__deps: ['$_setNetworkCallback'],
  emscripten_set_socket_error_callback: (userData, callback) =>
    _setNetworkCallback('error', userData, callback),
  emscripten_set_socket_open_callback__deps: ['$_setNetworkCallback'],
  emscripten_set_socket_open_callback: (userData, callback) =>
    _setNetworkCallback('open', userData, callback),
  emscripten_set_socket_listen_callback__deps: ['$_setNetworkCallback'],
  emscripten_set_socket_listen_callback: (userData, callback) =>
    _setNetworkCallback('listen', userData, callback),
  emscripten_set_socket_connection_callback__deps: ['$_setNetworkCallback'],
  emscripten_set_socket_connection_callback: (userData, callback) =>
    _setNetworkCallback('connection', userData, callback),
  emscripten_set_socket_message_callback__deps: ['$_setNetworkCallback'],
  emscripten_set_socket_message_callback: (userData, callback) =>
    _setNetworkCallback('message', userData, callback),
  emscripten_set_socket_close_callback__deps: ['$_setNetworkCallback'],
  emscripten_set_socket_close_callback: (userData, callback) =>
    _setNetworkCallback('close', userData, callback),
});

// The socket syscalls. Relocated here from libsyscall.js because they are
// SOCKFS-coupled ($getSocketFromFD / SOCKFS) and the blocking data ops build on
// $streamWaitOp (libfs.js). Under PROXY_POSIX_SOCKETS the socket syscalls are
// native (libsockets.a); under WASMFS this file is not linked at all.
//
// __proxy: 'sync' consolidates socket ownership on the main thread (a node:net
// handle is a per-realm JS object; only linear memory is shared), matching the
// FS + epoll model - inert unless SHARED_MEMORY. The blocking data ops carry
// __async: 'auto' and delegate to $streamWaitOp, which suspends the wasm stack
// (JSPI/Asyncify) or, when proxied from a worker, resolves the sync-proxy across
// the main-thread event loop; on a purely-synchronous build they degrade to the
// old immediate-EAGAIN behaviour.
#if PROXY_POSIX_SOCKETS == 0 && WASMFS == 0
var SockFSSyscalls = {
  $getSocketFromFD__deps: ['$SOCKFS', '$FS'],
  $getSocketFromFD: (fd) => {
    var socket = SOCKFS.getSocket(fd);
    if (!socket) throw new FS.ErrnoError({{{ cDefs.EBADF }}});
#if SYSCALL_DEBUG
    dbg(`    (socket: "${socket.path}")`);
#endif
    return socket;
  },
  $getSocketAddress__deps: ['$readSockaddr', '$FS', '$DNS'],
  $getSocketAddress: (addrp, addrlen) => {
    var info = readSockaddr(addrp, addrlen);
    if (info.errno) throw new FS.ErrnoError(info.errno);
    info.addr = DNS.lookup_addr(info.addr) || info.addr;
#if SYSCALL_DEBUG
    dbg(`    (socketaddress: "${[info.addr, info.port]}")`);
#endif
    return info;
  },
  __syscall_socket__proxy: 'sync',
  __syscall_socket__deps: ['$SOCKFS'],
  __syscall_socket: (domain, type, protocol, u1, u2, u3) => {
    var sock = SOCKFS.createSocket(domain, type, protocol);
    return sock.stream.fd;
  },
  __syscall_getsockname__proxy: 'sync',
  __syscall_getsockname__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_getsockname: (fd, addr, len, u1, u2, u3) => {
    var sock = getSocketFromFD(fd);
    // TODO: sock.saddr should never be undefined, see TODO in websocket_sock_ops.getname
    var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(sock.saddr || '0.0.0.0'), sock.sport, len);
#if ASSERTIONS
    assert(!errno);
#endif
    return 0;
  },
  __syscall_getpeername__proxy: 'sync',
  __syscall_getpeername__deps: ['$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_getpeername: (fd, addr, len, u1, u2, u3) => {
    var sock = getSocketFromFD(fd);
    if (!sock.daddr) {
      return -{{{ cDefs.ENOTCONN }}}; // The socket is not connected.
    }
    var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(sock.daddr), sock.dport, len);
#if ASSERTIONS
    assert(!errno);
#endif
    return 0;
  },
  // Blocking connect: the first attempt initiates the connect and reports it
  // pending; the wait resolves once the socket becomes writable (or errors),
  // reading SO_ERROR to distinguish success (0) from a failed connect (-err).
  // The pending/completion signalling is backend-agnostic: it uses the same
  // poll()/SO_ERROR the reactor uses, driven by the node backend's
  // 'connect'/'error' (main: 'open'/'error') wait-queue emissions.
  __syscall_connect__proxy: 'sync',
  __syscall_connect__async: 'auto',
  __syscall_connect__deps: ['$streamWaitOp', '$getSocketFromFD', '$getSocketAddress'],
  __syscall_connect: (fd, addr, len, u1, u2, u3) => {
    var sock = getSocketFromFD(fd);
    var info = getSocketAddress(addr, len);
    var started = false;
    return streamWaitOp(fd, () => {
      if (!started) {
        started = true;
        sock.sock_ops.connect(sock, info.addr, info.port);
        // Datagram connect is synchronous.
        if (sock.type == {{{ cDefs.SOCK_DGRAM }}}) return 0;
#if ASYNCIFY || PTHREADS
        // Non-blocking: report the connect in progress; blocking: wait.
        return (sock.stream.flags & {{{ cDefs.O_NONBLOCK }}}) ? -{{{ cDefs.EINPROGRESS }}} : -{{{ cDefs.EAGAIN }}};
#else
        // No suspension available: non-blocking reports EINPROGRESS, blocking
        // keeps the legacy fire-and-forget success.
        return (sock.stream.flags & {{{ cDefs.O_NONBLOCK }}}) ? -{{{ cDefs.EINPROGRESS }}} : 0;
#endif
      }
      // Woken: writable/error means the connect resolved. Read (and clear)
      // SO_ERROR to map success/failure onto the syscall result.
      var mask = sock.sock_ops.poll(sock);
      if (!(mask & ({{{ cDefs.POLLOUT }}} | {{{ cDefs.POLLERR }}} | {{{ cDefs.POLLHUP }}}))) {
        return -{{{ cDefs.EAGAIN }}};
      }
      var err = sock.error;
      sock.error = null;
      return err ? -err : 0;
    });
  },
  __syscall_shutdown__proxy: 'sync',
  __syscall_shutdown__deps: ['$getSocketFromFD'],
  __syscall_shutdown: (fd, how, u1, u2, u3, u4) => {
    var sock = getSocketFromFD(fd);
#if NODERAWSOCKETS
    return sock.sock_ops.shutdown(sock, how);
#else
    return -{{{ cDefs.ENOSYS }}}; // unsupported feature
#endif
  },
  // Blocking accept: waits until a connection is pending (listener readable),
  // then hands it out. `flags` (SOCK_NONBLOCK/SOCK_CLOEXEC) apply to the new fd
  // only, never to the listener's own blocking behaviour.
  __syscall_accept4__proxy: 'sync',
  __syscall_accept4__async: 'auto',
  __syscall_accept4__deps: ['$streamWaitOp', '$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_accept4: (fd, addr, len, flags, u1, u2) => {
    var sock = getSocketFromFD(fd);
    return streamWaitOp(fd, () => {
      var newsock = sock.sock_ops.accept(sock); // throws EAGAIN until one is pending
      if (addr) {
        var errno = writeSockaddr(addr, newsock.family, DNS.lookup_name(newsock.daddr), newsock.dport, len);
#if ASSERTIONS
        assert(!errno);
#endif
      }
      if (flags & {{{ cDefs.SOCK_NONBLOCK }}}) newsock.stream.flags |= {{{ cDefs.O_NONBLOCK }}};
      else newsock.stream.flags &= ~{{{ cDefs.O_NONBLOCK }}};
      if (flags & {{{ cDefs.SOCK_CLOEXEC }}}) newsock.stream.flags |= {{{ cDefs.O_CLOEXEC }}};
      return newsock.stream.fd;
    });
  },
  __syscall_bind__proxy: 'sync',
  __syscall_bind__deps: ['$getSocketFromFD', '$getSocketAddress'],
  __syscall_bind: (fd, addr, len, u1, u2, u3) => {
    var sock = getSocketFromFD(fd);
    var info = getSocketAddress(addr, len);
    sock.sock_ops.bind(sock, info.addr, info.port);
    return 0;
  },
  __syscall_listen__proxy: 'sync',
  __syscall_listen__deps: ['$getSocketFromFD'],
  __syscall_listen: (fd, backlog, u1, u2, u3, u4) => {
    var sock = getSocketFromFD(fd);
    sock.sock_ops.listen(sock, backlog);
    return 0;
  },
  // Blocking recv: waits until data is present (or EOF); MSG_DONTWAIT forces a
  // single non-blocking attempt for that call.
  __syscall_recvfrom__proxy: 'sync',
  __syscall_recvfrom__async: 'auto',
  __syscall_recvfrom__deps: ['$streamWaitOp', '$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_recvfrom: (fd, buf, len, flags, addr, alen) => {
    var sock = getSocketFromFD(fd);
    return streamWaitOp(fd, () => {
      var msg = sock.sock_ops.recvmsg(sock, len, flags); // throws EAGAIN until data present
      if (!msg) return 0; // socket is closed
      if (addr) {
        var errno = writeSockaddr(addr, sock.family, DNS.lookup_name(msg.addr), msg.port, alen);
#if ASSERTIONS
        assert(!errno);
#endif
      }
      HEAPU8.set(msg.buffer, buf);
      return msg.buffer.byteLength;
    }, flags & {{{ cDefs.MSG_DONTWAIT }}});
  },
  // Blocking send: a blocking socket's backend never would-blocks (it buffers),
  // so this normally resolves synchronously; the wait exists for symmetry with a
  // backend that reports EAGAIN on a full send buffer (wakes on writable).
  __syscall_sendto__proxy: 'sync',
  __syscall_sendto__async: 'auto',
  __syscall_sendto__deps: ['$streamWaitOp', '$getSocketFromFD', '$getSocketAddress'],
  __syscall_sendto: (fd, buf, len, flags, addr, alen) => {
    var sock = getSocketFromFD(fd);
    return streamWaitOp(fd, () => {
      if (!addr) {
        // send, no address provided
        return FS.write(sock.stream, HEAP8, buf, len);
      }
      var dest = getSocketAddress(addr, alen);
      // sendto an address
      return sock.sock_ops.sendmsg(sock, HEAP8, buf, len, dest.addr, dest.port);
    }, flags & {{{ cDefs.MSG_DONTWAIT }}});
  },
  __syscall_getsockopt__proxy: 'sync',
  __syscall_getsockopt__deps: ['$getSocketFromFD'],
  __syscall_getsockopt: (fd, level, optname, optval, optlen, unused) => {
    var sock = getSocketFromFD(fd);
#if NODERAWSOCKETS
    // The node:net backend handles all socket options.
    return sock.sock_ops.getsockopt(sock, level, optname, optval, optlen);
#else
    // Minimal getsockopt aimed at resolving https://github.com/emscripten-core/emscripten/issues/2211
    // so only supports SOL_SOCKET with SO_ERROR.
    if (level === {{{ cDefs.SOL_SOCKET }}}) {
      if (optname === {{{ cDefs.SO_ERROR }}}) {
        {{{ makeSetValue('optval', 0, 'sock.error', 'i32') }}};
        {{{ makeSetValue('optlen', 0, 4, 'i32') }}};
        sock.error = null; // Clear the error (The SO_ERROR option obtains and then clears this field).
        return 0;
      }
    }
    return -{{{ cDefs.ENOPROTOOPT }}}; // The option is unknown at the level indicated.
#endif
  },
  // Defined in JS rather than as a weak native stub so the node:net backend can
  // provide it without a separate libstubs variation. Without that backend it
  // just reports the option as unknown.
  __syscall_setsockopt__proxy: 'sync',
  __syscall_setsockopt__deps: ['$getSocketFromFD'],
  __syscall_setsockopt: (fd, level, optname, optval, optlen, unused) => {
#if NODERAWSOCKETS
    var sock = getSocketFromFD(fd);
    return sock.sock_ops.setsockopt(sock, level, optname, optval, optlen);
#else
    getSocketFromFD(fd); // validate the fd (and keep this syscall's catch reachable)
    return -{{{ cDefs.ENOPROTOOPT }}}; // The option is unknown at the level indicated.
#endif
  },
  __syscall_sendmsg__proxy: 'sync',
  __syscall_sendmsg__async: 'auto',
  __syscall_sendmsg__deps: ['$streamWaitOp', '$getSocketFromFD', '$getSocketAddress'],
  __syscall_sendmsg: (fd, message, flags, u1, u2, u3) => {
    var sock = getSocketFromFD(fd);
    return streamWaitOp(fd, () => {
      var iov = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iov, '*') }}};
      var num = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iovlen, 'i32') }}};
      // read the address and port to send to
      var addr, port;
      var name = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_name, '*') }}};
      var namelen = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_namelen, 'i32') }}};
      if (name) {
        var info = getSocketAddress(name, namelen);
        port = info.port;
        addr = info.addr;
      }
      // concatenate scatter-gather arrays into one message buffer
      var total = 0;
      for (var i = 0; i < num; i++) {
        total += {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_len}`, 'i32') }}};
      }
      var view = new Uint8Array(total);
      var offset = 0;
      for (var i = 0; i < num; i++) {
        var iovbase = {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_base}`, '*') }}};
        var iovlen = {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_len}`, 'i32') }}};
        for (var j = 0; j < iovlen; j++) {
          view[offset++] = {{{ makeGetValue('iovbase', 'j', 'i8') }}};
        }
      }
      // write the buffer
      return sock.sock_ops.sendmsg(sock, view, 0, total, addr, port);
    }, flags & {{{ cDefs.MSG_DONTWAIT }}});
  },
  // Blocking recvmsg: like recvfrom, waits until data is present (or EOF).
  __syscall_recvmsg__proxy: 'sync',
  __syscall_recvmsg__async: 'auto',
  __syscall_recvmsg__deps: ['$streamWaitOp', '$getSocketFromFD', '$writeSockaddr', '$DNS'],
  __syscall_recvmsg: (fd, message, flags, u1, u2, u3) => {
    var sock = getSocketFromFD(fd);
    return streamWaitOp(fd, () => {
      var iov = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iov, '*') }}};
      var num = {{{ makeGetValue('message', C_STRUCTS.msghdr.msg_iovlen, 'i32') }}};
      // get the total amount of data we can read across all arrays
      var total = 0;
      for (var i = 0; i < num; i++) {
        total += {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_len}`, 'i32') }}};
      }
      // try to read total data (MSG_PEEK, when set, leaves it buffered)
      var msg = sock.sock_ops.recvmsg(sock, total, flags); // throws EAGAIN until data present
      if (!msg) return 0; // socket is closed

      // TODO honor flags:
      // MSG_OOB
      // Requests out-of-band data. The significance and semantics of out-of-band data are protocol-specific.
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
        var iovbase = {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_base}`, '*') }}};
        var iovlen = {{{ makeGetValue('iov', `(${C_STRUCTS.iovec.__size__} * i) + ${C_STRUCTS.iovec.iov_len}`, 'i32') }}};
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
    }, flags & {{{ cDefs.MSG_DONTWAIT }}});
  },
};

for (const name of Object.keys(SockFSSyscalls)) {
  wrapSyscallFunction(name, SockFSSyscalls, false);
}

addToLibrary(SockFSSyscalls);
#endif // PROXY_POSIX_SOCKETS == 0 && WASMFS == 0
