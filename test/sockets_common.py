# Copyright 2026 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Shared server harnesses and helpers for the sockets test suites.

Used by test_sockets_node.py and test_sockets_browser.py.
"""

import multiprocessing
import os
import socket
import socketserver
import sys
import time
from subprocess import Popen

import clang_native
import common
from common import PYTHON
from decorators import test_file

from tools import config
from tools.shared import CLANG_CC, EMCC
from tools.utils import run_process

npm_checked = False

EMTEST_SKIP_PYTHON_DEV_PACKAGES = int(os.getenv('EMTEST_SKIP_PYTHON_DEV_PACKAGES', '0'))
EMTEST_SKIP_NODE_DEV_PACKAGES = int(os.getenv('EMTEST_SKIP_NODE_DEV_PACKAGES', '0'))


def requires_python_dev_packages(func):
  assert callable(func)

  @common.wraps(func)
  def decorated(self, *args, **kwargs):
    if EMTEST_SKIP_PYTHON_DEV_PACKAGES:
      return self.skipTest('python websockify based tests are disabled by EMTEST_SKIP_PYTHON_DEV_PACKAGES=1')
    return func(self, *args, **kwargs)

  return decorated


class EchoHandler(socketserver.BaseRequestHandler):
  def handle(self):
    data = self.request.recv(64)
    if data:
      self.request.sendall(data)


def _probe_ipv6_loopback():
  # Some CI containers have no IPv6 loopback, so bind(::1) fails with
  # EADDRNOTAVAIL. Probe once at startup so the IPv6 tests can skip there.
  if not socket.has_ipv6:
    return False
  try:
    with socket.socket(socket.AF_INET6, socket.SOCK_STREAM) as s:
      s.bind(('::1', 0))
    return True
  except OSError:
    return False


HAS_IPV6_LOOPBACK = _probe_ipv6_loopback()


def verify_tcp_connection(port, retries=10, timeout=1):
  # Poll a listening TCP port until it accepts a connection, so a harness
  # doesn't return before its server is ready and race the client.
  for _ in range(retries):
    try:
      # Use explicit '127.0.0.1' (IPv4) instead of 'localhost' because Emscripten's
      # WebSocket server binds to 0.0.0.0 (IPv4), whereas 'localhost' can resolve to
      # IPv6 ::1 first, causing connect timeouts.
      sock = socket.create_connection(('127.0.0.1', port), timeout=timeout)
      sock.close()
      return True
    except OSError:
      time.sleep(1)
  return False


def clean_process(p):
  if getattr(p, 'exitcode', None) is None and getattr(p, 'returncode', None) is None:
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


class WebsockifyServerHarness:
  def __init__(self, filename, args, listen_port, do_server_check=True):
    self.processes = []
    self.filename = filename
    self.listen_port = listen_port
    self.target_port = listen_port - 1
    self.args = args or []
    self.do_server_check = do_server_check

  def __enter__(self):
    try:
      import websockify  # type: ignore
    except ModuleNotFoundError:
      raise Exception('Unable to import module websockify. Run "python3 -m pip install websockify" or set environment variable EMTEST_SKIP_PYTHON_DEV_PACKAGES=1 to skip this test.') from None

    # compile the server
    # NOTE empty filename support is a hack to support
    # the current test_enet
    if self.filename:
      cmd = [CLANG_CC, test_file(self.filename), '-o', 'server', f'-DSOCKK={self.target_port}', *clang_native.get_clang_native_args(), *self.args]
      print(cmd)
      run_process(cmd, env=clang_native.get_clang_native_env())
      process = Popen([os.path.abspath('server')])
      self.processes.append(process)

    # start the websocket proxy
    print('running websockify on %d, forward to tcp %d' % (self.listen_port, self.target_port), file=sys.stderr)
    # source_is_ipv6=True here signals to websockify that it should prefer ipv6 address when
    # resolving host names.  This matches what the node `ws` module does and means that `localhost`
    # resolves to `::1` on IPv6 systems.
    wsp = websockify.WebSocketProxy(verbose=True, source_is_ipv6=True, listen_host="127.0.0.1", listen_port=self.listen_port, target_host="127.0.0.1", target_port=self.target_port, run_once=True)
    self.websockify = multiprocessing.Process(target=wsp.start_server)
    self.websockify.start()
    self.processes.append(self.websockify)
    # Make sure both the actual server and the websocket proxy are running
    if self.do_server_check and not verify_tcp_connection(self.target_port):
      self.clean_processes()
      raise Exception('[Socket server failed to start up in a timely manner]')
    if not verify_tcp_connection(self.listen_port):
      self.clean_processes()
      raise Exception('[Websockify proxy failed to start up in a timely manner]')

    print('[Websockify on process %s]' % str(self.processes[-2:]))
    return self

  def __exit__(self, *args, **kwargs):
    # try to kill the websockify proxy gracefully
    if self.websockify.is_alive():
      self.websockify.terminate()
    self.websockify.join()

    # clean up any processes we started
    self.clean_processes()

  def clean_processes(self):
    for p in self.processes:
      clean_process(p)


class CompiledServerHarness:
  def __init__(self, filename, args, listen_port, do_server_check=True):
    self.process = None
    self.filename = filename
    self.listen_port = listen_port
    self.args = args or []
    self.do_server_check = do_server_check

  def __enter__(self):
    # assuming this is only used for WebSocket tests at the moment, validate that
    # the ws module is installed
    global npm_checked
    if not npm_checked:
      child = run_process([*config.NODE_JS, '-e', 'require("ws");'], check=False)
      assert child.returncode == 0, '"ws" node module not found. Run "npm install" to obtain Node.js dev dependencies, or set environment variable EMTEST_SKIP_NODE_DEV_PACKAGES=1 to skip this test.'
      npm_checked = True

    # compile the server
    suffix = '.mjs' if '-sEXPORT_ES6' in self.args else '.js'
    proc = run_process([EMCC, '-Werror', test_file(self.filename), '-o', 'server' + suffix, f'-DSOCKK={self.listen_port}', *self.args])
    print('Socket server build: out:', proc.stdout or '', '/ err:', proc.stderr or '')

    self.process = Popen([*config.NODE_JS, 'server' + suffix])

    # Wait for the server to start listening before returning: the node ws
    # server binds its port asynchronously after process startup, so a client
    # that connects too early races the listen() and sees ECONNREFUSED. Skipped
    # for tests whose server intentionally never listens (e.g. server-down).
    if self.do_server_check and not verify_tcp_connection(self.listen_port):
      clean_process(self.process)
      raise Exception('[Compiled server failed to start up in a timely manner]')

    return self

  def __exit__(self, *args, **kwargs):
    clean_process(self.process)


# Executes a native executable server process
class BackgroundServerProcess:
  def __init__(self, args):
    self.process = None
    self.args = args

  def __enter__(self):
    print('Running background server: ' + str(self.args))
    self.process = Popen(self.args)
    return self

  def __exit__(self, *args, **kwargs):
    clean_process(self.process)


def NodeJsWebSocketEchoServerProcess():
  return BackgroundServerProcess([*config.NODE_JS, test_file('websocket/nodejs_websocket_echo_server.js')])


def PythonTcpEchoServerProcess(port):
  return BackgroundServerProcess([PYTHON, test_file('websocket/tcp_echo_server.py'), port])
