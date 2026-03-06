.. Networking:

==========
Networking
==========

Emscripten compiled applications have a number of ways to connect with online
servers. Check the subtopics here to learn about the different strategies that
are available.

If you are familiar with networking concepts provided by different web APIs,
such as XmlHttpRequest, Fetch, WebSockets and WebRTC, you can quickly get
started by leveraging what you already know: by calling out from C/C++ code to
JavaScript (see the "Connecting C++ and JavaScript" section), you can establish
networked connections by writing regular JavaScript. For C/C++ developers,
Emscripten provides a few approaches, described here.

Emscripten WebSockets API
=========================

WebSockets API provides connection-oriented message-framed bidirectional
asynchronous networking communication to the browser. It is the closest to TCP
on the web that web sites can access, direct access to TCP sockets is not
possible from web browsers.

Emscripten provides a passthrough API for accessing the WebSockets API from
C/C++ code. This is useful for developers who would prefer not to write any
JavaScript code, or deal with the C/C++ and JavaScript language interop. See the
system include file ``<emscripten/websocket.h>`` for details. One benefit that
the Emscripten WebSockets API provides over manual WebSockets access in
JavaScript is the ability to share access to a WebSocket handle across multiple
threads, something that can be time consuming to develop from scratch.

To target Emscripten WebSockets API, you must link it in with a
``-lwebsocket.js`` linker directive.

Emulated POSIX TCP Sockets over WebSockets
==========================================

If you have existing TCP networking code written in C/C++ that utilizes the
Posix Sockets API, by default Emscripten attempts to emulate such connections to
take place over the WebSocket protocol instead. For this to work, you will need
to use something like WebSockify on the server side to enable the TCP server
stack to receive incoming WebSocket connections. This emulation is not very
complete at the moment, it is likely that you will run into problems out of the
box and need to adapt the code to work within the limitations that this
emulation provides.

This is the default build mode for Emscripten. Use the linker flag
``-sWEBSOCKET_URL`` or ``Module['websocket']['url']`` to specify the WebSocket URL
to connect to, and the linker flag ``-sWEBSOCKET_SUBPROTOCOL`` or
``Module['websocket']['subprotocol']`` to control the connection type
(``'binary'`` or ``'text'``).

Full POSIX Sockets over WebSocket Proxy Server
==============================================

Emscripten provides a native POSIX Sockets proxy server program, located in
directory ``tools/websocket_to_posix_proxy/``, that allows full POSIX Sockets
API access from a web browser. This support works by proxying all POSIX Sockets
API calls from the browser to the Emscripten POSIX Sockets proxy server (via
transparent use of the WebSockets API), and the proxy server then performs the
native TCP/UDP calls on behalf of the page. This allows a web browser page to
run full TCP & UDP connections, act as a server to accept incoming connections,
and perform host name lookups and reverse lookups. Because all API calls are
individually proxied, this support can be slow. This support is mostly useful
for developing testing infrastructure and debugging.

The following POSIX sockets functions are proxied in this manner:
 - ``socket()``, ``socketpair()``, ``shutdown()``, ``bind()``, ``connect()``, ``listen()``, ``accept()``, ``getsockname()``, ``getpeername()``, ``send()``, ``recv()``, ``sendto()``, ``recvfrom()``, ``sendmsg()``, ``recvmsg()``, ``getsockopt()``, ``setsockopt()``, ``getaddrinfo()``, ``getnameinfo()``.

The following POSIX sockets functions are currently not proxied (and will not work):
 - ``poll()``, ``close()`` (use ``shutdown()`` instead), ``select()``

To use POSIX sockets proxying, link the application with flags ``-lwebsocket.js
-sPROXY_POSIX_SOCKETS -pthread -sPROXY_TO_PTHREAD``. That is,
POSIX sockets proxying builds on top of the Emscripten WebSockets library, and
requires multithreading and proxying the application ``main()`` to a pthread.

For an example of how the POSIX Sockets proxy server works in an Emscripten
client program, see the file ``test/websocket/tcp_echo_client.c``.

XmlHttpRequests and Fetch API
=============================

