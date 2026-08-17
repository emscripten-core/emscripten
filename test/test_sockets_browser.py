# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil

if __name__ == '__main__':
  raise Exception('do not run this file directly; do something like: test/runner sockets_browser')

import common
from browser_common import BrowserCore
from common import create_file, read_file
from decorators import (
  no_windows,
  parameterized,
  requires_dev_dependency,
  test_file,
)
from sockets_common import (
  EMTEST_SKIP_NODE_DEV_PACKAGES,
  EMTEST_SKIP_PYTHON_DEV_PACKAGES,
  BackgroundServerProcess,
  CompiledServerHarness,
  NodeJsWebSocketEchoServerProcess,
  PythonTcpEchoServerProcess,
  WebsockifyServerHarness,
  requires_python_dev_packages,
)

from tools.utils import path_from_root


class sockets_browser(BrowserCore):
  @classmethod
  def setUpClass(cls):
    super().setUpClass()
    print()
    print('Running the socket tests. Make sure the browser allows popups from localhost.')
    print()

  # Note: in the WebsockifyServerHarness and CompiledServerHarness tests below, explicitly use
  # consecutive server listen ports, because server teardown might not occur deterministically
  # (python dtor time) and is a bit racy.
  # WebsockifyServerHarness uses two port numbers, x and x-1, so increment it by two.
  # CompiledServerHarness only uses one. Start with 49160 & 49159 as the first server port
  # addresses. If adding new tests, increment the used port addresses below.
  @parameterized({
    'websockify': (WebsockifyServerHarness, 49160, ['-DTEST_DGRAM=0']),
    'tcp': (CompiledServerHarness, 49161, ['-DTEST_DGRAM=0']),
    'udp': (CompiledServerHarness, 49162, ['-DTEST_DGRAM=1']),
    # The following forces non-NULL addr and addlen parameters for the accept call
    'accept_addr': (CompiledServerHarness, 49163, ['-DTEST_DGRAM=0', '-DTEST_ACCEPT_ADDR=1']),
  })
  def test_sockets_echo(self, harness_class, port, args):
    if harness_class == WebsockifyServerHarness and common.EMTEST_LACKS_NATIVE_CLANG:
      self.skipTest('requires native clang')
    if harness_class == WebsockifyServerHarness and EMTEST_SKIP_PYTHON_DEV_PACKAGES:
      self.skipTest('requires python websockify and EMTEST_SKIP_PYTHON_DEV_PACKAGES=1')
    if harness_class == CompiledServerHarness and EMTEST_SKIP_NODE_DEV_PACKAGES:
      self.skipTest('requires node ws and EMTEST_SKIP_NODE_DEV_PACKAGES=1')

    with harness_class(test_file('sockets/test_sockets_echo_server.c'), args, port) as harness:
      self.btest_exit('sockets/test_sockets_echo_client.c', cflags=[f'-DSOCKK={harness.listen_port}', *args])

  @requires_dev_dependency('ws')
  def test_sockets_echo_pthreads(self):
    with CompiledServerHarness(test_file('sockets/test_sockets_echo_server.c'), [], 49161) as harness:
      self.btest_exit('sockets/test_sockets_echo_client.c', cflags=['-pthread', '-sPROXY_TO_PTHREAD', f'-DSOCKK={harness.listen_port}'])

  @requires_dev_dependency('ws')
  def test_sdl2_sockets_echo(self):
    with CompiledServerHarness('sockets/sdl2_net_server.c', ['-sUSE_SDL=2', '-sUSE_SDL_NET=2'], 49164) as harness:
      self.btest_exit('sockets/sdl2_net_client.c', cflags=['-sUSE_SDL=2', '-sUSE_SDL_NET=2', f'-DSOCKK={harness.listen_port}'])

  @parameterized({
    'websockify': (WebsockifyServerHarness, 49166, ['-DTEST_DGRAM=0']),
    'tcp': (CompiledServerHarness, 49167, ['-DTEST_DGRAM=0']),
    'udp': (CompiledServerHarness, 49168, ['-DTEST_DGRAM=1']),
    # The following forces non-NULL addr and addlen parameters for the accept call
    'accept_addr': (CompiledServerHarness, 49169, ['-DTEST_DGRAM=0', '-DTEST_ACCEPT_ADDR=1']),
  })
  def test_sockets_async_echo(self, harness_class, port, args):
    if harness_class == WebsockifyServerHarness and common.EMTEST_LACKS_NATIVE_CLANG:
      self.skipTest('requires native clang')
    if harness_class == WebsockifyServerHarness and EMTEST_SKIP_PYTHON_DEV_PACKAGES:
      self.skipTest('requires python websockify and EMTEST_SKIP_PYTHON_DEV_PACKAGES=1')
    if harness_class == CompiledServerHarness and EMTEST_SKIP_NODE_DEV_PACKAGES:
      self.skipTest('requires node ws and EMTEST_SKIP_NODE_DEV_PACKAGES=1')

    args.append('-DTEST_ASYNC=1')
    with harness_class(test_file('sockets/test_sockets_echo_server.c'), args, port) as harness:
      self.btest_exit('sockets/test_sockets_echo_client.c', cflags=[f'-DSOCKK={harness.listen_port}', *args])

  def test_sockets_async_bad_port(self):
    # Deliberately attempt a connection on a port that will fail to test the error callback and
    # getsockopt
    self.btest_exit('sockets/test_sockets_echo_client.c', cflags=['-DSOCKK=49169', '-DTEST_ASYNC=1'])

  @parameterized({
    'websockify': (WebsockifyServerHarness, 49171, ['-DTEST_DGRAM=0']),
    'tcp': (CompiledServerHarness, 49172, ['-DTEST_DGRAM=0']),
    'udp': (CompiledServerHarness, 49173, ['-DTEST_DGRAM=1']),
  })
  def test_sockets_echo_bigdata(self, harness_class, port, args):
    if harness_class == WebsockifyServerHarness and common.EMTEST_LACKS_NATIVE_CLANG:
      self.skipTest('requires native clang')
    if harness_class == WebsockifyServerHarness and EMTEST_SKIP_PYTHON_DEV_PACKAGES:
      self.skipTest('requires python websockify and EMTEST_SKIP_PYTHON_DEV_PACKAGES=1')
    if harness_class == CompiledServerHarness and EMTEST_SKIP_NODE_DEV_PACKAGES:
      self.skipTest('requires node ws and EMTEST_SKIP_NODE_DEV_PACKAGES=1')
    sockets_include = '-I' + test_file('sockets')

    # generate a large string literal to use as our message
    message = ''
    for i in range(256 * 256 * 2):
      message += str(chr(ord('a') + (i % 26)))

    # re-write the client test with this literal (it's too big to pass via command line)
    src = read_file(test_file('sockets/test_sockets_echo_client.c'))
    create_file('test_sockets_echo_bigdata.c', src.replace('#define MESSAGE "pingtothepong"', '#define MESSAGE "%s"' % message))

    with harness_class(test_file('sockets/test_sockets_echo_server.c'), args, port) as harness:
      self.btest_exit('test_sockets_echo_bigdata.c', cflags=[sockets_include, f'-DSOCKK={harness.listen_port}', *args])

  @no_windows('This test is Unix-specific.')
  @requires_python_dev_packages
  @requires_dev_dependency('ws')
  def test_sockets_partial(self):
    for harness in [
      WebsockifyServerHarness(test_file('sockets/test_sockets_partial_server.c'), [], 49180),
      CompiledServerHarness(test_file('sockets/test_sockets_partial_server.c'), [], 49181),
    ]:
      with harness:
        self.btest_exit('sockets/test_sockets_partial_client.c', assert_returncode=165, cflags=[f'-DSOCKK={harness.listen_port}'])

  @no_windows('This test is Unix-specific.')
  @requires_python_dev_packages
  @requires_dev_dependency('ws')
  def test_sockets_select_server_down(self):
    for harness in [
      WebsockifyServerHarness(test_file('sockets/test_sockets_select_server_down_server.c'), [], 49190, do_server_check=False),
      CompiledServerHarness(test_file('sockets/test_sockets_select_server_down_server.c'), [], 49191, do_server_check=False),
    ]:
      with harness:
        self.btest_exit('sockets/test_sockets_select_server_down_client.c', cflags=['-DSOCKK=%d' % harness.listen_port])

  @no_windows('This test is Unix-specific.')
  @requires_python_dev_packages
  @requires_dev_dependency('ws')
  def test_sockets_select_server_closes_connection_rw(self):
    for harness in [
      WebsockifyServerHarness(test_file('sockets/test_sockets_echo_server.c'), ['-DCLOSE_CLIENT_AFTER_ECHO'], 49200),
      CompiledServerHarness(test_file('sockets/test_sockets_echo_server.c'), ['-DCLOSE_CLIENT_AFTER_ECHO'], 49201),
    ]:
      with harness:
        self.btest_exit('sockets/test_sockets_select_server_closes_connection_client_rw.c', cflags=['-DSOCKK=%d' % harness.listen_port])

  @no_windows('This test uses Unix-specific build architecture.')
  @requires_dev_dependency('ws')
  def test_enet(self):
    # this is also a good test of raw usage of emconfigure and emmake
    shutil.copytree(test_file('third_party', 'enet'), 'enet')
    with common.chdir('enet'):
      self.run_process([common.EMCONFIGURE, './configure', '--disable-shared'])
      self.run_process([common.EMMAKE, 'make'])
      enet = [self.in_dir('enet', '.libs', 'libenet.a'), '-I' + self.in_dir('enet', 'include')]

    with CompiledServerHarness(test_file('sockets/test_enet_server.c'), enet, 49210) as harness:
      self.btest_exit('sockets/test_enet_client.c', cflags=[*enet, f'-DSOCKK={harness.listen_port}'])

  # Test Emscripten WebSockets API to send and receive text and binary messages against an echo server.
  # N.B. running this test requires 'npm install ws' in Emscripten root directory
  # NOTE: Shared buffer is not allowed for websocket sending.
  @parameterized({
    '': ([],),
    'shared': (['-sSHARED_MEMORY'],),
    'deinitialize': (['-DTEST_EMSCRIPTEN_WEBSOCKET_DEINITIALIZE'],),
  })
  @requires_dev_dependency('ws')
  def test_websocket_send(self, args):
    with NodeJsWebSocketEchoServerProcess():
      self.btest_exit('websocket/test_websocket_send.c', cflags=['-lwebsocket', '-sNO_EXIT_RUNTIME', '-sWEBSOCKET_DEBUG', *args])

  def test_websocket_new(self):
    self.btest_exit('websocket/test_websocket_new.c', cflags=['-lwebsocket'])

  # Test that native POSIX sockets API can be used by proxying calls to an intermediate WebSockets
  # -> POSIX sockets bridge server
  def test_posix_proxy_sockets(self):
    # Build the websocket bridge server
    self.run_process(['cmake', path_from_root('tools/websocket_to_posix_proxy')])
    self.run_process(['cmake', '--build', '.'])
    if os.name == 'nt': # This is not quite exact, instead of "isWindows()" this should be "If CMake defaults to building with Visual Studio", but there is no good check for that, so assume Windows==VS.
      proxy_server = self.in_dir('Debug', 'websocket_to_posix_proxy.exe')
    else:
      proxy_server = self.in_dir('websocket_to_posix_proxy')

    with BackgroundServerProcess([proxy_server, '8080']):
      with PythonTcpEchoServerProcess('7777'):
        # Build and run the TCP echo client program with Emscripten
        self.btest_exit('websocket/tcp_echo_client.c', cflags=['-lwebsocket', '-sPROXY_POSIX_SOCKETS', '-pthread', '-sPROXY_TO_PTHREAD'])

  # Test that calling send() right after a socket connect() works.
  @requires_dev_dependency('ws')
  def test_sockets_send_while_connecting(self):
    with NodeJsWebSocketEchoServerProcess():
      self.btest('sockets/test_sockets_send_while_connecting.c', cflags=['-DSOCKET_DEBUG'], expected='0')


class sockets_browser64(sockets_browser):
  def setUp(self):
    super().setUp()
    self.cflags.append('-m64')
    self.require_wasm64()
