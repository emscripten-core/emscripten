import os, multiprocessing, subprocess, socket, time
from runner import BrowserCore, path_from_root
from tools.shared import *

node_ws_module_installed = False
try:
  NPM = os.path.join(os.path.dirname(NODE_JS[0]), 'npm.cmd' if WINDOWS else 'npm')
except:
  pass

def clean_processes(processes):
  import signal, errno
  for p in processes:
    if (not hasattr(p, 'exitcode') or p.exitcode == None) and (not hasattr(p, 'returncode') or p.returncode == None):
      # ask nicely (to try and catch the children)
      try:
        p.terminate() # SIGTERM
      except:
        pass
      time.sleep(1)
      # send a forcible kill immediately afterwards. If the process did not die before, this should clean it.
      try:
        p.kill() # SIGKILL
      except:
        pass

def make_relay_server(port1, port2):
  print >> sys.stderr, 'creating relay server on ports %d,%d' % (port1, port2)
  proc = Popen([PYTHON, path_from_root('tests', 'sockets', 'socket_relay.py'), str(port1), str(port2)])
  return proc

class WebsockifyServerHarness:
  def __init__(self, filename, args, listen_port, do_server_check=True):
    self.processes = []
    self.filename = filename
    self.listen_port = listen_port
    self.target_port = listen_port-1
    self.args = args or []
    self.do_server_check = do_server_check

  def __enter__(self):
    import socket, websockify

    # compile the server
    # NOTE empty filename support is a hack to support
    # the current test_enet
    if self.filename:
      sp = Popen([CLANG_CC, path_from_root('tests', self.filename), '-o', 'server', '-DSOCKK=%d' % self.target_port] + get_clang_native_args() + self.args, env=get_clang_native_env(), stdout=PIPE, stderr=PIPE)
      out = sp.communicate()
      print 'Socket server build: out:', out[0] or '', '/ err:', out[1] or ''
      assert sp.returncode == 0
      process = Popen([os.path.abspath('server')])
      self.processes.append(process)

    # start the websocket proxy
    print >> sys.stderr, 'running websockify on %d, forward to tcp %d' % (self.listen_port, self.target_port)
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
      except:
        time.sleep(1)
    else:
      clean_processes(self.processes)
      raise Exception('[Websockify failed to start up in a timely manner]')

    print '[Websockify on process %s]' % str(self.processes[-2:])

  def __exit__(self, *args, **kwargs):
    # try to kill the websockify proxy gracefully
    if self.websockify.is_alive():
      self.websockify.terminate()
    self.websockify.join()

    # clean up any processes we started
    clean_processes(self.processes)


