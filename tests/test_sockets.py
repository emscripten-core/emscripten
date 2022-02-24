# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import multiprocessing
import os
import socket
import shutil
import sys
import time
from subprocess import Popen, PIPE

if __name__ == '__main__':
  raise Exception('do not run this file directly; do something like: tests/runner sockets')

try:
  import websockify
except Exception:
  # websockify won't successfully import on Windows under Python3, because socketserver.py doesn't export ForkingMixIn.
  # (On python2, ForkingMixIn was exported but it didn't actually work on Windows).
  # Swallowing the error here means that this file can always be imported, but won't work if actually used on Windows,
  # which is the same behavior as before.
  pass
import clang_native
from common import BrowserCore, no_windows, create_file, test_file, read_file
from tools import shared, config, utils
from tools.shared import PYTHON, EMCC, path_from_root, WINDOWS, run_process, CLANG_CC

npm_checked = False


def clean_processes(processes):
  for p in processes:
    if (not hasattr(p, 'exitcode') or p.exitcode is None) and (not hasattr(p, 'returncode') or p.returncode is None):
      # ask nicely (to try and catch the children)
      try:
        p.terminate() # SIGTERM
      except OSError:
        pass
      time.sleep(1)
      # send a forcible kill immediately afterwards. If the process did not die before, this should clean it.
      try:
        p.terminate() # SIGKILL
      except OSError:
        pass


class WebsockifyServerHarness():
  def __init__(self, filename, args, listen_port, do_server_check=True):
    self.processes = []
    self.filename = filename
    self.listen_port = listen_port
    self.target_port = listen_port - 1
    self.args = args or []
    self.do_server_check = do_server_check

  def __enter__(self):
    # compile the server
    # NOTE empty filename support is a hack to support
    # the current test_enet
    if self.filename:
      proc = run_process([CLANG_CC, test_file(self.filename), '-o', 'server', '-DSOCKK=%d' % self.target_port] + clang_native.get_clang_native_args() + self.args, clang_native.get_clang_native_env(), stdout=PIPE, stderr=PIPE)
      print('Socket server build: out:', proc.stdout, '/ err:', proc.stderr)
      process = Popen([os.path.abspath('server')])
      self.processes.append(process)

    # start the websocket proxy
    print('running websockify on %d, forward to tcp %d' % (self.listen_port, self.target_port), file=sys.stderr)
    wsp = websockify.WebSocketProxy(verbose=True, listen_port=self.listen_port, target_host="127.0.0.1", target_port=self.target_port, run_once=True)
    self.websockify = multiprocessing.Process(target=wsp.start_server)
    self.websockify.start()
    self.processes.append(self.websockify)
    # Make sure both the actual server and the websocket proxy are running
    for i in range(10):
      try:
        if self.do_server_check:
            server_sock = socket.create_connection(('localhost', self.target_port), timeout=1)
            server_sock.close()
        proxy_sock = socket.create_connection(('localhost', self.listen_port), timeout=1)
        proxy_sock.close()
        break
      except IOError:
        time.sleep(1)
    else:
      clean_processes(self.processes)
      raise Exception('[Websockify failed to start up in a timely manner]')

    print('[Websockify on process %s]' % str(self.processes[-2:]))

  def __exit__(self, *args, **kwargs):
    # try to kill the websockify proxy gracefully
    if self.websockify.is_alive():
      self.websockify.terminate()
    self.websockify.join()

    # clean up any processes we started
    clean_processes(self.processes)


class CompiledServerHarness():
  def __init__(self, filename, args, listen_port):
    self.processes = []
    self.filename = filename
    self.listen_port = listen_port
    self.args = args or []

  def __enter__(self):
    # assuming this is only used for WebSocket tests at the moment, validate that
    # the ws module is installed
    global npm_checked
    if not npm_checked:
      child = run_process(config.NODE_JS + ['-e', 'require("ws");'], check=False)
      assert child.returncode == 0, '"ws" node module not found.  you may need to run npm install'
      npm_checked = True

    # compile the server
    proc = run_process([EMCC, '-Werror', test_file(self.filename), '-o', 'server.js', '-DSOCKK=%d' % self.listen_port] + self.args)
    print('Socket server build: out:', proc.stdout or '', '/ err:', proc.stderr or '')

    process = Popen(config.NODE_JS + ['server.js'])
    self.processes.append(process)

  def __exit__(self, *args, **kwargs):
    # clean up any processes we started
    clean_processes(self.processes)

    # always run these tests last
    # make sure to use different ports in each one because it takes a while for the processes to be cleaned up


