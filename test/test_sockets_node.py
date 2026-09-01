# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import socket
import socketserver
import threading
from functools import wraps

if __name__ == '__main__':
  raise Exception('do not run this file directly; do something like: test/runner sockets_node')

import common
from common import NON_ZERO, RunnerCore, create_file
from decorators import (
  also_with_proxy_to_pthread,
  crossplatform,
  parameterized,
  requires_native_clang,
  test_file,
)
from sockets_common import (
  EMTEST_SKIP_NODE_DEV_PACKAGES,
  EMTEST_SKIP_PYTHON_DEV_PACKAGES,
  HAS_IPV6_LOOPBACK,
  CompiledServerHarness,
  EchoHandler,
  WebsockifyServerHarness,
  requires_python_dev_packages,
)

from tools.shared import EMCC


def requires_jspi_node(func):
  # require_jspi() falls back to the d8/SpiderMonkey shells when node is too
  # old, but these tests need node (real sockets via node:net), so handle the
  # node setup directly. The new JSPI API requires node >= 24, so skip below
  # that.
  assert callable(func)

  @wraps(func)
  def decorated(self, *args, **kwargs):
    if not common.check_node_version(24):
      self.skipTest('JSPI requires node >= 24')
    if not common.check_node_version(26):
      self.node_args += ['--experimental-wasm-stack-switching']
    self.cflags += ['-Wno-experimental']
    self.set_setting('JSPI')
    return func(self, *args, **kwargs)
  return decorated