For HTTP transfers, one can use the browser built-in XmlHttpRequest (XHR) API
and the newer Fetch API. These can be accessed directly from JavaScript.
Emscripten also provides passthrough APIs to perform HTTP requests. For more
information, see the ``emscripten_async_wget*()`` C API and the Emscripten Fetch
API.

WebRTC and UDP
==============

Direct UDP communication is not available in browsers, but as a close
alternative, the WebRTC specification provides a mechanism to perform UDP-like
communication with WebRTC Data Channels. Currently Emscripten does not provide a
C/C++ API for interacting with WebRTC.

Direct Sockets API (Isolated Web Apps)
=======================================

The `Direct Sockets API <https://wicg.github.io/direct-sockets/>`_ provides
real TCP and UDP socket access from the browser, without needing a proxy server.

Emscripten can route POSIX socket syscalls through the Direct Sockets API using
``TCPSocket``, ``TCPServerSocket``, and ``UDPSocket``. This enables existing
C/C++ networking code (including libraries like OpenSSL and Tor) to work
with real network sockets inside the browser.

To enable Direct Sockets support, compile and link with
``-sDIRECT_SOCKETS``. This also requires ``-sASYNCIFY`` (or JSPI).

Permissions
-----------

The Direct Sockets API requires a secure context and an isolated context
(cross-origin isolation). In practice, this means the API is only usable inside
Chrome Isolated Web Apps. Three permissions policy features control access:

 - ``direct-sockets`` — gates creation of all socket types. Defaults to
   ``'none'``.
 - ``direct-sockets-private`` — controls access to private/local network
   addresses. Without this policy, connections to private IP ranges will be
   rejected. Defaults to ``'none'``.
 - ``direct-sockets-multicast`` — required for multicast operations (see below).
   Defaults to ``'none'``.

All three must be explicitly enabled in the IWA's permissions policy for the
corresponding functionality to work. Note that ``TCPServerSocket`` is restricted
to ports above 32767.

The following POSIX socket functions are supported:
 - ``socket()``, ``bind()``, ``connect()``, ``listen()``, ``accept()``,
   ``send()``, ``recv()``, ``sendto()``, ``recvfrom()``, ``sendmsg()``,
   ``recvmsg()``, ``shutdown()``, ``getsockname()``, ``getpeername()``,
   ``setsockopt()``, ``getsockopt()``, ``poll()``, ``ioctl()``,
   ``fcntl()``, ``pipe()``, ``socketpair()``

Multicast
---------

The Direct Sockets API defines a ``MulticastController`` interface for UDP
bound sockets, providing ``joinGroup()`` and ``leaveGroup()`` methods with
support for both Any-Source Multicast (ASM) and Source-Specific Multicast (SSM).
Additional options include ``multicastTimeToLive``, ``multicastLoopback``, and
``multicastAllowAddressSharing``. Multicast requires the
``direct-sockets-multicast`` permissions policy, which defaults to ``'none'``.

Emscripten maps ``IP_ADD_MEMBERSHIP`` and ``IP_DROP_MEMBERSHIP`` socket options
to the ``MulticastController`` API. While this provides exposure to the
underlying multicast machinery, existing multicast code cross-compiled from a
POSIX environment should not be expected to work out of the box — the browser's
multicast surface area is considerably narrower than what a native stack
provides, and the permission model adds additional constraints.

DNS
---

The Direct Sockets API provides limited, non-configurable DNS resolution. When
a hostname is passed to ``TCPSocket`` or ``UDPSocket`` (connected mode), the
browser resolves it automatically using the OS resolver. The only control
available is ``dnsQueryType``, which selects between ``'ipv4'`` and ``'ipv6'``
record types. There is no support for custom DNS servers, caching control, or
DNSSEC validation.

For POSIX code that calls ``getaddrinfo()`` or ``gethostbyname()`` directly,
Emscripten's built-in ``DNS.lookup_name`` is used, which is limited to a static
address map and ``localhost``. This means hostname resolution outside of
``connect()``/``sendto()`` will fail for real hostnames unless the address map is
populated ahead of time. A DNS-over-HTTPS resolver is planned to lift this
limitation — see the companion PR for async DNS resolution via DoH.

WebTransport and QUIC
=====================

WebTransport may be used to send UDP like datagrams over QUIC.
Currently Emscripten does not provide a C/C++ API for interacting with WebTransport.
