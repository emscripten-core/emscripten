# vim: tabstop=4 shiftwidth=4 softtabstop=4

# Copyright(c) 2015 Red Hat, Inc All Rights Reserved.
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

""" Unit tests for websocketproxy """

import unittest
import unittest
import socket

import stubout

from websockify import websocket
from websockify import websocketproxy
from websockify import token_plugins
from websockify import auth_plugins

try:
    from StringIO import StringIO
    BytesIO = StringIO
except ImportError:
    from io import StringIO
    from io import BytesIO


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


class FakeServer(object):
    class EClose(Exception):
        pass

    def __init__(self):
        self.token_plugin = None
        self.auth_plugin = None
        self.wrap_cmd = None
        self.ssl_target = None
        self.unix_target = None

class ProxyRequestHandlerTestCase(unittest.TestCase):
    def setUp(self):
        super(ProxyRequestHandlerTestCase, self).setUp()
        self.stubs = stubout.StubOutForTesting()
        self.handler = websocketproxy.ProxyRequestHandler(
            FakeSocket(''), "127.0.0.1", FakeServer())
        self.handler.path = "https://localhost:6080/websockify?token=blah"
        self.handler.headers = None
        self.stubs.Set(websocket.WebSocketServer, 'socket',
                       staticmethod(lambda *args, **kwargs: None))

    def tearDown(self):
        self.stubs.UnsetAll()
        super(ProxyRequestHandlerTestCase, self).tearDown()

    def test_get_target(self):
        class TestPlugin(token_plugins.BasePlugin):
            def lookup(self, token):
                return ("some host", "some port")

        host, port = self.handler.get_target(
            TestPlugin(None), self.handler.path)

        self.assertEqual(host, "some host")
        self.assertEqual(port, "some port")

    def test_get_target_raises_error_on_unknown_token(self):
        class TestPlugin(token_plugins.BasePlugin):
            def lookup(self, token):
                return None

        self.assertRaises(FakeServer.EClose, self.handler.get_target,
            TestPlugin(None), "https://localhost:6080/websockify?token=blah")

    def test_token_plugin(self):
        class TestPlugin(token_plugins.BasePlugin):
            def lookup(self, token):
                return (self.source + token).split(',')

        self.stubs.Set(websocketproxy.ProxyRequestHandler, 'send_auth_error',
                       staticmethod(lambda *args, **kwargs: None))

        self.handler.server.token_plugin = TestPlugin("somehost,")
        self.handler.validate_connection()

        self.assertEqual(self.handler.server.target_host, "somehost")
        self.assertEqual(self.handler.server.target_port, "blah")

    def test_auth_plugin(self):
        class TestPlugin(auth_plugins.BasePlugin):
            def authenticate(self, headers, target_host, target_port):
                if target_host == self.source:
                    raise auth_plugins.AuthenticationError(response_msg="some_error")

        self.stubs.Set(websocketproxy.ProxyRequestHandler, 'send_auth_error',
                       staticmethod(lambda *args, **kwargs: None))

        self.handler.server.auth_plugin = TestPlugin("somehost")
        self.handler.server.target_host = "somehost"
        self.handler.server.target_port = "someport"

        self.assertRaises(auth_plugins.AuthenticationError,
                          self.handler.validate_connection)

        self.handler.server.target_host = "someotherhost"
        self.handler.validate_connection()

