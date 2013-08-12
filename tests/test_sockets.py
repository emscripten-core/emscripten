import os, multiprocessing, subprocess
from runner import BrowserCore, path_from_root
from tools.shared import *

def clean_pids(pids):
  import signal, errno
  def pid_exists(pid):
    try:
      # NOTE: may just kill the process in Windows
      os.kill(pid, 0)
    except OSError, e:
      return e.errno == errno.EPERM
    else:
        return True
  def kill_pids(pids, sig):
    for pid in pids:
      if not pid_exists(pid):
        break
      print '[killing %d]' % pid
      try:
        os.kill(pid, sig)
        print '[kill succeeded]'
      except:
        print '[kill fail]'
  # ask nicely (to try and catch the children)
  kill_pids(pids, signal.SIGTERM)
  time.sleep(1)
  # extreme prejudice, may leave children
  kill_pids(pids, signal.SIGKILL)

def make_relay_server(port1, port2):
  print >> sys.stderr, 'creating relay server on ports %d,%d' % (port1, port2)
  proc = Popen([PYTHON, path_from_root('tests', 'sockets', 'socket_relay.py'), str(port1), str(port2)])
  return proc

class WebsockifyServerHarness:
  def __init__(self, filename, args, listen_port, target_port):
    self.pids = []
    self.filename = filename
    self.target_port = target_port
    self.listen_port = listen_port
    self.args = args or []

  def __enter__(self):
    import socket, websockify

    # compile the server
    # NOTE empty filename support is a hack to support
    # the current test_enet
    if self.filename:
      Popen([CLANG_CC, path_from_root('tests', self.filename), '-o', 'server'] + self.args).communicate()
      process = Popen([os.path.abspath('server')])
      self.pids.append(process.pid)

    # start the websocket proxy
    print >> sys.stderr, 'running websockify on %d, forward to tcp %d' % (self.listen_port, self.target_port)
    wsp = websockify.WebSocketProxy(verbose=True, listen_port=self.listen_port, target_host="127.0.0.1", target_port=self.target_port, run_once=True)
    self.websockify = multiprocessing.Process(target=wsp.start_server)
    self.websockify.start()
    self.pids.append(self.websockify.pid)
    print '[Websockify on process %s]' % str(self.pids[-2:])

  def __exit__(self, *args, **kwargs):
    # try to kill the websockify proxy gracefully
    if self.websockify.is_alive():
      self.websockify.terminate()
    self.websockify.join()

    # clean up any processes we started
    clean_pids(self.pids)


class CompiledServerHarness:
  def __init__(self, filename, args):
    self.pids = []
    self.filename = filename
    self.args = args or []

  def __enter__(self):
    import socket, websockify

    # compile the server
    Popen([PYTHON, EMCC, path_from_root('tests', self.filename), '-o', 'server.js'] + self.args).communicate()
    process = Popen([NODE_JS, 'server.js'])
    self.pids.append(process.pid)

  def __exit__(self, *args, **kwargs):
    # clean up any processes we started
    clean_pids(self.pids)

    # always run these tests last
    # make sure to use different ports in each one because it takes a while for the processes to be cleaned up

    # NOTE all datagram tests are temporarily disabled, as
    # we can't truly test datagram sockets until we have
    # proper listen server support.

