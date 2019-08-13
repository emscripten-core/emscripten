# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function
import multiprocessing
import os
import socket
import shutil
import sys
import time

if __name__ == '__main__':
  raise Exception('do not run this file directly; do something like: tests/runner.py sockets')

import websockify
from runner import BrowserCore, no_windows, chdir
from tools import shared
from tools.shared import PYTHON, EMCC, NODE_JS, path_from_root, Popen, PIPE, WINDOWS, run_process, run_js, JS_ENGINES, CLANG_CC

npm_checked = False

NPM = os.path.join(os.path.dirname(NODE_JS[0]), 'npm.cmd' if WINDOWS else 'npm')


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
        p.kill() # SIGKILL
      except OSError:
        pass


class WebsockifyServerHarness(object):
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
      proc = run_process([CLANG_CC, path_from_root('tests', self.filename), '-o', 'server', '-DSOCKK=%d' % self.target_port] + shared.get_clang_native_args() + self.args, env=shared.get_clang_native_env(), stdout=PIPE, stderr=PIPE)
      print('Socket server build: out:', proc.stdout or '', '/ err:', proc.stderr or '')
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


class CompiledServerHarness(object):
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
      child = run_process(NODE_JS + ['-e', 'require("ws");'], check=False)
      assert child.returncode == 0, '"ws" node module not found.  you may need to run npm install'
      npm_checked = True

    # compile the server
    proc = run_process([PYTHON, EMCC, path_from_root('tests', self.filename), '-o', 'server.js', '-DSOCKK=%d' % self.listen_port] + self.args)
    print('Socket server build: out:', proc.stdout or '', '/ err:', proc.stderr or '')

    process = Popen(NODE_JS + ['server.js'])
    self.processes.append(process)

  def __exit__(self, *args, **kwargs):
    # clean up any processes we started
    clean_processes(self.processes)

    # always run these tests last
    # make sure to use different ports in each one because it takes a while for the processes to be cleaned up


