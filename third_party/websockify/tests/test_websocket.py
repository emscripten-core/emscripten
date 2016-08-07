# vim: tabstop=4 shiftwidth=4 softtabstop=4

# Copyright(c)2013 NTT corp. All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.

""" Unit tests for websocket """
import errno
import os
import logging
import select
import shutil
import socket
import ssl
import stubout
import sys
import tempfile
import unittest
import socket
import signal
from websockify import websocket

try:
    from SimpleHTTPServer import SimpleHTTPRequestHandler
except ImportError:
    from http.server import SimpleHTTPRequestHandler

try:
    from StringIO import StringIO
    BytesIO = StringIO
except ImportError:
    from io import StringIO
    from io import BytesIO




def raise_oserror(*args, **kwargs):
    raise OSError('fake error')


class FakeSocket(object):
    def __init__(self, data=''):
        if isinstance(data, bytes):
            self._data = data
        else:
            self._data = data.encode('latin_1')

    def recv(self, amt, flags=None):
        res = self._data[0:amt]
        if not (flags & socket.MSG_PEEK):
            self._data = self._data[amt:]

        return res

    def makefile(self, mode='r', buffsize=None):
        if 'b' in mode:
            return BytesIO(self._data)
        else:
            return StringIO(self._data.decode('latin_1'))


class WebSocketRequestHandlerTestCase(unittest.TestCase):
    def setUp(self):
        super(WebSocketRequestHandlerTestCase, self).setUp()
        self.stubs = stubout.StubOutForTesting()
        self.tmpdir = tempfile.mkdtemp('-websockify-tests')
        # Mock this out cause it screws tests up
        self.stubs.Set(os, 'chdir', lambda *args, **kwargs: None)
        self.stubs.Set(SimpleHTTPRequestHandler, 'send_response',
                       lambda *args, **kwargs: None)

    def tearDown(self):
        """Called automatically after each test."""
        self.stubs.UnsetAll()
        os.rmdir(self.tmpdir)
        super(WebSocketRequestHandlerTestCase, self).tearDown()

    def _get_server(self, handler_class=websocket.WebSocketRequestHandler,
                    **kwargs):
        web = kwargs.pop('web', self.tmpdir)
        return websocket.WebSocketServer(
            handler_class, listen_host='localhost',
            listen_port=80, key=self.tmpdir, web=web,
            record=self.tmpdir, daemon=False, ssl_only=0, idle_timeout=1,
            **kwargs)

    def test_normal_get_with_only_upgrade_returns_error(self):
        server = self._get_server(web=None)
        handler = websocket.WebSocketRequestHandler(
            FakeSocket('GET /tmp.txt HTTP/1.1'), '127.0.0.1', server)

        def fake_send_response(self, code, message=None):
            self.last_code = code

        self.stubs.Set(SimpleHTTPRequestHandler, 'send_response',
                       fake_send_response)

        handler.do_GET()
        self.assertEqual(handler.last_code, 405)

    def test_list_dir_with_file_only_returns_error(self):
        server = self._get_server(file_only=True)
        handler = websocket.WebSocketRequestHandler(
            FakeSocket('GET / HTTP/1.1'), '127.0.0.1', server)

        def fake_send_response(self, code, message=None):
            self.last_code = code

        self.stubs.Set(SimpleHTTPRequestHandler, 'send_response',
                       fake_send_response)

        handler.path = '/'
        handler.do_GET()
        self.assertEqual(handler.last_code, 404)