class sockets_node(RunnerCore):
  # Note: in the WebsockifyServerHarness and CompiledServerHarness tests below, explicitly use
  # consecutive server listen ports, because server teardown might not occur deterministically
  # (python dtor time) and is a bit racy.
  # WebsockifyServerHarness uses two port numbers, x and x-1, so increment it by two.
  # CompiledServerHarness only uses one. If adding new tests, increment the used port
  # addresses below.
  @crossplatform
  @parameterized({
    'native': (WebsockifyServerHarness, 59160, ['-DTEST_DGRAM=0']),
    'tcp': (CompiledServerHarness, 59162, ['-DTEST_DGRAM=0', '-sEXPORT_ES6', '--extern-post-js', test_file('modularize_post_js.js')]),
    'udp': (CompiledServerHarness, 59164, ['-DTEST_DGRAM=1']),
    'pthread': (CompiledServerHarness, 59166, ['-pthread', '-sPROXY_TO_PTHREAD']),
  })
  def test_nodejs_sockets_echo(self, harness_class, port, args):
    if harness_class == WebsockifyServerHarness and common.EMTEST_LACKS_NATIVE_CLANG:
      self.skipTest('requires native clang')
    if harness_class == WebsockifyServerHarness and EMTEST_SKIP_PYTHON_DEV_PACKAGES:
      self.skipTest('requires python websockify and EMTEST_SKIP_PYTHON_DEV_PACKAGES=1')
    if harness_class == CompiledServerHarness and EMTEST_SKIP_NODE_DEV_PACKAGES:
      self.skipTest('requires node ws and EMTEST_SKIP_NODE_DEV_PACKAGES=1')

    # Basic test of node client against both a Websockified and compiled echo server.
    with harness_class(test_file('sockets/test_sockets_echo_server.c'), args, port) as harness:
      expected = 'do_msg_read: read 14 bytes'
      self.do_runf('sockets/test_sockets_echo_client.c', expected, cflags=[f'-DSOCKK={harness.listen_port}', *args])

  def test_nodejs_sockets_connect_failure(self):
    self.do_runf('sockets/test_sockets_echo_client.c', r'connect failed: (Connection refused|Host is unreachable)', regex=True, cflags=['-DSOCKK=666'], assert_returncode=NON_ZERO)

  def _run_against_echo_server(self, src):
    # Start a loopback TCP echo server on an ephemeral port and run the test
    # against it, passing the port as argv[1].
    server = socketserver.TCPServer(('127.0.0.1', 0), EchoHandler)
    port = server.server_address[1]
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
      self.do_runf(src, 'done\n', cflags=['-sNODERAWSOCKETS'], args=[str(port)])
    finally:
      server.shutdown()
      server.server_close()
      thread.join()

  # The proxy_to_pthread variant proves the backend works when socket syscalls
  # are proxied to the main thread: with PROXY_TO_PTHREAD, main() runs on a
  # worker and every socket call funnels to the main thread where node:net lives.
  @also_with_proxy_to_pthread
  def test_noderawsockets_echo(self):
    # With -sNODERAWSOCKETS the client does a non-blocking connect, send and
    # recv over a real OS socket against a loopback echo server we run here.
    self._run_against_echo_server('sockets/test_tcp_echo.c')

  def test_noderawsockets_client_bind(self):
    # A client that bind()s an explicit source port has it honored by connect(),
    # and the plain client path never realizes a private tcp_wrap handle. We
    # allocate a free source port here and pass it alongside the echo server's.
    # Reserve a free loopback port for the client's bound source port.
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(('127.0.0.1', 0))
    src_port = s.getsockname()[1]
    s.close()

    server = socketserver.TCPServer(('127.0.0.1', 0), EchoHandler)
    port = server.server_address[1]
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
      self.do_runf('sockets/test_tcp_client_bind.c', 'done\n', cflags=['-sNODERAWSOCKETS'], args=[str(port), str(src_port)])
    finally:
      server.shutdown()
      server.server_close()
      thread.join()

  def test_noderawsockets_connect_getsockname(self):
    # getsockname() immediately after a non-blocking connect() on an unbound
    # client reports the ephemeral source port synchronously (kernel semantics:
    # the port is assigned at connect(), not when the connection completes).
    self.do_runf('sockets/test_tcp_connect_getsockname.c', 'done\n', cflags=['-sNODERAWSOCKETS'])

  def test_noderawsockets_client_semantics(self):
    # EISCONN on a second connect, shutdown(SHUT_WR) leaving reads working,
    # EPIPE on a write after that, and POLLHUP after a full shutdown(SHUT_RDWR).
    self._run_against_echo_server('sockets/test_tcp_client_semantics.c')

  def test_noderawsockets_refused(self):
    # A connect to a loopback port with nothing listening reports ECONNREFUSED.
    self.do_runf('sockets/test_tcp_refused.c', 'done\n', cflags=['-sNODERAWSOCKETS'])

  def test_noderawsockets_backpressure(self):
    # A sink server that accepts but never reads, so the client's writes fill
    # the buffers and send() reports EAGAIN rather than buffering unboundedly.
    done = threading.Event()

    class SinkHandler(socketserver.BaseRequestHandler):
      def handle(self):
        done.wait(30) # hold the connection open without ever reading

    server = socketserver.TCPServer(('127.0.0.1', 0), SinkHandler)
    port = server.server_address[1]
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
      self.do_runf('sockets/test_tcp_backpressure.c', 'done\n', cflags=['-sNODERAWSOCKETS'], args=[str(port)])
    finally:
      done.set()
      server.shutdown()
      server.server_close()
      thread.join()

  @also_with_proxy_to_pthread
  def test_noderawsockets_server(self):
    # Self-contained loopback accept+echo, exercising bind(:0)+getsockname
    # (synchronous ephemeral port), listen, accept, non-blocking connect, send
    # and recv over real OS sockets via the tcp_wrap server path.
    self.do_runf('sockets/test_tcp_server.c', 'done\n', cflags=['-sNODERAWSOCKETS'])

  @also_with_proxy_to_pthread
  def test_noderawsockets_peek(self):
    # recv(MSG_PEEK) must leave the data buffered: a peek returns the bytes, the
    # socket stays readable, and the following plain recv returns them again.
    self.do_runf('sockets/test_tcp_peek.c', 'done\n', cflags=['-sNODERAWSOCKETS'])

  # AF_UNIX is gated behind NODERAWFS: the socket path lives in the host
  # filesystem, which only stays coherent with the program's own file syscalls
  # (bind's parent dir, getsockname, unlink) when the FS is the host FS.
  @also_with_proxy_to_pthread
  def test_noderawsockets_unix_server(self):
    # Self-contained named AF_UNIX (pathname) loopback accept+echo: bind(path),
    # listen, getsockname (the bound path), accept, getpeername, non-blocking
    # connect-by-path, send and recv over a real node pipe.
    self.do_runf('sockets/test_unix_server.c', 'done\n', cflags=['-sNODERAWSOCKETS', '-sNODERAWFS'])

  def test_noderawsockets_unix_refused(self):
    # A connect to an AF_UNIX path with no socket file reports ENOENT.
    self.do_runf('sockets/test_unix_refused.c', 'done\n', cflags=['-sNODERAWSOCKETS', '-sNODERAWFS'])

  def test_noderawsockets_unix_bind_inuse(self):
    # Binding an already-bound AF_UNIX path fails synchronously with EADDRINUSE.
    self.do_runf('sockets/test_unix_bind_inuse.c', 'done\n', cflags=['-sNODERAWSOCKETS', '-sNODERAWFS'])

  def test_noderawsockets_server_autobind(self):
    # listen() without a prior bind() must auto-bind an ephemeral port and
    # getsockname() must report it (POSIX), then accept+echo as usual.
    self.do_runf('sockets/test_tcp_server.c', 'done\n', cflags=['-sNODERAWSOCKETS', '-DNO_EXPLICIT_BIND'])

  def test_noderawsockets_tcp_ipv6(self):
    # Self-contained IPv6 TCP loopback accept+echo over ::1: bind(:0)+getsockname,
    # listen, accept, non-blocking connect, send/recv on AF_INET6 sockets.
    if not HAS_IPV6_LOOPBACK:
      self.skipTest('no IPv6 loopback available')
    self.do_runf('sockets/test_tcp_ipv6.c', 'done\n', cflags=['-sNODERAWSOCKETS'])

  def test_noderawsockets_udp_ipv6(self):
    # Self-contained IPv6 UDP loopback echo over ::1 on AF_INET6 sockets.
    if not HAS_IPV6_LOOPBACK:
      self.skipTest('no IPv6 loopback available')
    self.do_runf('sockets/test_udp_ipv6.c', 'done\n', cflags=['-sNODERAWSOCKETS'])

  def test_noderawsockets_epoll_socket_blocking(self):
    # A blocking epoll_wait() on a socket is woken by an incoming datagram
    # through the unified readiness wait-queue (the SOCKFS.emit bridge), with
    # main() proxied to a worker so the wait can suspend.
    self.do_runf('sockets/test_epoll_socket_blocking.c', 'done\n',
                 cflags=['-sNODERAWSOCKETS', '-pthread', '-sPROXY_TO_PTHREAD', '-sEXIT_RUNTIME'])

  @requires_jspi_node
  def test_noderawsockets_epoll_socket_blocking_jspi(self):
    # Same, but the blocking epoll_wait() suspends the wasm stack under JSPI.
    self.do_runf('sockets/test_epoll_socket_blocking.c', 'done\n',
                 cflags=['-sNODERAWSOCKETS', '-sEXIT_RUNTIME'])

  def test_noderawsockets_epoll_rdhup(self):
    # A blocking epoll_wait reports EPOLLRDHUP when the TCP peer half-closes its
    # write side (FIN), distinct from a full EPOLLHUP, and only when requested.
    self.do_runf('sockets/test_epoll_rdhup.c', 'done\n',
                 cflags=['-sNODERAWSOCKETS', '-pthread', '-sPROXY_TO_PTHREAD', '-sEXIT_RUNTIME'])

  @requires_jspi_node
  def test_noderawsockets_epoll_rdhup_jspi(self):
    # Same, but the blocking calls suspend the wasm stack under JSPI.
    self.do_runf('sockets/test_epoll_rdhup.c', 'done\n',
                 cflags=['-sNODERAWSOCKETS', '-sEXIT_RUNTIME'])

  @also_with_proxy_to_pthread
  def test_noderawsockets_udp(self):
    # Self-contained loopback UDP echo: the server binds(:0)+getsockname for its
    # ephemeral port, the client sends a datagram, the server echoes it back.
    self.do_runf('sockets/test_udp_echo.c', 'done\n', cflags=['-sNODERAWSOCKETS'])

  def test_noderawsockets_udp_recvmsg(self):
    # recvmsg scatters a datagram across multiple iovecs at the right offsets
    # and updates msg_namelen/msg_controllen/msg_flags in the caller's msghdr.
    self.do_runf('sockets/test_udp_recvmsg.c', 'done\n', cflags=['-sNODERAWSOCKETS'])

  def test_noderawsockets_mmsg(self):
    # sendmmsg batches two datagrams out, recvmmsg receives them back in one
    # call, updating msg_len per message.
    self.do_runf('sockets/test_udp_mmsg.c', 'done\n', cflags=['-sNODERAWSOCKETS'])

  @also_with_proxy_to_pthread
  def test_noderawsockets_udp_connect(self):
    # Connected UDP: sendto() with an address gives EISCONN, send() reaches the
    # peer, and datagrams from a non-peer socket are filtered out.
    self.do_runf('sockets/test_udp_connect.c', 'done\n', cflags=['-sNODERAWSOCKETS'])

  @also_with_proxy_to_pthread
  def test_noderawsockets_udp_sockopts(self):
    # UDP multicast socket options: IP_MULTICAST_TTL/LOOP and their IPv6
    # counterparts round-trip through set/getsockopt, with POSIX defaults
    # readable before any set. EXIT_RUNTIME so the plain synchronous main()
    # tears down the proxy worker on return (otherwise noExitRuntime keeps the
    # worker, and thus node, alive under PROXY_TO_PTHREAD).
    self.do_runf('sockets/test_udp_sockopts.c', 'done\n', cflags=['-sNODERAWSOCKETS', '-sEXIT_RUNTIME'])

  @also_with_proxy_to_pthread
  def test_noderawsockets_socket_options(self):
    # Socket metadata/options on a fresh socket: fstat reports S_ISSOCK, SO_TYPE
    # reports the socket type, and SO_LINGER round-trips a struct linger.
    self.do_runf('sockets/test_socket_options.c', 'done\n',
                 cflags=['-sNODERAWSOCKETS', '-sEXIT_RUNTIME'])

  @requires_native_clang
  @requires_python_dev_packages
  def test_nodejs_sockets_echo_subprotocol(self):
    # Test against a Websockified server with compile time configured WebSocket subprotocol. We use a Websockified
    # server because as long as the subprotocol list contains binary it will configure itself to accept binary
    # This test also checks that the connect url contains the correct subprotocols.
    with WebsockifyServerHarness(test_file('sockets/test_sockets_echo_server.c'), [], 59168):
      self.run_process([EMCC, '-Werror', test_file('sockets/test_sockets_echo_client.c'), '-o', 'client.js', '-sSOCKET_DEBUG', '-sWEBSOCKET_SUBPROTOCOL="base64, binary"', '-DSOCKK=59168'])

      out = self.run_js('client.js')
      self.assertContained('do_msg_read: read 14 bytes', out)
      self.assertContained(['connect: ws://127.0.0.1:59168, base64,binary', 'connect: ws://127.0.0.1:59168/, base64,binary'], out)

  @requires_native_clang
  @requires_python_dev_packages
  def test_nodejs_sockets_echo_subprotocol_runtime(self):
    # Test against a Websockified server with runtime WebSocket configuration. We specify both url and subprotocol.
    # In this test we have *deliberately* used the wrong port '-DSOCKK=12345' to configure the echo_client.c, so
    # the connection would fail without us specifying a valid WebSocket URL in the configuration.
    create_file('websocket_pre.js', '''
      var Module = {
        websocket: {
          url: 'ws://localhost:59168/testA/testB',
          subprotocol: 'text, base64, binary',
        }
      };
    ''')
    with WebsockifyServerHarness(test_file('sockets/test_sockets_echo_server.c'), [], 59168):
      self.run_process([EMCC, '-Werror', test_file('sockets/test_sockets_echo_client.c'), '-o', 'client.js', '--pre-js=websocket_pre.js', '-sSOCKET_DEBUG', '-DSOCKK=12345'])

      out = self.run_js('client.js')
      self.assertContained('do_msg_read: read 14 bytes', out)
      self.assertContained('connect: ws://localhost:59168/testA/testB, text,base64,binary', out)


class sockets_node64(sockets_node):
  def setUp(self):
    super().setUp()
    self.cflags.append('-m64')
    self.require_wasm64()