# Executes a native executable server process
class BackgroundServerProcess():
  def __init__(self, args):
    self.processes = []
    self.args = args

  def __enter__(self):
    print('Running background server: ' + str(self.args))
    process = Popen(self.args)
    self.processes.append(process)

  def __exit__(self, *args, **kwargs):
    clean_processes(self.processes)


def NodeJsWebSocketEchoServerProcess():
  return BackgroundServerProcess(config.NODE_JS + [test_file('websocket/nodejs_websocket_echo_server.js')])


def PythonTcpEchoServerProcess(port):
  return BackgroundServerProcess([PYTHON, test_file('websocket/tcp_echo_server.py'), port])


class sockets(BrowserCore):
  emcc_args = []

  @classmethod
  def setUpClass(cls):
    super().setUpClass()
    print()
    print('Running the socket tests. Make sure the browser allows popups from localhost.')
    print()

    # Use emscripten root for node module lookup.  This is needed because the unit tests each
    # run with CWD set to a temporary directory outside the emscripten tree.
    print('Setting NODE_PATH=' + path_from_root('node_modules'))
    os.environ['NODE_PATH'] = path_from_root('node_modules')

  def test_sockets_echo(self, extra_args=[]):
    sockets_include = '-I' + test_file('sockets')

    # Note: in the WebsockifyServerHarness and CompiledServerHarness tests below, explicitly use consecutive server listen ports,
    # because server teardown might not occur deterministically (python dtor time) and is a bit racy.
    # WebsockifyServerHarness uses two port numbers, x and x-1, so increment it by two.
    # CompiledServerHarness only uses one. Start with 49160 & 49159 as the first server port addresses. If adding new tests,
    # increment the used port addresses below.

    # Websockify-proxied servers can't run dgram tests
    harnesses = [
      (CompiledServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0'], 49161), 0),
      (CompiledServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=1'], 49162), 1),
      # The following forces non-NULL addr and addlen parameters for the accept call
      (CompiledServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0', '-DTEST_ACCEPT_ADDR=1'], 49163), 0)
    ]

    if not WINDOWS: # TODO: Python pickling bug causes WebsockifyServerHarness to not work on Windows.
      harnesses += [(WebsockifyServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include], 49160), 0)]

    for harness, datagram in harnesses:
      with harness:
        self.btest_exit(test_file('sockets/test_sockets_echo_client.c'), args=['-DSOCKK=%d' % harness.listen_port, '-DTEST_DGRAM=%d' % datagram, sockets_include])

  def test_sockets_echo_pthreads(self, extra_args=[]):
    self.test_sockets_echo(['-sUSE_PTHREADS', '-sPROXY_TO_PTHREAD'])

  def test_sdl2_sockets_echo(self):
    harness = CompiledServerHarness('sdl2_net_server.c', ['-sUSE_SDL=2', '-sUSE_SDL_NET=2'], 49164)
    with harness:
      self.btest_exit('sdl2_net_client.c', args=['-sUSE_SDL=2', '-sUSE_SDL_NET=2', '-DSOCKK=%d' % harness.listen_port])

  def test_sockets_async_echo(self):
    sockets_include = '-I' + test_file('sockets')

    # Websockify-proxied servers can't run dgram tests
    harnesses = [
      (CompiledServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0', '-DTEST_ASYNC=1'], 49167), 0),
      (CompiledServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=1', '-DTEST_ASYNC=1'], 49168), 1),
      # The following forces non-NULL addr and addlen parameters for the accept call
      (CompiledServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0', '-DTEST_ACCEPT_ADDR=1', '-DTEST_ASYNC=1'], 49169), 0)
    ]

    if not WINDOWS: # TODO: Python pickling bug causes WebsockifyServerHarness to not work on Windows.
      harnesses += [(WebsockifyServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include, '-DTEST_ASYNC=1'], 49166), 0)]

    for harness, datagram in harnesses:
      print('harness:', harness)
      with harness:
        self.btest_exit(test_file('sockets/test_sockets_echo_client.c'), args=['-DSOCKK=%d' % harness.listen_port, '-DTEST_DGRAM=%d' % datagram, '-DTEST_ASYNC=1', sockets_include])
        return

    # Deliberately attempt a connection on a port that will fail to test the error callback and getsockopt
    print('expect fail')
    self.btest_exit(test_file('sockets/test_sockets_echo_client.c'), args=['-DSOCKK=49169', '-DTEST_ASYNC=1', sockets_include])

  def test_sockets_echo_bigdata(self):
    sockets_include = '-I' + test_file('sockets')

    # generate a large string literal to use as our message
    message = ''
    for i in range(256 * 256 * 2):
      message += str(chr(ord('a') + (i % 26)))

    # re-write the client test with this literal (it's too big to pass via command line)
    src = read_file(test_file('sockets/test_sockets_echo_client.c'))
    create_file('test_sockets_echo_bigdata.c', src.replace('#define MESSAGE "pingtothepong"', '#define MESSAGE "%s"' % message))

    harnesses = [
      (CompiledServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0'], 49172), 0),
      (CompiledServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=1'], 49173), 1)
    ]

    if not WINDOWS: # TODO: Python pickling bug causes WebsockifyServerHarness to not work on Windows.
      harnesses += [(WebsockifyServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include], 49171), 0)]

    for harness, datagram in harnesses:
      with harness:
        self.btest_exit('test_sockets_echo_bigdata.c', args=[sockets_include, '-DSOCKK=%d' % harness.listen_port, '-DTEST_DGRAM=%d' % datagram])

  @no_windows('This test is Unix-specific.')
  def test_sockets_partial(self):
    for harness in [
      WebsockifyServerHarness(test_file('sockets/test_sockets_partial_server.c'), [], 49180),
      CompiledServerHarness(test_file('sockets/test_sockets_partial_server.c'), [], 49181)
    ]:
      with harness:
        self.btest_exit(test_file('sockets/test_sockets_partial_client.c'), assert_returncode=165, args=['-DSOCKK=%d' % harness.listen_port])

  @no_windows('This test is Unix-specific.')
  def test_sockets_select_server_down(self):
    for harness in [
      WebsockifyServerHarness(test_file('sockets/test_sockets_select_server_down_server.c'), [], 49190, do_server_check=False),
      CompiledServerHarness(test_file('sockets/test_sockets_select_server_down_server.c'), [], 49191)
    ]:
      with harness:
        self.btest_exit(test_file('sockets/test_sockets_select_server_down_client.c'), args=['-DSOCKK=%d' % harness.listen_port])

  @no_windows('This test is Unix-specific.')
  def test_sockets_select_server_closes_connection_rw(self):
    sockets_include = '-I' + test_file('sockets')

    for harness in [
      WebsockifyServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include, '-DCLOSE_CLIENT_AFTER_ECHO'], 49200),
      CompiledServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include, '-DCLOSE_CLIENT_AFTER_ECHO'], 49201)
    ]:
      with harness:
        self.btest_exit(test_file('sockets/test_sockets_select_server_closes_connection_client_rw.c'), args=[sockets_include, '-DSOCKK=%d' % harness.listen_port])

  @no_windows('This test uses Unix-specific build architecture.')
  def test_enet(self):
    # this is also a good test of raw usage of emconfigure and emmake
    shared.try_delete('enet')
    shutil.copytree(test_file('third_party', 'enet'), 'enet')
    with utils.chdir('enet'):
      self.run_process([path_from_root('emconfigure'), './configure', '--disable-shared'])
      self.run_process([path_from_root('emmake'), 'make'])
      enet = [self.in_dir('enet', '.libs', 'libenet.a'), '-I' + self.in_dir('enet', 'include')]

    for harness in [
      CompiledServerHarness(test_file('sockets/test_enet_server.c'), enet, 49210)
    ]:
      with harness:
        self.btest_exit(test_file('sockets/test_enet_client.c'), args=enet + ['-DSOCKK=%d' % harness.listen_port])

  def test_nodejs_sockets_echo(self):
    # This test checks that sockets work when the client code is run in Node.js
    if config.NODE_JS not in config.JS_ENGINES:
      self.skipTest('node is not present')

    sockets_include = '-I' + test_file('sockets')

    harnesses = [
      (CompiledServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0'], 59162), 0),
      (CompiledServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=1'], 59164), 1)
    ]

    if not WINDOWS: # TODO: Python pickling bug causes WebsockifyServerHarness to not work on Windows.
      harnesses += [(WebsockifyServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include], 59160), 0)]

    # Basic test of node client against both a Websockified and compiled echo server.
    for harness, datagram in harnesses:
      with harness:
        expected = 'do_msg_read: read 14 bytes'
        self.do_runf(test_file('sockets/test_sockets_echo_client.c'), expected, emcc_args=['-DSOCKK=%d' % harness.listen_port, '-DTEST_DGRAM=%d' % datagram])

    if not WINDOWS: # TODO: Python pickling bug causes WebsockifyServerHarness to not work on Windows.
      # Test against a Websockified server with compile time configured WebSocket subprotocol. We use a Websockified
      # server because as long as the subprotocol list contains binary it will configure itself to accept binary
      # This test also checks that the connect url contains the correct subprotocols.
      print("\nTesting compile time WebSocket configuration.\n")
      for harness in [
        WebsockifyServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include], 59166)
      ]:
        with harness:
          self.run_process([EMCC, '-Werror', test_file('sockets/test_sockets_echo_client.c'), '-o', 'client.js', '-sSOCKET_DEBUG', '-sWEBSOCKET_SUBPROTOCOL="base64, binary"', '-DSOCKK=59166'])

          out = self.run_js('client.js')
          self.assertContained('do_msg_read: read 14 bytes', out)
          self.assertContained(['connect: ws://127.0.0.1:59166, base64,binary', 'connect: ws://127.0.0.1:59166/, base64,binary'], out)

      # Test against a Websockified server with runtime WebSocket configuration. We specify both url and subprotocol.
      # In this test we have *deliberately* used the wrong port '-DSOCKK=12345' to configure the echo_client.c, so
      # the connection would fail without us specifying a valid WebSocket URL in the configuration.
      print("\nTesting runtime WebSocket configuration.\n")
      create_file('websocket_pre.js', '''
        var Module = {
          websocket: {
            url: 'ws://localhost:59168/testA/testB',
            subprotocol: 'text, base64, binary',
          }
        };
      ''')
      for harness in [
        WebsockifyServerHarness(test_file('sockets/test_sockets_echo_server.c'), [sockets_include], 59168)
      ]:
        with harness:
          self.run_process([EMCC, '-Werror', test_file('sockets/test_sockets_echo_client.c'), '-o', 'client.js', '--pre-js=websocket_pre.js', '-sSOCKET_DEBUG', '-DSOCKK=12345'])

          out = self.run_js('client.js')
          self.assertContained('do_msg_read: read 14 bytes', out)
          self.assertContained('connect: ws://localhost:59168/testA/testB, text,base64,binary', out)

  # Test Emscripten WebSockets API to send and receive text and binary messages against an echo server.
  # N.B. running this test requires 'npm install ws' in Emscripten root directory
  def test_websocket_send(self):
    with NodeJsWebSocketEchoServerProcess():
      self.btest_exit(test_file('websocket/test_websocket_send.c'), args=['-lwebsocket', '-sNO_EXIT_RUNTIME', '-sWEBSOCKET_DEBUG'])

  # Test that native POSIX sockets API can be used by proxying calls to an intermediate WebSockets -> POSIX sockets bridge server
  def test_posix_proxy_sockets(self):
    # Build the websocket bridge server
    self.run_process(['cmake', path_from_root('tools/websocket_to_posix_proxy')])
    self.run_process(['cmake', '--build', '.'])
    if os.name == 'nt': # This is not quite exact, instead of "isWindows()" this should be "If CMake defaults to building with Visual Studio", but there is no good check for that, so assume Windows==VS.
      proxy_server = os.path.join(self.get_dir(), 'Debug', 'websocket_to_posix_proxy.exe')
    else:
      proxy_server = os.path.join(self.get_dir(), 'websocket_to_posix_proxy')

    with BackgroundServerProcess([proxy_server, '8080']):
      with PythonTcpEchoServerProcess('7777'):
        # Build and run the TCP echo client program with Emscripten
        self.btest_exit(test_file('websocket/tcp_echo_client.cpp'), args=['-lwebsocket', '-sPROXY_POSIX_SOCKETS', '-sUSE_PTHREADS', '-sPROXY_TO_PTHREAD'])