class WebSocketServerTestCase(unittest.TestCase):
    def setUp(self):
        super(WebSocketServerTestCase, self).setUp()
        self.stubs = stubout.StubOutForTesting()
        self.tmpdir = tempfile.mkdtemp('-websockify-tests')
        # Mock this out cause it screws tests up
        self.stubs.Set(os, 'chdir', lambda *args, **kwargs: None)

    def tearDown(self):
        """Called automatically after each test."""
        self.stubs.UnsetAll()
        os.rmdir(self.tmpdir)
        super(WebSocketServerTestCase, self).tearDown()

    def _get_server(self, handler_class=websocket.WebSocketRequestHandler,
                    **kwargs):
        return websocket.WebSocketServer(
            handler_class, listen_host='localhost',
            listen_port=80, key=self.tmpdir, web=self.tmpdir,
            record=self.tmpdir, **kwargs)

    def test_daemonize_raises_error_while_closing_fds(self):
        server = self._get_server(daemon=True, ssl_only=1, idle_timeout=1)
        self.stubs.Set(os, 'fork', lambda *args: 0)
        self.stubs.Set(signal, 'signal', lambda *args: None)
        self.stubs.Set(os, 'setsid', lambda *args: None)
        self.stubs.Set(os, 'close', raise_oserror)
        self.assertRaises(OSError, server.daemonize, keepfd=None, chdir='./')

    def test_daemonize_ignores_ebadf_error_while_closing_fds(self):
        def raise_oserror_ebadf(fd):
            raise OSError(errno.EBADF, 'fake error')

        server = self._get_server(daemon=True, ssl_only=1, idle_timeout=1)
        self.stubs.Set(os, 'fork', lambda *args: 0)
        self.stubs.Set(os, 'setsid', lambda *args: None)
        self.stubs.Set(signal, 'signal', lambda *args: None)
        self.stubs.Set(os, 'close', raise_oserror_ebadf)
        self.stubs.Set(os, 'open', raise_oserror)
        self.assertRaises(OSError, server.daemonize, keepfd=None, chdir='./')

    def test_handshake_fails_on_not_ready(self):
        server = self._get_server(daemon=True, ssl_only=0, idle_timeout=1)

        def fake_select(rlist, wlist, xlist, timeout=None):
            return ([], [], [])

        self.stubs.Set(select, 'select', fake_select)
        self.assertRaises(
            websocket.WebSocketServer.EClose, server.do_handshake,
            FakeSocket(), '127.0.0.1')

    def test_empty_handshake_fails(self):
        server = self._get_server(daemon=True, ssl_only=0, idle_timeout=1)

        sock = FakeSocket('')

        def fake_select(rlist, wlist, xlist, timeout=None):
            return ([sock], [], [])

        self.stubs.Set(select, 'select', fake_select)
        self.assertRaises(
            websocket.WebSocketServer.EClose, server.do_handshake,
            sock, '127.0.0.1')

    def test_handshake_policy_request(self):
        # TODO(directxman12): implement
        pass

    def test_handshake_ssl_only_without_ssl_raises_error(self):
        server = self._get_server(daemon=True, ssl_only=1, idle_timeout=1)

        sock = FakeSocket('some initial data')

        def fake_select(rlist, wlist, xlist, timeout=None):
            return ([sock], [], [])

        self.stubs.Set(select, 'select', fake_select)
        self.assertRaises(
            websocket.WebSocketServer.EClose, server.do_handshake,
            sock, '127.0.0.1')

    def test_do_handshake_no_ssl(self):
        class FakeHandler(object):
            CALLED = False
            def __init__(self, *args, **kwargs):
                type(self).CALLED = True

        FakeHandler.CALLED = False

        server = self._get_server(
            handler_class=FakeHandler, daemon=True,
            ssl_only=0, idle_timeout=1)

        sock = FakeSocket('some initial data')

        def fake_select(rlist, wlist, xlist, timeout=None):
            return ([sock], [], [])

        self.stubs.Set(select, 'select', fake_select)
        self.assertEqual(server.do_handshake(sock, '127.0.0.1'), sock)
        self.assertTrue(FakeHandler.CALLED, True)

    def test_do_handshake_ssl(self):
        # TODO(directxman12): implement this
        pass

    def test_do_handshake_ssl_without_ssl_raises_error(self):
        # TODO(directxman12): implement this
        pass

    def test_do_handshake_ssl_without_cert_raises_error(self):
        server = self._get_server(daemon=True, ssl_only=0, idle_timeout=1,
                                  cert='afdsfasdafdsafdsafdsafdas')

        sock = FakeSocket("\x16some ssl data")

        def fake_select(rlist, wlist, xlist, timeout=None):
            return ([sock], [], [])

        self.stubs.Set(select, 'select', fake_select)
        self.assertRaises(
            websocket.WebSocketServer.EClose, server.do_handshake,
            sock, '127.0.0.1')

    def test_do_handshake_ssl_error_eof_raises_close_error(self):
        server = self._get_server(daemon=True, ssl_only=0, idle_timeout=1)

        sock = FakeSocket("\x16some ssl data")

        def fake_select(rlist, wlist, xlist, timeout=None):
            return ([sock], [], [])

        def fake_wrap_socket(*args, **kwargs):
            raise ssl.SSLError(ssl.SSL_ERROR_EOF)

        self.stubs.Set(select, 'select', fake_select)
        self.stubs.Set(ssl, 'wrap_socket', fake_wrap_socket)
        self.assertRaises(
            websocket.WebSocketServer.EClose, server.do_handshake,
            sock, '127.0.0.1')

    def test_fallback_sigchld_handler(self):
        # TODO(directxman12): implement this
        pass

    def test_start_server_error(self):
        server = self._get_server(daemon=False, ssl_only=1, idle_timeout=1)
        sock = server.socket('localhost')

        def fake_select(rlist, wlist, xlist, timeout=None):
            raise Exception("fake error")

        self.stubs.Set(websocket.WebSocketServer, 'socket',
                       lambda *args, **kwargs: sock)
        self.stubs.Set(websocket.WebSocketServer, 'daemonize',
                       lambda *args, **kwargs: None)
        self.stubs.Set(select, 'select', fake_select)
        server.start_server()

    def test_start_server_keyboardinterrupt(self):
        server = self._get_server(daemon=False, ssl_only=0, idle_timeout=1)
        sock = server.socket('localhost')

        def fake_select(rlist, wlist, xlist, timeout=None):
            raise KeyboardInterrupt

        self.stubs.Set(websocket.WebSocketServer, 'socket',
                       lambda *args, **kwargs: sock)
        self.stubs.Set(websocket.WebSocketServer, 'daemonize',
                       lambda *args, **kwargs: None)
        self.stubs.Set(select, 'select', fake_select)
        server.start_server()

    def test_start_server_systemexit(self):
        server = self._get_server(daemon=False, ssl_only=0, idle_timeout=1)
        sock = server.socket('localhost')

        def fake_select(rlist, wlist, xlist, timeout=None):
            sys.exit()

        self.stubs.Set(websocket.WebSocketServer, 'socket',
                       lambda *args, **kwargs: sock)
        self.stubs.Set(websocket.WebSocketServer, 'daemonize',
                       lambda *args, **kwargs: None)
        self.stubs.Set(select, 'select', fake_select)
        server.start_server()

    def test_socket_set_keepalive_options(self):
        keepcnt = 12
        keepidle = 34
        keepintvl = 56

        server = self._get_server(daemon=False, ssl_only=0, idle_timeout=1)
        sock = server.socket('localhost',
                             tcp_keepcnt=keepcnt,
                             tcp_keepidle=keepidle,
                             tcp_keepintvl=keepintvl)

        self.assertEqual(sock.getsockopt(socket.SOL_TCP,
                                         socket.TCP_KEEPCNT), keepcnt)
        self.assertEqual(sock.getsockopt(socket.SOL_TCP,
                                         socket.TCP_KEEPIDLE), keepidle)
        self.assertEqual(sock.getsockopt(socket.SOL_TCP,
                                         socket.TCP_KEEPINTVL), keepintvl)

        sock = server.socket('localhost',
                             tcp_keepalive=False,
                             tcp_keepcnt=keepcnt,
                             tcp_keepidle=keepidle,
                             tcp_keepintvl=keepintvl)

        self.assertNotEqual(sock.getsockopt(socket.SOL_TCP,
                                            socket.TCP_KEEPCNT), keepcnt)
        self.assertNotEqual(sock.getsockopt(socket.SOL_TCP,
                                            socket.TCP_KEEPIDLE), keepidle)
        self.assertNotEqual(sock.getsockopt(socket.SOL_TCP,
                                            socket.TCP_KEEPINTVL), keepintvl)