class CompiledServerHarness:
  def __init__(self, filename, args, listen_port):
    self.processes = []
    self.filename = filename
    self.listen_port = listen_port
    self.args = args or []

  def __enter__(self):
    # assuming this is only used for WebSocket tests at the moment, validate that
    # the ws module is installed
    child = Popen(NODE_JS + ['-e', 'require("ws");'])
    child.communicate()
    global node_ws_module_installed
    # Attempt to automatically install ws module for Node.js.
    if child.returncode != 0 and not node_ws_module_installed:
      node_ws_module_installed = True
      Popen([NPM, 'install', path_from_root('tests', 'sockets', 'ws')], cwd=os.path.dirname(EMCC)).communicate()
      # Did installation succeed?
      child = Popen(NODE_JS + ['-e', 'require("ws");'])
      child.communicate()
    assert child.returncode == 0, 'ws module for Node.js not installed, and automatic installation failed! Please run \'npm install\' from %s' % EMSCRIPTEN_ROOT

    # compile the server
    sp = Popen([PYTHON, EMCC, path_from_root('tests', self.filename), '-o', 'server.js', '-DSOCKK=%d' % self.listen_port] + self.args)
    out = sp.communicate()
    print 'Socket server build: out:', out[0] or '', '/ err:', out[1] or ''
    assert sp.returncode == 0

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
  def setUpClass(self):
    super(sockets, self).setUpClass()
    print
    print 'Running the socket tests. Make sure the browser allows popups from localhost.'
    print

  def test_inet(self):
    src = r'''
      #include <stdio.h>
      #include <arpa/inet.h>

      int main() {
        printf("*%x,%x,%x,%x,%x,%x*\n", htonl(0xa1b2c3d4), htonl(0xfe3572e0), htonl(0x07abcdf0), htons(0xabcd), ntohl(0x43211234), ntohs(0xbeaf));
        in_addr_t i = inet_addr("190.180.10.78");
        printf("%x\n", i);
        return 0;
      }
    '''
    self.do_run(src, '*d4c3b2a1,e07235fe,f0cdab07,cdab,34122143,afbe*\n4e0ab4be\n')

  def test_inet2(self):
    src = r'''
      #include <stdio.h>
      #include <arpa/inet.h>

      int main() {
        struct in_addr x, x2;
        int *y = (int*)&x;
        *y = 0x12345678;
        printf("%s\n", inet_ntoa(x));
        int r = inet_aton(inet_ntoa(x), &x2);
        printf("%s\n", inet_ntoa(x2));
        return 0;
      }
    '''
    self.do_run(src, '120.86.52.18\n120.86.52.18\n')

  def test_inet3(self):
    src = r'''
      #include <stdio.h>
      #include <arpa/inet.h>
      #include <sys/socket.h>
      int main() {
        char dst[64];
        struct in_addr x, x2;
        int *y = (int*)&x;
        *y = 0x12345678;
        printf("%s\n", inet_ntop(AF_INET,&x,dst,sizeof dst));
        int r = inet_aton(inet_ntoa(x), &x2);
        printf("%s\n", inet_ntop(AF_INET,&x2,dst,sizeof dst));
        return 0;
      }
    '''
    self.do_run(src, '120.86.52.18\n120.86.52.18\n')

  def test_inet4(self):
    src = r'''
      #include <stdio.h>
      #include <arpa/inet.h>
      #include <sys/socket.h>

      void test(const char *test_addr, bool first=true){
          char str[40];
          struct in6_addr addr;
          unsigned char *p = (unsigned char*)&addr;
          int ret;
          ret = inet_pton(AF_INET6,test_addr,&addr);
          if(ret == -1) return;
          if(ret == 0) return;
          if(inet_ntop(AF_INET6,&addr,str,sizeof(str)) == NULL ) return;
          printf("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x - %s\n",
               p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],p[12],p[13],p[14],p[15],str);
          if (first) test(str, false); // check again, on our output
      }
      int main(){
          test("::");
          test("::1");
          test("::1.2.3.4");
          test("::17.18.19.20");
          test("::ffff:1.2.3.4");
          test("1::ffff");
          test("::255.255.255.255");
          test("0:ff00:1::");
          test("0:ff::");
          test("abcd::");
          test("ffff::a");
          test("ffff::a:b");
          test("ffff::a:b:c");
          test("ffff::a:b:c:d");
          test("ffff::a:b:c:d:e");
          test("::1:2:0:0:0");
          test("0:0:1:2:3::");
          test("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
          test("1::255.255.255.255");

          //below should fail and not produce results..
          test("1.2.3.4");
          test("");
          test("-");

          printf("ok.\n");
      }
    '''
    self.do_run(src, r'''0000:0000:0000:0000:0000:0000:0000:0000 - ::
0000:0000:0000:0000:0000:0000:0000:0000 - ::
0000:0000:0000:0000:0000:0000:0000:0001 - ::1
0000:0000:0000:0000:0000:0000:0000:0001 - ::1
0000:0000:0000:0000:0000:0000:0102:0304 - ::102:304
0000:0000:0000:0000:0000:0000:0102:0304 - ::102:304
0000:0000:0000:0000:0000:0000:1112:1314 - ::1112:1314
0000:0000:0000:0000:0000:0000:1112:1314 - ::1112:1314
0000:0000:0000:0000:0000:ffff:0102:0304 - ::ffff:1.2.3.4
0000:0000:0000:0000:0000:ffff:0102:0304 - ::ffff:1.2.3.4
0001:0000:0000:0000:0000:0000:0000:ffff - 1::ffff
0001:0000:0000:0000:0000:0000:0000:ffff - 1::ffff
0000:0000:0000:0000:0000:0000:ffff:ffff - ::ffff:ffff
0000:0000:0000:0000:0000:0000:ffff:ffff - ::ffff:ffff
0000:ff00:0001:0000:0000:0000:0000:0000 - 0:ff00:1::
0000:ff00:0001:0000:0000:0000:0000:0000 - 0:ff00:1::
0000:00ff:0000:0000:0000:0000:0000:0000 - 0:ff::
0000:00ff:0000:0000:0000:0000:0000:0000 - 0:ff::
abcd:0000:0000:0000:0000:0000:0000:0000 - abcd::
abcd:0000:0000:0000:0000:0000:0000:0000 - abcd::
ffff:0000:0000:0000:0000:0000:0000:000a - ffff::a
ffff:0000:0000:0000:0000:0000:0000:000a - ffff::a
ffff:0000:0000:0000:0000:0000:000a:000b - ffff::a:b
ffff:0000:0000:0000:0000:0000:000a:000b - ffff::a:b
ffff:0000:0000:0000:0000:000a:000b:000c - ffff::a:b:c
ffff:0000:0000:0000:0000:000a:000b:000c - ffff::a:b:c
ffff:0000:0000:0000:000a:000b:000c:000d - ffff::a:b:c:d
ffff:0000:0000:0000:000a:000b:000c:000d - ffff::a:b:c:d
ffff:0000:0000:000a:000b:000c:000d:000e - ffff::a:b:c:d:e
ffff:0000:0000:000a:000b:000c:000d:000e - ffff::a:b:c:d:e
0000:0000:0000:0001:0002:0000:0000:0000 - ::1:2:0:0:0
0000:0000:0000:0001:0002:0000:0000:0000 - ::1:2:0:0:0
0000:0000:0001:0002:0003:0000:0000:0000 - 0:0:1:2:3::
0000:0000:0001:0002:0003:0000:0000:0000 - 0:0:1:2:3::
ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff - ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff - ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
0001:0000:0000:0000:0000:0000:ffff:ffff - 1::ffff:ffff
0001:0000:0000:0000:0000:0000:ffff:ffff - 1::ffff:ffff
ok.
''')

  def test_getsockname_unconnected_socket(self):
    self.do_run(r'''
      #include <sys/socket.h>
      #include <stdio.h>
      #include <assert.h>
      #include <sys/socket.h>
      #include <netinet/in.h>
      #include <arpa/inet.h>
      #include <string.h>
      int main() {
        int fd;
        int z;
        fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct sockaddr_in adr_inet;
        socklen_t len_inet = sizeof adr_inet;
        z = getsockname(fd, (struct sockaddr *)&adr_inet, &len_inet);
        if (z != 0) {
          perror("getsockname error");
          return 1;
        }
        char buffer[1000];
        sprintf(buffer, "%s:%u", inet_ntoa(adr_inet.sin_addr), (unsigned)ntohs(adr_inet.sin_port));
        const char *correct = "0.0.0.0:0";
        printf("got (expected) socket: %s (%s), size %d (%d)\n", buffer, correct, strlen(buffer), strlen(correct));
        assert(strlen(buffer) == strlen(correct));
        assert(strcmp(buffer, correct) == 0);
        puts("success.");
      }
    ''', 'success.')

  def test_getpeername_unconnected_socket(self):
    self.do_run(r'''
      #include <sys/socket.h>
      #include <stdio.h>
      #include <assert.h>
      #include <sys/socket.h>
      #include <netinet/in.h>
      #include <arpa/inet.h>
      #include <string.h>
      int main() {
        int fd;
        int z;
        fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct sockaddr_in adr_inet;
        socklen_t len_inet = sizeof adr_inet;
        z = getpeername(fd, (struct sockaddr *)&adr_inet, &len_inet);
        if (z != 0) {
          perror("getpeername error");
          return 1;
        }
        puts("unexpected success.");
      }
    ''', 'getpeername error: Socket not connected')

  def test_getaddrinfo(self):
    self.emcc_args=[]
    self.do_run(open(path_from_root('tests', 'sockets', 'test_getaddrinfo.c')).read(), 'success')

  def test_getnameinfo(self):
    self.do_run(open(path_from_root('tests', 'sockets', 'test_getnameinfo.c')).read(), 'success')

  def test_gethostbyname(self):
    self.do_run(open(path_from_root('tests', 'sockets', 'test_gethostbyname.c')).read(), 'success')

  def test_getprotobyname(self):
    self.do_run(open(path_from_root('tests', 'sockets', 'test_getprotobyname.c')).read(), 'success')

  def test_sockets_echo(self):
    sockets_include = '-I'+path_from_root('tests', 'sockets')

    # Websockify-proxied servers can't run dgram tests
    harnesses = [
      (WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include], 49160), 0),
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0'], 49161), 0),
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=1'], 49162), 1),
      # The following forces non-NULL addr and addlen parameters for the accept call
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0', '-DTEST_ACCEPT_ADDR=1'], 49163), 0)
    ]

    for harness, datagram in harnesses:
      with harness:
        self.btest(os.path.join('sockets', 'test_sockets_echo_client.c'), expected='0', args=['-DSOCKK=%d' % harness.listen_port, '-DTEST_DGRAM=%d' % datagram, sockets_include])

  def test_sdl2_sockets_echo(self):
    harness = CompiledServerHarness('sdl2_net_server.c', ['-s', 'USE_SDL=2', '-s', 'USE_SDL_NET=2'], 49164)
    with harness:
      self.btest('sdl2_net_client.c', expected='0', args=['-s', 'USE_SDL=2', '-s', 'USE_SDL_NET=2', '-DSOCKK=%d' % harness.listen_port])

  def test_sockets_async_echo(self):
    if WINDOWS: return self.skip('This test is Unix-specific.')
    # Run with ./runner.py sockets.test_sockets_async_echo
    sockets_include = '-I'+path_from_root('tests', 'sockets')

    # Websockify-proxied servers can't run dgram tests
    harnesses = [
      (WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_ASYNC=1'], 49165), 0),
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0', '-DTEST_ASYNC=1'], 49166), 0),
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=1', '-DTEST_ASYNC=1'], 49167), 1),
      # The following forces non-NULL addr and addlen parameters for the accept call
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0', '-DTEST_ACCEPT_ADDR=1', '-DTEST_ASYNC=1'], 49168), 0)
    ]

    for harness, datagram in harnesses:
      print 'harness:', harness
      with harness:
        self.btest(os.path.join('sockets', 'test_sockets_echo_client.c'), expected='0', args=['-DSOCKK=%d' % harness.listen_port, '-DTEST_DGRAM=%d' % datagram, '-DTEST_ASYNC=1', sockets_include])

    # Deliberately attempt a connection on a port that will fail to test the error callback and getsockopt
    print 'expect fail'
    self.btest(os.path.join('sockets', 'test_sockets_echo_client.c'), expected='0', args=['-DSOCKK=49169', '-DTEST_ASYNC=1', sockets_include])

  def test_sockets_echo_bigdata(self):
    sockets_include = '-I'+path_from_root('tests', 'sockets')

    # generate a large string literal to use as our message
    message = ''
    for i in range(256*256*2):
        message += str(unichr(ord('a') + (i % 26)))

    # re-write the client test with this literal (it's too big to pass via command line)
    input_filename = path_from_root('tests', 'sockets', 'test_sockets_echo_client.c')
    input = open(input_filename).read()
    output = input.replace('#define MESSAGE "pingtothepong"', '#define MESSAGE "%s"' % message)

    harnesses = [
      (WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include], 49170), 0),
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0'], 49171), 0),
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=1'], 49172), 1)
    ]

    for harness, datagram in harnesses:
      with harness:
        self.btest(output, expected='0', args=[sockets_include, '-DSOCKK=%d' % harness.listen_port, '-DTEST_DGRAM=%d' % datagram], force_c=True)

  def test_sockets_partial(self):
    if WINDOWS: return self.skip('This test is Unix-specific.')
    for harness in [
      WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_partial_server.c'), [], 49180),
      CompiledServerHarness(os.path.join('sockets', 'test_sockets_partial_server.c'), [], 49181)
    ]:
      with harness:
        self.btest(os.path.join('sockets', 'test_sockets_partial_client.c'), expected='165', args=['-DSOCKK=%d' % harness.listen_port])

  def test_sockets_select_server_down(self):
    if WINDOWS: return self.skip('This test is Unix-specific.')
    for harness in [
      WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_select_server_down_server.c'), [], 49190, do_server_check=False),
      CompiledServerHarness(os.path.join('sockets', 'test_sockets_select_server_down_server.c'), [], 49191)
    ]:
      with harness:
        self.btest(os.path.join('sockets', 'test_sockets_select_server_down_client.c'), expected='266', args=['-DSOCKK=%d' % harness.listen_port])

  def test_sockets_select_server_closes_connection_rw(self):
    if WINDOWS: return self.skip('This test is Unix-specific.')
    sockets_include = '-I'+path_from_root('tests', 'sockets')

    for harness in [
      WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DCLOSE_CLIENT_AFTER_ECHO'], 49200),
      CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DCLOSE_CLIENT_AFTER_ECHO'], 49201)
    ]:
      with harness:
        self.btest(os.path.join('sockets', 'test_sockets_select_server_closes_connection_client_rw.c'), expected='266', args=[sockets_include, '-DSOCKK=%d' % harness.listen_port])

  def test_enet(self):
    if WINDOWS: return self.skip('This test uses Unix-specific build architecture.')
    # this is also a good test of raw usage of emconfigure and emmake
    try_delete(self.in_dir('enet'))
    shutil.copytree(path_from_root('tests', 'enet'), self.in_dir('enet'))
    pwd = os.getcwd()
    os.chdir(self.in_dir('enet'))
    Popen([PYTHON, path_from_root('emconfigure'), './configure']).communicate()
    Popen([PYTHON, path_from_root('emmake'), 'make']).communicate()
    enet = [self.in_dir('enet', '.libs', 'libenet.a'), '-I'+path_from_root('tests', 'enet', 'include')]
    os.chdir(pwd)

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
  #   try_delete(self.in_dir('enet'))
  #   shutil.copytree(path_from_root('tests', 'enet'), self.in_dir('enet'))
  #   pwd = os.getcwd()
  #   os.chdir(self.in_dir('enet'))
  #   Popen([PYTHON, path_from_root('emconfigure'), './configure']).communicate()
  #   Popen([PYTHON, path_from_root('emmake'), 'make']).communicate()
  #   enet = [self.in_dir('enet', '.libs', 'libenet.a'), '-I'+path_from_root('tests', 'enet', 'include')]
  #   os.chdir(pwd)
  #   Popen([PYTHON, EMCC, path_from_root('tests', 'sockets', 'test_enet_server.c'), '-o', 'server.html', '-DSOCKK=2235'] + enet).communicate()

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
    return self.skip('WebRTC support is not up to date.')
    host_src = 'webrtc_host.c'
    peer_src = 'webrtc_peer.c'

    host_outfile = 'host.html'
    peer_outfile = 'peer.html'

    host_filepath = path_from_root('tests', 'sockets', host_src)
    temp_host_filepath = os.path.join(self.get_dir(), os.path.basename(host_src))
    with open(host_filepath) as f: host_src = f.read()
    with open(temp_host_filepath, 'w') as f: f.write(self.with_report_result(host_src))

    peer_filepath = path_from_root('tests', 'sockets', peer_src)
    temp_peer_filepath = os.path.join(self.get_dir(), os.path.basename(peer_src))
    with open(peer_filepath) as f: peer_src = f.read()
    with open(temp_peer_filepath, 'w') as f: f.write(self.with_report_result(peer_src))

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

    Popen([PYTHON, EMCC, temp_host_filepath, '-o', host_outfile] + ['-s', 'GL_TESTING=1', '--pre-js', 'host_pre.js', '-s', 'SOCKET_WEBRTC=1', '-s', 'SOCKET_DEBUG=1']).communicate()
    Popen([PYTHON, EMCC, temp_peer_filepath, '-o', peer_outfile] + ['-s', 'GL_TESTING=1', '--pre-js', 'peer_pre.js', '-s', 'SOCKET_WEBRTC=1', '-s', 'SOCKET_DEBUG=1']).communicate()

    # note: you may need to run this manually yourself, if npm is not in the path, or if you need a version that is not in the path
    Popen([NPM, 'install', path_from_root('tests', 'sockets', 'p2p')]).communicate()
    broker = Popen(NODE_JS + [path_from_root('tests', 'sockets', 'p2p', 'broker', 'p2p-broker.js')])

    expected = '1'
    self.run_browser(host_outfile, '.', ['/report_result?' + e for e in expected])

    broker.kill();

  def test_nodejs_sockets_echo(self):
    # This test checks that sockets work when the client code is run in Node.js
    # Run with ./runner.py sockets.test_nodejs_sockets_echo
    if not NODE_JS in JS_ENGINES:
        return self.skip('node is not present')

    sockets_include = '-I'+path_from_root('tests', 'sockets')

    harnesses = [
      (WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include], 59160), 0),
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=0'], 59162), 0),
      (CompiledServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include, '-DTEST_DGRAM=1'], 59164), 1)
    ]

    # Basic test of node client against both a Websockified and compiled echo server.
    for harness, datagram in harnesses:
      with harness:
        Popen([PYTHON, EMCC, path_from_root('tests', 'sockets', 'test_sockets_echo_client.c'), '-o', 'client.js', '-DSOCKK=%d' % harness.listen_port, '-DTEST_DGRAM=%d' % datagram, '-DREPORT_RESULT=int dummy'], stdout=PIPE, stderr=PIPE).communicate()

        out = run_js('client.js', engine=NODE_JS, full_output=True)
        self.assertContained('do_msg_read: read 14 bytes', out)

    # Test against a Websockified server with compile time configured WebSocket subprotocol. We use a Websockified
    # server because as long as the subprotocol list contains binary it will configure itself to accept binary
    # This test also checks that the connect url contains the correct subprotocols.
    print "\nTesting compile time WebSocket configuration.\n"
    for harness in [
      WebsockifyServerHarness(os.path.join('sockets', 'test_sockets_echo_server.c'), [sockets_include], 59166)
    ]:
      with harness:
        Popen([PYTHON, EMCC, path_from_root('tests', 'sockets', 'test_sockets_echo_client.c'), '-o', 'client.js', '-s', 'SOCKET_DEBUG=1', '-s', 'WEBSOCKET_SUBPROTOCOL="base64, binary"', '-DSOCKK=59166', '-DREPORT_RESULT=int dummy'], stdout=PIPE, stderr=PIPE).communicate()

        out = run_js('client.js', engine=NODE_JS, full_output=True)
        self.assertContained('do_msg_read: read 14 bytes', out)
        self.assertContained(['connect: ws://127.0.0.1:59166, base64,binary', 'connect: ws://127.0.0.1:59166/, base64,binary'], out)

    # Test against a Websockified server with runtime WebSocket configuration. We specify both url and subprotocol.
    # In this test we have *deliberately* used the wrong port '-DSOCKK=12345' to configure the echo_client.c, so
    # the connection would fail without us specifying a valid WebSocket URL in the configuration.
    print "\nTesting runtime WebSocket configuration.\n"
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

        Popen([PYTHON, EMCC, path_from_root('tests', 'sockets', 'test_sockets_echo_client.c'), '-o', 'client.js', '--pre-js', 'websocket_pre.js', '-s', 'SOCKET_DEBUG=1', '-DSOCKK=12345', '-DREPORT_RESULT=int dummy'], stdout=PIPE, stderr=PIPE).communicate()

        out = run_js('client.js', engine=NODE_JS, full_output=True)
        self.assertContained('do_msg_read: read 14 bytes', out)
        self.assertContained('connect: ws://localhost:59168/testA/testB, text,base64,binary', out)