class sockets(BrowserCore):
  def test_sockets_echo(self):
    sockets_include = '-I'+path_from_root('tests', 'sockets')

    for datagram in [0]:
      dgram_define = '-DTEST_DGRAM=%d' % datagram

      for harness in [
        WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), ['-DSOCKK=8990', dgram_define, sockets_include], 8991, 8990)
        # CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), ['-DSOCKK=8990', dgram_define, sockets_include])
      ]:
        with harness:
          self.btest(os.path.join('sockets', 'test_sockets_echo_client.c'), expected='0', args=['-DSOCKK=8991', dgram_define, sockets_include])

  def test_sockets_echo_bigdata(self):
    sockets_include = '-I'+path_from_root('tests', 'sockets')

    for datagram in [0]:
      dgram_define = '-DTEST_DGRAM=%d' % datagram

      # generate a large string literal to use as our message
      message = ''
      for i in range(256*256*2):
          message += str(unichr(ord('a') + (i % 26)))

      # re-write the client test with this literal (it's too big to pass via command line)
      input_filename = path_from_root('tests', 'sockets', 'test_sockets_echo_client.c')
      input = open(input_filename).read()
      output = input.replace('#define MESSAGE "pingtothepong"', '#define MESSAGE "%s"' % message)

      for harness in [
        WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), ['-DSOCKK=8992', dgram_define, sockets_include], 8993, 8992)
      ]:
        with harness:
          self.btest(output, expected='0', args=['-DSOCKK=8993', dgram_define, sockets_include], force_c=True)

  def test_sockets_partial(self):
    for harness in [
      WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_partial_server.c'), ['-DSOCKK=8994'], 8995, 8994)
    ]:
      with harness:
        self.btest(os.path.join('sockets', 'test_sockets_partial_client.c'), expected='165', args=['-DSOCKK=8995'])

  # TODO add support for gethostbyaddr to re-enable this test
  # def test_sockets_gethostbyname(self):
  #   self.btest(os.path.join('sockets', 'test_sockets_gethostbyname.c'), expected='0', args=['-O2', '-DSOCKK=8997'])

  def test_sockets_select_server_no_accept(self):
    for harness in [
      WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_select_server_no_accept_server.c'), ['-DSOCKK=8995'], 8996, 8995)
    ]:
      self.btest(os.path.join('sockets', 'test_sockets_select_server_no_accept_client.c'), expected='266', args=['-DSOCKK=8996'])

  def test_sockets_select_server_closes_connection_rw(self):
    sockets_include = '-I'+path_from_root('tests', 'sockets')

    for harness in [
      WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), ['-DSOCKK=9004', sockets_include], 9005, 9004)
    ]:
      with harness:
        self.btest(os.path.join('sockets', 'test_sockets_select_server_closes_connection_client_rw.c'), expected='266', args=['-DSOCKK=9005', sockets_include])

  # TODO remove this once we have proper listen server support built into emscripten.
  # being that enet uses datagram sockets, we can't proxy to a native server with
  # websockify, so we're emulating the listen server in the browser and relaying
  # between two TCP servers.
  def test_enet(self):
    try_delete(self.in_dir('enet'))
    shutil.copytree(path_from_root('tests', 'enet'), self.in_dir('enet'))
    pwd = os.getcwd()
    os.chdir(self.in_dir('enet'))
    Popen([PYTHON, path_from_root('emconfigure'), './configure']).communicate()
    Popen([PYTHON, path_from_root('emmake'), 'make']).communicate()
    enet = [self.in_dir('enet', '.libs', 'libenet.a'), '-I'+path_from_root('tests', 'enet', 'include')]
    os.chdir(pwd)
    Popen([PYTHON, EMCC, path_from_root('tests', 'sockets', 'test_enet_server.c'), '-o', 'server.html', '-DSOCKK=2235'] + enet).communicate()

    with WebsockifyServerHarness('', [], 2235, 2234):
      with WebsockifyServerHarness('', [], 2237, 2236):
        pids = []
        try:
          proc = make_relay_server(2234, 2236)
          pids.append(proc.pid)
          self.btest(os.path.join('sockets', 'test_enet_client.c'), expected='0', args=['-DSOCKK=2237'] + enet)
        finally:
          clean_pids(pids);

  # TODO use this once we have listen server support
  # def test_enet(self):
  #   try_delete(self.in_dir('enet'))
  #   shutil.copytree(path_from_root('tests', 'enet'), self.in_dir('enet'))
  #   pwd = os.getcwd()
  #   os.chdir(self.in_dir('enet'))
  #   Popen([PYTHON, path_from_root('emconfigure'), './configure']).communicate()
  #   Popen([PYTHON, path_from_root('emmake'), 'make']).communicate()
  #   enet = [self.in_dir('enet', '.libs', 'libenet.a'), '-I'+path_from_root('tests', 'enet', 'include')]
  #   os.chdir(pwd)

  #   for harness in [
  #     self.CompiledServerHarness(os.path.join('sockets', 'test_enet_server.c'), ['-DSOCKK=9010'] + enet, 9011, 9010)
  #   ]:
  #     with harness:
  #       self.btest(os.path.join('sockets', 'test_enet_client.c'), expected='0', args=['-DSOCKK=9011'] + enet)