class HyBiEncodeDecodeTestCase(unittest.TestCase):
    def test_decode_hybi_text(self):
        buf = b'\x81\x85\x37\xfa\x21\x3d\x7f\x9f\x4d\x51\x58'
        res = websocket.WebSocketRequestHandler.decode_hybi(buf)

        self.assertEqual(res['fin'], 1)
        self.assertEqual(res['opcode'], 0x1)
        self.assertEqual(res['masked'], True)
        self.assertEqual(res['length'], 5)
        self.assertEqual(res['payload'], b'Hello')
        self.assertEqual(res['left'], 0)

    def test_decode_hybi_binary(self):
        buf = b'\x82\x04\x01\x02\x03\x04'
        res = websocket.WebSocketRequestHandler.decode_hybi(buf, strict=False)

        self.assertEqual(res['fin'], 1)
        self.assertEqual(res['opcode'], 0x2)
        self.assertEqual(res['length'], 4)
        self.assertEqual(res['payload'], b'\x01\x02\x03\x04')
        self.assertEqual(res['left'], 0)

    def test_decode_hybi_extended_16bit_binary(self):
        data = (b'\x01\x02\x03\x04' * 65)  # len > 126 -- len == 260
        buf = b'\x82\x7e\x01\x04' + data
        res = websocket.WebSocketRequestHandler.decode_hybi(buf, strict=False)

        self.assertEqual(res['fin'], 1)
        self.assertEqual(res['opcode'], 0x2)
        self.assertEqual(res['length'], 260)
        self.assertEqual(res['payload'], data)
        self.assertEqual(res['left'], 0)

    def test_decode_hybi_extended_64bit_binary(self):
        data = (b'\x01\x02\x03\x04' * 65)  # len > 126 -- len == 260
        buf = b'\x82\x7f\x00\x00\x00\x00\x00\x00\x01\x04' + data
        res = websocket.WebSocketRequestHandler.decode_hybi(buf, strict=False)

        self.assertEqual(res['fin'], 1)
        self.assertEqual(res['opcode'], 0x2)
        self.assertEqual(res['length'], 260)
        self.assertEqual(res['payload'], data)
        self.assertEqual(res['left'], 0)

    def test_decode_hybi_multi(self):
        buf1 = b'\x01\x03\x48\x65\x6c'
        buf2 = b'\x80\x02\x6c\x6f'

        res1 = websocket.WebSocketRequestHandler.decode_hybi(buf1, strict=False)
        self.assertEqual(res1['fin'], 0)
        self.assertEqual(res1['opcode'], 0x1)
        self.assertEqual(res1['length'], 3)
        self.assertEqual(res1['payload'], b'Hel')
        self.assertEqual(res1['left'], 0)

        res2 = websocket.WebSocketRequestHandler.decode_hybi(buf2, strict=False)
        self.assertEqual(res2['fin'], 1)
        self.assertEqual(res2['opcode'], 0x0)
        self.assertEqual(res2['length'], 2)
        self.assertEqual(res2['payload'], b'lo')
        self.assertEqual(res2['left'], 0)

    def test_encode_hybi_basic(self):
        res = websocket.WebSocketRequestHandler.encode_hybi(b'Hello', 0x1)
        expected = (b'\x81\x05\x48\x65\x6c\x6c\x6f', 2, 0)

        self.assertEqual(res, expected)

    def test_strict_mode_refuses_unmasked_client_frames(self):
        buf = b'\x81\x05\x48\x65\x6c\x6c\x6f'
        self.assertRaises(websocket.WebSocketRequestHandler.CClose,
                          websocket.WebSocketRequestHandler.decode_hybi,
                          buf)

    def test_no_strict_mode_accepts_unmasked_client_frames(self):
        buf = b'\x81\x05\x48\x65\x6c\x6c\x6f'
        res = websocket.WebSocketRequestHandler.decode_hybi(buf, strict=False)

        self.assertEqual(res['fin'], 1)
        self.assertEqual(res['opcode'], 0x1)
        self.assertEqual(res['masked'], False)
        self.assertEqual(res['length'], 5)
        self.assertEqual(res['payload'], b'Hello')