class sockets(BrowserCore):
  emcc_args = []

  @classmethod
  def setUpClass(cls):
    super(sockets, cls).setUpClass()
    print()
    print('Running the socket tests. Make sure the browser allows popups from localhost.')
    print()

  def test_sockets_echo(self):
    sockets_include = '-I' + path_from_root('tests', 'sockets')

    # Note: in the WebsockifyServerHarness and CompiledServerHarness tests below, explicitly use consecutive server listen ports,
    # because server teardown might not occur deterministically (python dtor time) and is a bit racy.
    # WebsockifyServerHarness uses two port numbers, x and x-1, so increment it by two.
    # CompiledServerHarness only uses one. Start with 49160 & 49159 as the first server port addresses. If adding new tests,
    # increment the used port addresses below.

    # Websockify-proxied servers can't run dgram tests
    harnesses = [
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0'], 49161), 0),
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=1'], 49162), 1),
      # The following forces non-NULL addr and addlen parameters for the accept call
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0', '-DTEST_ACCEPT_ADDR=1'], 49163), 0)
    ]

    if not WINDOWS: # TODO: Python pickling bug causes WebsockifyServerHarness to not work on Windows.
      harnesses += [(WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include], 49160), 0)]

    for harness, datagram in harnesses:
      with harness:
        self.btest(os.path.join('sockets', 'test_sockets_echo_client.c'), expected='0', args=['-DSOCKK=%d' % harness.listen_port, '-DTEST_DGRAM=%d' % datagram, sockets_include])

  def test_sdl2_sockets_echo(self):
    harness = CompiledServerHarness('sdl2_net_server.c', ['-s', 'USE_SDL=2', '-s', 'USE_SDL_NET=2'], 49164)
    with harness:
      self.btest('sdl2_net_client.c', expected='0', args=['-s', 'USE_SDL=2', '-s', 'USE_SDL_NET=2', '-DSOCKK=%d' % harness.listen_port])

  def test_sockets_async_echo(self):
    # Run with ./runner.py sockets.test_sockets_async_echo
    sockets_include = '-I' + path_from_root('tests', 'sockets')

    # Websockify-proxied servers can't run dgram tests
    harnesses = [
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0', '-DTEST_ASYNC=1'], 49167), 0),
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=1', '-DTEST_ASYNC=1'], 49168), 1),
      # The following forces non-NULL addr and addlen parameters for the accept call
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0', '-DTEST_ACCEPT_ADDR=1', '-DTEST_ASYNC=1'], 49169), 0)
    ]

    if not WINDOWS: # TODO: Python pickling bug causes WebsockifyServerHarness to not work on Windows.
      harnesses += [(WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_ASYNC=1'], 49166), 0)]

    for harness, datagram in harnesses:
      print('harness:', harness)
      with harness:
        self.btest(os.path.join('sockets', 'test_sockets_echo_client.c'), expected='0', args=['-DSOCKK=%d' % harness.listen_port, '-DTEST_DGRAM=%d' % datagram, '-DTEST_ASYNC=1', sockets_include])

    # Deliberately attempt a connection on a port that will fail to test the error callback and getsockopt
    print('expect fail')
    self.btest(os.path.join('sockets', 'test_sockets_echo_client.c'), expected='0', args=['-DSOCKK=49169', '-DTEST_ASYNC=1', sockets_include])

  def test_sockets_echo_bigdata(self):
    sockets_include = '-I' + path_from_root('tests', 'sockets')

    # generate a large string literal to use as our message
    message = ''
    for i in range(256 * 256 * 2):
        message += str(chr(ord('a') + (i % 26)))

    # re-write the client test with this literal (it's too big to pass via command line)
    input_filename = path_from_root('tests', 'sockets', 'test_sockets_echo_client.c')
    input = open(input_filename).read()
    output = input.replace('#define MESSAGE "pingtothepong"', '#define MESSAGE "%s"' % message)

    harnesses = [
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0'], 49172), 0),
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=1'], 49173), 1)
    ]

    if not WINDOWS: # TODO: Python pickling bug causes WebsockifyServerHarness to not work on Windows.
      harnesses += [(WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include], 49171), 0)]

    for harness, datagram in harnesses:
      with harness:
        self.btest(output, expected='0', args=[sockets_include, '-DSOCKK=%d' % harness.listen_port, '-DTEST_DGRAM=%d' % datagram], force_c=True)

  @no_windows('This test is Unix-specific.')
  def test_sockets_partial(self):
    for harness in [
      WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_partial_server.c'), [], 49180),
      CompiledServerHarness(os.path.join('sockets', 'test_sockets_partial_server.c'), [], 49181)
    ]:
      with harness:
        self.btest(os.path.join('sockets', 'test_sockets_partial_client.c'), expected='165', args=['-DSOCKK=%d' % harness.listen_port])

  @no_windows('This test is Unix-specific.')
  def test_sockets_select_server_down(self):
    for harness in [
      WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_select_server_down_server.c'), [], 49190, do_server_check=False),
      CompiledServerHarness(os.path.join('sockets', 'test_sockets_select_server_down_server.c'), [], 49191)
    ]:
      with harness:
        self.btest(os.path.join('sockets', 'test_sockets_select_server_down_client.c'), expected='266', args=['-DSOCKK=%d' % harness.listen_port])

  @no_windows('This test is Unix-specific.')
  def test_sockets_select_server_closes_connection_rw(self):
    sockets_include = '-I' + path_from_root('tests', 'sockets')

    for harness in [
      WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DCLOSE_CLIENT_AFTER_ECHO'], 49200),
      CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DCLOSE_CLIENT_AFTER_ECHO'], 49201)
    ]:
      with harness:
        self.btest(os.path.join('sockets', 'test_sockets_select_server_closes_connection_client_rw.c'), expected='266', args=[sockets_include, '-DSOCKK=%d' % harness.listen_port])

  @no_windows('This test uses Unix-specific build architecture.')
  def test_enet(self):
    # this is also a good test of raw usage of emconfigure and emmake
    shared.try_delete('enet')
    shutil.copytree(path_from_root('tests', 'enet'), 'enet')
    with chdir('enet'):
      run_process([PYTHON, path_from_root('emconfigure'), './configure'])
      run_process([PYTHON, path_from_root('emmake'), 'make'])
      enet = [self.in_dir('enet', '.libs', 'libenet.a'), '-I' + path_from_root('tests', 'enet', 'include')]

    for harness in [
      CompiledServerHarness(os.path.join('sockets', 'test_enet_server.c'), enet, 49210)
    ]:
      with harness:
        self.btest(os.path.join('sockets', 'test_enet_client.c'), expected='0', args=enet + ['-DSOCKK=%d' % harness.listen_port])

  # This test is no longer in use for WebSockets as we can't truly emulate
  # a server in the browser (in the past, there were some hacks to make it
  # somewhat work, but those have been removed). However, with WebRTC it
  # should be able to resurect this test.
  # def test_enet_in_browser(self):
  #   shared.try_delete('enet')
  #   shutil.copytree(path_from_root('tests', 'enet'), 'enet')
  #   pwd = os.getcwd()
  #   os.chdir('enet')
  #   run_process([PYTHON, path_from_root('emconfigure'), './configure'])
  #   run_process([PYTHON, path_from_root('emmake'), 'make'])
  #   enet = [self.in_dir('enet', '.libs', 'libenet.a'), '-I' + path_from_root('tests', 'enet', 'include')]
  #   os.chdir(pwd)
  #   run_process([PYTHON, EMCC, path_from_root('tests', 'sockets', 'test_enet_server.c'), '-o', 'server.html', '-DSOCKK=2235'] + enet)
  #   def make_relay_server(port1, port2):
  #     print('creating relay server on ports %d,%d' % (port1, port2), file=sys.stderr)
  #     proc = run_process([PYTHON, path_from_root('tests', 'sockets', 'socket_relay.py'), str(port1), str(port2)])
  #     return proc
  #   with WebsockifyServerHarness('', [], 2235, 2234):
  #     with WebsockifyServerHarness('', [], 2237, 2236):
  #       pids = []
  #       try:
  #         proc = make_relay_server(2234, 2236)
  #         pids.append(proc.pid)
  #         self.btest(os.path.join('sockets', 'test_enet_client.c'), expected='0', args=['-DSOCKK=2237', '-DUSE_IFRAME=1'] + enet)
  #       finally:
  #         clean_pids(pids);

  def test_webrtc(self): # XXX see src/settings.js, this is disabled pending investigation
    self.skipTest('WebRTC support is not up to date.')
    host_src = 'webrtc_host.c'
    peer_src = 'webrtc_peer.c'

    host_outfile = 'host.html'
    peer_outfile = 'peer.html'

    host_filepath = path_from_root('tests', 'sockets', host_src)
    temp_host_filepath = os.path.join(self.get_dir(), os.path.basename(host_src))
    with open(host_filepath) as f:
      host_src = f.read()
    with open(temp_host_filepath, 'w') as f:
      f.write(self.with_report_result(host_src))

    peer_filepath = path_from_root('tests', 'sockets', peer_src)
    temp_peer_filepath = os.path.join(self.get_dir(), os.path.basename(peer_src))
    with open(peer_filepath) as f:
      peer_src = f.read()
    with open(temp_peer_filepath, 'w') as f:
      f.write(self.with_report_result(peer_src))

    open(os.path.join(self.get_dir(), 'host_pre.js'), 'w').write('''
      var Module = {
        webrtc: {
          broker: 'http://localhost:8182',
          session: undefined,
          onpeer: function(peer, route) {
            window.open('http://localhost:8888/peer.html?' + route);
            // iframe = document.createElement("IFRAME");
            // iframe.setAttribute("src", "http://localhost:8888/peer.html?" + route);
            // iframe.style.display = "none";
            // document.body.appendChild(iframe);
            peer.listen();
          },
          onconnect: function(peer) {
          },
          ondisconnect: function(peer) {
          },
          onerror: function(error) {
            console.error(error);
          }
        },
        setStatus: function(text) {
          console.log('status: ' + text);
        }
      };
    ''')

    open(os.path.join(self.get_dir(), 'peer_pre.js'), 'w').write('''
      var Module = {
        webrtc: {
          broker: 'http://localhost:8182',
          session: window.location.toString().split('?')[1],
          onpeer: function(peer, route) {
            peer.connect(Module['webrtc']['session']);
          },
          onconnect: function(peer) {
          },
          ondisconnect: function(peer) {
            // Calling window.close() from this handler hangs my browser, so run it in the next turn
            setTimeout(window.close, 0);
          },
          onerror: function(error) {
            console.error(error);
          },
        },
        setStatus: function(text) {
          console.log('status: ' + text);
        }
      };
    ''')

    run_process([PYTHON, EMCC, temp_host_filepath, '-o', host_outfile] + ['-s', 'GL_TESTING=1', '--pre-js', 'host_pre.js', '-s', 'SOCKET_WEBRTC=1', '-s', 'SOCKET_DEBUG=1'])
    run_process([PYTHON, EMCC, temp_peer_filepath, '-o', peer_outfile] + ['-s', 'GL_TESTING=1', '--pre-js', 'peer_pre.js', '-s', 'SOCKET_WEBRTC=1', '-s', 'SOCKET_DEBUG=1'])

    # note: you may need to run this manually yourself, if npm is not in the path, or if you need a version that is not in the path
    run_process([NPM, 'install', path_from_root('tests', 'sockets', 'p2p')])
    broker = Popen(NODE_JS + [path_from_root('tests', 'sockets', 'p2p', 'broker', 'p2p-broker.js')])

    expected = '1'
    self.run_browser(host_outfile, '.', ['/report_result?' + e for e in expected])

    broker.kill()

  def test_nodejs_sockets_echo(self):
    # This test checks that sockets work when the client code is run in Node.js
    # Run with ./runner.py sockets.test_nodejs_sockets_echo
    if NODE_JS not in JS_ENGINES:
      self.skipTest('node is not present')

    sockets_include = '-I' + path_from_root('tests', 'sockets')

    harnesses = [
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0'], 59162), 0),
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=1'], 59164), 1)
    ]

    if not WINDOWS: # TODO: Python pickling bug causes WebsockifyServerHarness to not work on Windows.
      harnesses += [(WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include], 59160), 0)]

    # Basic test of node client against both a Websockified and compiled echo server.
    for harness, datagram in harnesses:
      with harness:
        run_process([PYTHON, EMCC, path_from_root('tests', 'sockets', 'test_sockets_echo_client.c'), '-o', 'client.js', '-DSOCKK=%d' % harness.listen_port, '-DTEST_DGRAM=%d' % datagram], stdout=PIPE, stderr=PIPE)

        out = run_js('client.js', engine=NODE_JS, full_output=True)
        self.assertContained('do_msg_read: read 14 bytes', out)

    if not WINDOWS: # TODO: Python pickling bug causes WebsockifyServerHarness to not work on Windows.
      # Test against a Websockified server with compile time configured WebSocket subprotocol. We use a Websockified
      # server because as long as the subprotocol list contains binary it will configure itself to accept binary
      # This test also checks that the connect url contains the correct subprotocols.
      print("\nTesting compile time WebSocket configuration.\n")
      for harness in [
        WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include], 59166)
      ]:
        with harness:
          run_process([PYTHON, EMCC, path_from_root('tests', 'sockets', 'test_sockets_echo_client.c'), '-o', 'client.js', '-s', 'SOCKET_DEBUG=1', '-s', 'WEBSOCKET_SUBPROTOCOL="base64, binary"', '-DSOCKK=59166'], stdout=PIPE, stderr=PIPE)

          out = run_js('client.js', engine=NODE_JS, full_output=True)
          self.assertContained('do_msg_read: read 14 bytes', out)
          self.assertContained(['connect: ws://127.0.0.1:59166, base64,binary', 'connect: ws://127.0.0.1:59166/, base64,binary'], out)

      # Test against a Websockified server with runtime WebSocket configuration. We specify both url and subprotocol.
      # In this test we have *deliberately* used the wrong port '-DSOCKK=12345' to configure the echo_client.c, so
      # the connection would fail without us specifying a valid WebSocket URL in the configuration.
      print("\nTesting runtime WebSocket configuration.\n")
      for harness in [
        WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include], 59168)
      ]:
        with harness:
          open(os.path.join(self.get_dir(), 'websocket_pre.js'), 'w').write('''
          var Module = {
            websocket: {
              url: 'ws://localhost:59168/testA/testB',
              subprotocol: 'text, base64, binary',
            }
          };
          ''')

          run_process([PYTHON, EMCC, path_from_root('tests', 'sockets', 'test_sockets_echo_client.c'), '-o', 'client.js', '--pre-js', 'websocket_pre.js', '-s', 'SOCKET_DEBUG=1', '-DSOCKK=12345'], stdout=PIPE, stderr=PIPE)

          out = run_js('client.js', engine=NODE_JS, full_output=True)
          self.assertContained('do_msg_read: read 14 bytes', out)
          self.assertContained('connect: ws://localhost:59168/testA/testB, text,base64,binary', out)
