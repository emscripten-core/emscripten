#!/usr/bin/env python

'''
Python WebSocket library with support for "wss://" encryption.
Copyright 2011 Joel Martin
Licensed under LGPL version 3 (see docs/LICENSE.LGPL-3)

Supports following protocol versions:
    - http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-07
    - http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-10
    - http://tools.ietf.org/html/rfc6455

You can make a cert/key with openssl using:
openssl req -new -x509 -days 365 -nodes -out self.pem -keyout self.pem
as taken from http://docs.python.org/dev/library/ssl.html#certificates

'''

import os, sys, time, errno, signal, socket, select, logging
import array, struct
from base64 import b64encode, b64decode

# Imports that vary by python version

# python 3.0 differences
if sys.hexversion > 0x3000000:
    b2s = lambda buf: buf.decode('latin_1')
    s2b = lambda s: s.encode('latin_1')
    s2a = lambda s: s
else:
    b2s = lambda buf: buf  # No-op
    s2b = lambda s: s      # No-op
    s2a = lambda s: [ord(c) for c in s]
try:    from io import StringIO
except: from cStringIO import StringIO
try:    from http.server import SimpleHTTPRequestHandler
except: from SimpleHTTPServer import SimpleHTTPRequestHandler

# python 2.6 differences
try:    from hashlib import sha1
except: from sha import sha as sha1

# python 2.5 differences
try:
    from struct import pack, unpack_from
except:
    from struct import pack
    def unpack_from(fmt, buf, offset=0):
        slice = buffer(buf, offset, struct.calcsize(fmt))
        return struct.unpack(fmt, slice)

# Degraded functionality if these imports are missing
for mod, msg in [('numpy', 'HyBi protocol will be slower'),
                 ('ssl', 'TLS/SSL/wss is disabled'),
                 ('multiprocessing', 'Multi-Processing is disabled'),
                 ('resource', 'daemonizing is disabled')]:
    try:
        globals()[mod] = __import__(mod)
    except ImportError:
        globals()[mod] = None
        print("WARNING: no '%s' module, %s" % (mod, msg))

if multiprocessing and sys.platform == 'win32':
    # make sockets pickle-able/inheritable
    import multiprocessing.reduction


# HTTP handler with WebSocket upgrade support
class WebSocketRequestHandler(SimpleHTTPRequestHandler):
    """
    WebSocket Request Handler Class, derived from SimpleHTTPRequestHandler.
    Must be sub-classed with new_websocket_client method definition.
    The request handler can be configured by setting optional
    attributes on the server object:

    * only_upgrade: If true, SimpleHTTPRequestHandler will not be enabled,
      only websocket is allowed.
    * verbose: If true, verbose logging is activated.
    * daemon: Running as daemon, do not write to console etc
    * record: Record raw frame data as JavaScript array into specified filename
    * run_once: Handle a single request
    * handler_id: A sequence number for this connection, appended to record filename
    """
    buffer_size = 65536

    GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

    server_version = "WebSockify"

    protocol_version = "HTTP/1.1"

    # An exception while the WebSocket client was connected
    class CClose(Exception):
        pass

    def __init__(self, req, addr, server):
        # Retrieve a few configuration variables from the server
        self.only_upgrade = getattr(server, "only_upgrade", False)
        self.verbose = getattr(server, "verbose", False)
        self.daemon = getattr(server, "daemon", False)
        self.record = getattr(server, "record", False)
        self.run_once = getattr(server, "run_once", False)
        self.rec        = None
        self.handler_id = getattr(server, "handler_id", False)
        self.file_only = getattr(server, "file_only", False)
        self.traffic = getattr(server, "traffic", False)
        self.auto_pong = getattr(server, "auto_pong", False)
        self.strict_mode = getattr(server, "strict_mode", True)

        self.logger = getattr(server, "logger", None)
        if self.logger is None:
            self.logger = WebSocketServer.get_logger()

        SimpleHTTPRequestHandler.__init__(self, req, addr, server)

    def log_message(self, format, *args):
        self.logger.info("%s - - [%s] %s" % (self.address_string(), self.log_date_time_string(), format % args))

    @staticmethod
    def unmask(buf, hlen, plen):
        pstart = hlen + 4
        pend = pstart + plen
        if numpy:
            b = c = s2b('')
            if plen >= 4:
                dtype=numpy.dtype('<u4')
                if sys.byteorder == 'big':
                    dtype = dtype.newbyteorder('>')
                mask = numpy.frombuffer(buf, dtype, offset=hlen, count=1)
                data = numpy.frombuffer(buf, dtype, offset=pstart,
                        count=int(plen / 4))
                #b = numpy.bitwise_xor(data, mask).data
                b = numpy.bitwise_xor(data, mask).tostring()

            if plen % 4:
                #self.msg("Partial unmask")
                dtype=numpy.dtype('B')
                if sys.byteorder == 'big':
                    dtype = dtype.newbyteorder('>')
                mask = numpy.frombuffer(buf, dtype, offset=hlen,
                        count=(plen % 4))
                data = numpy.frombuffer(buf, dtype,
                        offset=pend - (plen % 4), count=(plen % 4))
                c = numpy.bitwise_xor(data, mask).tostring()
            return b + c
        else:
            # Slower fallback
            mask = buf[hlen:hlen+4]
            data = array.array('B')
            mask = s2a(mask)
            data.fromstring(buf[pstart:pend])
            for i in range(len(data)):
                data[i] ^= mask[i % 4]
            return data.tostring()

    @staticmethod
    def encode_hybi(buf, opcode, base64=False):
        """ Encode a HyBi style WebSocket frame.
        Optional opcode:
            0x0 - continuation
            0x1 - text frame (base64 encode buf)
            0x2 - binary frame (use raw buf)
            0x8 - connection close
            0x9 - ping
            0xA - pong
        """
        if base64:
            buf = b64encode(buf)

        b1 = 0x80 | (opcode & 0x0f) # FIN + opcode
        payload_len = len(buf)
        if payload_len <= 125:
            header = pack('>BB', b1, payload_len)
        elif payload_len > 125 and payload_len < 65536:
            header = pack('>BBH', b1, 126, payload_len)
        elif payload_len >= 65536:
            header = pack('>BBQ', b1, 127, payload_len)

        #self.msg("Encoded: %s", repr(header + buf))

        return header + buf, len(header), 0

    @staticmethod
    def decode_hybi(buf, base64=False, logger=None, strict=True):
        """ Decode HyBi style WebSocket packets.
        Returns:
            {'fin'          : 0_or_1,
             'opcode'       : number,
             'masked'       : boolean,
             'hlen'         : header_bytes_number,
             'length'       : payload_bytes_number,
             'payload'      : decoded_buffer,
             'left'         : bytes_left_number,
             'close_code'   : number,
             'close_reason' : string}
        """

        f = {'fin'          : 0,
             'opcode'       : 0,
             'masked'       : False,
             'hlen'         : 2,
             'length'       : 0,
             'payload'      : None,
             'left'         : 0,
             'close_code'   : 1000,
             'close_reason' : ''}

        if logger is None:
            logger = WebSocketServer.get_logger()

        blen = len(buf)
        f['left'] = blen

        if blen < f['hlen']:
            return f # Incomplete frame header

        b1, b2 = unpack_from(">BB", buf)
        f['opcode'] = b1 & 0x0f
        f['fin'] = (b1 & 0x80) >> 7
        f['masked'] = (b2 & 0x80) >> 7

        f['length'] = b2 & 0x7f

        if f['length'] == 126:
            f['hlen'] = 4
            if blen < f['hlen']:
                return f # Incomplete frame header
            (f['length'],) = unpack_from('>xxH', buf)
        elif f['length'] == 127:
            f['hlen'] = 10
            if blen < f['hlen']:
                return f # Incomplete frame header
            (f['length'],) = unpack_from('>xxQ', buf)

        full_len = f['hlen'] + f['masked'] * 4 + f['length']

        if blen < full_len: # Incomplete frame
            return f # Incomplete frame header

        # Number of bytes that are part of the next frame(s)
        f['left'] = blen - full_len

        # Process 1 frame
        if f['masked']:
            # unmask payload
            f['payload'] = WebSocketRequestHandler.unmask(buf, f['hlen'],
                                                  f['length'])
        else:
            logger.debug("Unmasked frame: %s" % repr(buf))

            if strict:
                raise WebSocketRequestHandler.CClose(1002, "The client sent an unmasked frame.")

            f['payload'] = buf[(f['hlen'] + f['masked'] * 4):full_len]

        if base64 and f['opcode'] in [1, 2]:
            try:
                f['payload'] = b64decode(f['payload'])
            except:
                logger.exception("Exception while b64decoding buffer: %s" %
                                 (repr(buf)))
                raise

        if f['opcode'] == 0x08:
            if f['length'] >= 2:
                f['close_code'] = unpack_from(">H", f['payload'])[0]
            if f['length'] > 3:
                f['close_reason'] = f['payload'][2:]

        return f


    #
    # WebSocketRequestHandler logging/output functions
    #

    def print_traffic(self, token="."):
        """ Show traffic flow mode. """
        if self.traffic:
            sys.stdout.write(token)
            sys.stdout.flush()

    def msg(self, msg, *args, **kwargs):
        """ Output message with handler_id prefix. """
        prefix = "% 3d: " % self.handler_id
        self.logger.log(logging.INFO, "%s%s" % (prefix, msg), *args, **kwargs)

    def vmsg(self, msg, *args, **kwargs):
        """ Same as msg() but as debug. """
        prefix = "% 3d: " % self.handler_id
        self.logger.log(logging.DEBUG, "%s%s" % (prefix, msg), *args, **kwargs)

    def warn(self, msg, *args, **kwargs):
        """ Same as msg() but as warning. """
        prefix = "% 3d: " % self.handler_id
        self.logger.log(logging.WARN, "%s%s" % (prefix, msg), *args, **kwargs)

    #
    # Main WebSocketRequestHandler methods
    #
    def send_frames(self, bufs=None):
        """ Encode and send WebSocket frames. Any frames already
        queued will be sent first. If buf is not set then only queued
        frames will be sent. Returns the number of pending frames that
        could not be fully sent. If returned pending frames is greater
        than 0, then the caller should call again when the socket is
        ready. """

        tdelta = int(time.time()*1000) - self.start_time

        if bufs:
            for buf in bufs:
                if self.base64:
                    encbuf, lenhead, lentail = self.encode_hybi(buf, opcode=1, base64=True)
                else:
                    encbuf, lenhead, lentail = self.encode_hybi(buf, opcode=2, base64=False)

                if self.rec:
                    self.rec.write("%s,\n" %
                            repr("{%s{" % tdelta
                                + encbuf[lenhead:len(encbuf)-lentail]))

                self.send_parts.append(encbuf)

        while self.send_parts:
            # Send pending frames
            buf = self.send_parts.pop(0)
            sent = self.request.send(buf)

            if sent == len(buf):
                self.print_traffic("<")
            else:
                self.print_traffic("<.")
                self.send_parts.insert(0, buf[sent:])
                break

        return len(self.send_parts)

    def recv_frames(self):
        """ Receive and decode WebSocket frames.

        Returns:
            (bufs_list, closed_string)
        """

        closed = False
        bufs = []
        tdelta = int(time.time()*1000) - self.start_time

        buf = self.request.recv(self.buffer_size)
        if len(buf) == 0:
            closed = {'code': 1000, 'reason': "Client closed abruptly"}
            return bufs, closed

        if self.recv_part:
            # Add partially received frames to current read buffer
            buf = self.recv_part + buf
            self.recv_part = None

        while buf:
            frame = self.decode_hybi(buf, base64=self.base64,
                                     logger=self.logger,
                                     strict=self.strict_mode)
            #self.msg("Received buf: %s, frame: %s", repr(buf), frame)

            if frame['payload'] == None:
                # Incomplete/partial frame
                self.print_traffic("}.")
                if frame['left'] > 0:
                    self.recv_part = buf[-frame['left']:]
                break
            else:
                if frame['opcode'] == 0x8: # connection close
                    closed = {'code': frame['close_code'],
                              'reason': frame['close_reason']}
                    break
                elif self.auto_pong and frame['opcode'] == 0x9: # ping
                    self.print_traffic("} ping %s\n" %
                        repr(frame['payload']))
                    self.send_pong(frame['payload'])
                    return [], False
                elif frame['opcode'] == 0xA: # pong
                    self.print_traffic("} pong %s\n" %
                        repr(frame['payload']))
                    return [], False

            self.print_traffic("}")

            if self.rec:
                start = frame['hlen']
                end = frame['hlen'] + frame['length']
                if frame['masked']:
                    recbuf = WebSocketRequestHandler.unmask(buf, frame['hlen'],
                                                   frame['length'])
                else:
                    recbuf = buf[frame['hlen']:frame['hlen'] +
                                               frame['length']]
                self.rec.write("%s,\n" %
                        repr("}%s}" % tdelta + recbuf))


            bufs.append(frame['payload'])

            if frame['left']:
                buf = buf[-frame['left']:]
            else:
                buf = ''

        return bufs, closed

    def send_close(self, code=1000, reason=''):
        """ Send a WebSocket orderly close frame. """

        msg = pack(">H%ds" % len(reason), code, s2b(reason))
        buf, h, t = self.encode_hybi(msg, opcode=0x08, base64=False)
        self.request.send(buf)

    def send_pong(self, data=''):
        """ Send a WebSocket pong frame. """
        buf, h, t = self.encode_hybi(s2b(data), opcode=0x0A, base64=False)
        self.request.send(buf)

    def send_ping(self, data=''):
        """ Send a WebSocket ping frame. """
        buf, h, t = self.encode_hybi(s2b(data), opcode=0x09, base64=False)
        self.request.send(buf)

    def do_websocket_handshake(self):
        h = self.headers

        prot = 'WebSocket-Protocol'
        protocols = h.get('Sec-'+prot, h.get(prot, '')).split(',')

        ver = h.get('Sec-WebSocket-Version')
        if ver:
            # HyBi/IETF version of the protocol

            # HyBi-07 report version 7
            # HyBi-08 - HyBi-12 report version 8
            # HyBi-13 reports version 13
            if ver in ['7', '8', '13']:
                self.version = "hybi-%02d" % int(ver)
            else:
                self.send_error(400, "Unsupported protocol version %s" % ver)
                return False

            key = h['Sec-WebSocket-Key']

            # Choose binary if client supports it
            if 'binary' in protocols:
                self.base64 = False
            elif 'base64' in protocols:
                self.base64 = True
            else:
                self.send_error(400, "Client must support 'binary' or 'base64' protocol")
                return False

            # Generate the hash value for the accept header
            accept = b64encode(sha1(s2b(key + self.GUID)).digest())

            self.send_response(101, "Switching Protocols")
            self.send_header("Upgrade", "websocket")
            self.send_header("Connection", "Upgrade")
            self.send_header("Sec-WebSocket-Accept", b2s(accept))
            if self.base64:
                self.send_header("Sec-WebSocket-Protocol", "base64")
            else:
                self.send_header("Sec-WebSocket-Protocol", "binary")
            self.end_headers()
            return True
        else:
            self.send_error(400, "Missing Sec-WebSocket-Version header. Hixie protocols not supported.")

        return False

    def handle_websocket(self):
        """Upgrade a connection to Websocket, if requested. If this succeeds,
        new_websocket_client() will be called. Otherwise, False is returned.
        """

        if (self.headers.get('upgrade') and
            self.headers.get('upgrade').lower() == 'websocket'):

            # ensure connection is authorized, and determine the target
            self.validate_connection()

            if not self.do_websocket_handshake():
                return False

            # Indicate to server that a Websocket upgrade was done
            self.server.ws_connection = True
            # Initialize per client settings
            self.send_parts = []
            self.recv_part  = None
            self.start_time = int(time.time()*1000)

            # client_address is empty with, say, UNIX domain sockets
            client_addr = ""
            is_ssl = False
            try:
                client_addr = self.client_address[0]
                is_ssl = self.client_address[2]
            except IndexError:
                pass

            if is_ssl:
                self.stype = "SSL/TLS (wss://)"
            else:
                self.stype = "Plain non-SSL (ws://)"

            self.log_message("%s: %s WebSocket connection", client_addr,
                             self.stype)
            self.log_message("%s: Version %s, base64: '%s'", client_addr,
                             self.version, self.base64)
            if self.path != '/':
                self.log_message("%s: Path: '%s'", client_addr, self.path)

            if self.record:
                # Record raw frame data as JavaScript array
                fname = "%s.%s" % (self.record,
                                   self.handler_id)
                self.log_message("opening record file: %s", fname)
                self.rec = open(fname, 'w+')
                encoding = "binary"
                if self.base64: encoding = "base64"
                self.rec.write("var VNC_frame_encoding = '%s';\n"
                               % encoding)
                self.rec.write("var VNC_frame_data = [\n")

            try:
                self.new_websocket_client()
            except self.CClose:
                # Close the client
                _, exc, _ = sys.exc_info()
                self.send_close(exc.args[0], exc.args[1])
            return True
        else:
            return False

    def do_GET(self):
        """Handle GET request. Calls handle_websocket(). If unsuccessful,
        and web server is enabled, SimpleHTTPRequestHandler.do_GET will be called."""
        if not self.handle_websocket():
            if self.only_upgrade:
                self.send_error(405, "Method Not Allowed")
            else:
                SimpleHTTPRequestHandler.do_GET(self)

    def list_directory(self, path):
        if self.file_only:
            self.send_error(404, "No such file")
        else:
            return SimpleHTTPRequestHandler.list_directory(self, path)

    def new_websocket_client(self):
        """ Do something with a WebSockets client connection. """
        raise Exception("WebSocketRequestHandler.new_websocket_client() must be overloaded")

    def validate_connection(self):
        """ Ensure that the connection is a valid connection, and set the target. """
        pass

    def do_HEAD(self):
        if self.only_upgrade:
            self.send_error(405, "Method Not Allowed")
        else:
            SimpleHTTPRequestHandler.do_HEAD(self)

    def finish(self):
        if self.rec:
            self.rec.write("'EOF'];\n")
            self.rec.close()

    def handle(self):
        # When using run_once, we have a single process, so
        # we cannot loop in BaseHTTPRequestHandler.handle; we
        # must return and handle new connections
        if self.run_once:
            self.handle_one_request()
        else:
            SimpleHTTPRequestHandler.handle(self)

    def log_request(self, code='-', size='-'):
        if self.verbose:
            SimpleHTTPRequestHandler.log_request(self, code, size)


class WebSocketServer(object):
    """
    WebSockets server class.
    As an alternative, the standard library SocketServer can be used
    """

    policy_response = """<cross-domain-policy><allow-access-from domain="*" to-ports="*" /></cross-domain-policy>\n"""
    log_prefix = "websocket"

    # An exception before the WebSocket connection was established
    class EClose(Exception):
        pass

    class Terminate(Exception):
        pass

    def __init__(self, RequestHandlerClass, listen_host='',
                 listen_port=None, source_is_ipv6=False,
            verbose=False, cert='', key='', ssl_only=None,
            daemon=False, record='', web='',
            file_only=False,
            run_once=False, timeout=0, idle_timeout=0, traffic=False,
            tcp_keepalive=True, tcp_keepcnt=None, tcp_keepidle=None,
            tcp_keepintvl=None, auto_pong=False, strict_mode=True):

        # settings
        self.RequestHandlerClass = RequestHandlerClass
        self.verbose        = verbose
        self.listen_host    = listen_host
        self.listen_port    = listen_port
        self.prefer_ipv6    = source_is_ipv6
        self.ssl_only       = ssl_only
        self.daemon         = daemon
        self.run_once       = run_once
        self.timeout        = timeout
        self.idle_timeout   = idle_timeout
        self.traffic        = traffic
        self.file_only      = file_only
        self.strict_mode    = strict_mode

        self.launch_time    = time.time()
        self.ws_connection  = False
        self.handler_id     = 1

        self.logger         = self.get_logger()
        self.tcp_keepalive  = tcp_keepalive
        self.tcp_keepcnt    = tcp_keepcnt
        self.tcp_keepidle   = tcp_keepidle
        self.tcp_keepintvl  = tcp_keepintvl

        self.auto_pong      = auto_pong
        # Make paths settings absolute
        self.cert = os.path.abspath(cert)
        self.key = self.web = self.record = ''
        if key:
            self.key = os.path.abspath(key)
        if web:
            self.web = os.path.abspath(web)
        if record:
            self.record = os.path.abspath(record)

        if self.web:
            os.chdir(self.web)
        self.only_upgrade = not self.web

        # Sanity checks
        if not ssl and self.ssl_only:
            raise Exception("No 'ssl' module and SSL-only specified")
        if self.daemon and not resource:
            raise Exception("Module 'resource' required to daemonize")

        # Show configuration
        self.msg("WebSocket server settings:")
        self.msg("  - Listen on %s:%s",
                self.listen_host, self.listen_port)
        self.msg("  - Flash security policy server")
        if self.web:
            if self.file_only:
                self.msg("  - Web server (no directory listings). Web root: %s", self.web)
            else:
                self.msg("  - Web server. Web root: %s", self.web)
        if ssl:
            if os.path.exists(self.cert):
                self.msg("  - SSL/TLS support")
                if self.ssl_only:
                    self.msg("  - Deny non-SSL/TLS connections")
            else:
                self.msg("  - No SSL/TLS support (no cert file)")
        else:
            self.msg("  - No SSL/TLS support (no 'ssl' module)")
        if self.daemon:
            self.msg("  - Backgrounding (daemon)")
        if self.record:
            self.msg("  - Recording to '%s.*'", self.record)

    #
    # WebSocketServer static methods
    #

    @staticmethod
    def get_logger():
        return logging.getLogger("%s.%s" % (
            WebSocketServer.log_prefix,
            WebSocketServer.__class__.__name__))

    @staticmethod
    def socket(host, port=None, connect=False, prefer_ipv6=False,
               unix_socket=None, use_ssl=False, tcp_keepalive=True,
               tcp_keepcnt=None, tcp_keepidle=None, tcp_keepintvl=None):
        """ Resolve a host (and optional port) to an IPv4 or IPv6
        address. Create a socket. Bind to it if listen is set,
        otherwise connect to it. Return the socket.
        """
        flags = 0
        if host == '':
            host = None
        if connect and not (port or unix_socket):
            raise Exception("Connect mode requires a port")
        if use_ssl and not ssl:
            raise Exception("SSL socket requested but Python SSL module not loaded.");
        if not connect and use_ssl:
            raise Exception("SSL only supported in connect mode (for now)")
        if not connect:
            flags = flags | socket.AI_PASSIVE

        if not unix_socket:
            addrs = socket.getaddrinfo(host, port, 0, socket.SOCK_STREAM,
                    socket.IPPROTO_TCP, flags)
            if not addrs:
                raise Exception("Could not resolve host '%s'" % host)
            addrs.sort(key=lambda x: x[0])
            if prefer_ipv6:
                addrs.reverse()
            sock = socket.socket(addrs[0][0], addrs[0][1])

            if  tcp_keepalive:
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
                if tcp_keepcnt:
                    sock.setsockopt(socket.SOL_TCP, socket.TCP_KEEPCNT,
                                    tcp_keepcnt)
                if tcp_keepidle:
                    sock.setsockopt(socket.SOL_TCP, socket.TCP_KEEPIDLE,
                                    tcp_keepidle)
                if tcp_keepintvl:
                    sock.setsockopt(socket.SOL_TCP, socket.TCP_KEEPINTVL,
                                    tcp_keepintvl)

            if connect:
                sock.connect(addrs[0][4])
                if use_ssl:
                    sock = ssl.wrap_socket(sock)
            else:
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                sock.bind(addrs[0][4])
                sock.listen(100)
        else:
            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            sock.connect(unix_socket)

        return sock

    @staticmethod
    def daemonize(keepfd=None, chdir='/'):
        
        if keepfd is None:
            keepfd = []

        os.umask(0)
        if chdir:
            os.chdir(chdir)
        else:
            os.chdir('/')
        os.setgid(os.getgid())  # relinquish elevations
        os.setuid(os.getuid())  # relinquish elevations

        # Double fork to daemonize
        if os.fork() > 0: os._exit(0)  # Parent exits
        os.setsid()                    # Obtain new process group
        if os.fork() > 0: os._exit(0)  # Parent exits

        # Signal handling
        signal.signal(signal.SIGTERM, signal.SIG_IGN)
        signal.signal(signal.SIGINT, signal.SIG_IGN)

        # Close open files
        maxfd = resource.getrlimit(resource.RLIMIT_NOFILE)[1]
        if maxfd == resource.RLIM_INFINITY: maxfd = 256
        for fd in reversed(range(maxfd)):
            try:
                if fd not in keepfd:
                    os.close(fd)
            except OSError:
                _, exc, _ = sys.exc_info()
                if exc.errno != errno.EBADF: raise

        # Redirect I/O to /dev/null
        os.dup2(os.open(os.devnull, os.O_RDWR), sys.stdin.fileno())
        os.dup2(os.open(os.devnull, os.O_RDWR), sys.stdout.fileno())
        os.dup2(os.open(os.devnull, os.O_RDWR), sys.stderr.fileno())

    def do_handshake(self, sock, address):
        """
        do_handshake does the following:
        - Peek at the first few bytes from the socket.
        - If the connection is Flash policy request then answer it,
          close the socket and return.
        - If the connection is an HTTPS/SSL/TLS connection then SSL
          wrap the socket.
        - Read from the (possibly wrapped) socket.
        - If we have received a HTTP GET request and the webserver
          functionality is enabled, answer it, close the socket and
          return.
        - Assume we have a WebSockets connection, parse the client
          handshake data.
        - Send a WebSockets handshake server response.
        - Return the socket for this WebSocket client.
        """
        ready = select.select([sock], [], [], 3)[0]


        if not ready:
            raise self.EClose("ignoring socket not ready")
        # Peek, but do not read the data so that we have a opportunity
        # to SSL wrap the socket first
        handshake = sock.recv(1024, socket.MSG_PEEK)
        #self.msg("Handshake [%s]" % handshake)

        if not handshake:
            raise self.EClose("ignoring empty handshake")

        elif handshake.startswith(s2b("<policy-file-request/>")):
            # Answer Flash policy request
            handshake = sock.recv(1024)
            sock.send(s2b(self.policy_response))
            raise self.EClose("Sending flash policy response")

        elif handshake[0] in ("\x16", "\x80", 22, 128):
            # SSL wrap the connection
            if not ssl:
                raise self.EClose("SSL connection but no 'ssl' module")
            if not os.path.exists(self.cert):
                raise self.EClose("SSL connection but '%s' not found"
                                  % self.cert)
            retsock = None
            try:
                retsock = ssl.wrap_socket(
                        sock,
                        server_side=True,
                        certfile=self.cert,
                        keyfile=self.key)
            except ssl.SSLError:
                _, x, _ = sys.exc_info()
                if x.args[0] == ssl.SSL_ERROR_EOF:
                    if len(x.args) > 1:
                        raise self.EClose(x.args[1])
                    else:
                        raise self.EClose("Got SSL_ERROR_EOF")
                else:
                    raise

        elif self.ssl_only:
            raise self.EClose("non-SSL connection received but disallowed")

        else:
            retsock = sock

        # If the address is like (host, port), we are extending it
        # with a flag indicating SSL. Not many other options
        # available...
        if len(address) == 2:
            address = (address[0], address[1], (retsock != sock))

        self.RequestHandlerClass(retsock, address, self)

        # Return the WebSockets socket which may be SSL wrapped
        return retsock

    #
    # WebSocketServer logging/output functions
    #

    def msg(self, *args, **kwargs):
        """ Output message as info """
        self.logger.log(logging.INFO, *args, **kwargs)

    def vmsg(self, *args, **kwargs):
        """ Same as msg() but as debug. """
        self.logger.log(logging.DEBUG, *args, **kwargs)

    def warn(self, *args, **kwargs):
        """ Same as msg() but as warning. """
        self.logger.log(logging.WARN, *args, **kwargs)


    #
    # Events that can/should be overridden in sub-classes
    #
    def started(self):
        """ Called after WebSockets startup """
        self.vmsg("WebSockets server started")

    def poll(self):
        """ Run periodically while waiting for connections. """
        #self.vmsg("Running poll()")
        pass

    def terminate(self):
        raise self.Terminate()

    def multiprocessing_SIGCHLD(self, sig, stack):
        # TODO: figure out a way to actually log this information without
        #       calling `log` in the signal handlers
        multiprocessing.active_children()

    def fallback_SIGCHLD(self, sig, stack):
        # Reap zombies when using os.fork() (python 2.4)
        # TODO: figure out a way to actually log this information without
        #       calling `log` in the signal handlers
        try:
            result = os.waitpid(-1, os.WNOHANG)
            while result[0]:
                self.vmsg("Reaped child process %s" % result[0])
                result = os.waitpid(-1, os.WNOHANG)
        except (OSError):
            pass

    def do_SIGINT(self, sig, stack):
        # TODO: figure out a way to actually log this information without
        #       calling `log` in the signal handlers
        self.terminate()

    def do_SIGTERM(self, sig, stack):
        # TODO: figure out a way to actually log this information without
        #       calling `log` in the signal handlers
        self.terminate()

    def top_new_client(self, startsock, address):
        """ Do something with a WebSockets client connection. """
        # handler process
        client = None
        try:
            try:
                client = self.do_handshake(startsock, address)
            except self.EClose:
                _, exc, _ = sys.exc_info()
                # Connection was not a WebSockets connection
                if exc.args[0]:
                    self.msg("%s: %s" % (address[0], exc.args[0]))
            except WebSocketServer.Terminate:
                raise
            except Exception:
                _, exc, _ = sys.exc_info()
                self.msg("handler exception: %s" % str(exc))
                self.vmsg("exception", exc_info=True)
        finally:

            if client and client != startsock:
                # Close the SSL wrapped socket
                # Original socket closed by caller
                client.close()

    def get_log_fd(self):
        """
        Get file descriptors for the loggers.
        They should not be closed when the process is forked.
        """
        descriptors = []
        for handler in self.logger.parent.handlers:
            if isinstance(handler, logging.FileHandler):
                descriptors.append(handler.stream.fileno())

        return descriptors

    def start_server(self):
        """
        Daemonize if requested. Listen for for connections. Run
        do_handshake() method for each connection. If the connection
        is a WebSockets client then call new_websocket_client() method (which must
        be overridden) for each new client connection.
        """
        lsock = self.socket(self.listen_host, self.listen_port, False,
                            self.prefer_ipv6,
                            tcp_keepalive=self.tcp_keepalive,
                            tcp_keepcnt=self.tcp_keepcnt,
                            tcp_keepidle=self.tcp_keepidle,
                            tcp_keepintvl=self.tcp_keepintvl)

        if self.daemon:
            keepfd = self.get_log_fd()
            keepfd.append(lsock.fileno())
            self.daemonize(keepfd=keepfd, chdir=self.web)

        self.started()  # Some things need to happen after daemonizing

        # Allow override of signals
        original_signals = {
            signal.SIGINT: signal.getsignal(signal.SIGINT),
            signal.SIGTERM: signal.getsignal(signal.SIGTERM),
            signal.SIGCHLD: signal.getsignal(signal.SIGCHLD),
        }
        signal.signal(signal.SIGINT, self.do_SIGINT)
        signal.signal(signal.SIGTERM, self.do_SIGTERM)
        if not multiprocessing:
            # os.fork() (python 2.4) child reaper
            signal.signal(signal.SIGCHLD, self.fallback_SIGCHLD)
        else:
            # make sure that _cleanup is called when children die
            # by calling active_children on SIGCHLD
            signal.signal(signal.SIGCHLD, self.multiprocessing_SIGCHLD)

        last_active_time = self.launch_time
        try:
            while True:
                try:
                    try:
                        startsock = None
                        pid = err = 0
                        child_count = 0

                        if multiprocessing:
                            # Collect zombie child processes
                            child_count = len(multiprocessing.active_children())

                        time_elapsed = time.time() - self.launch_time
                        if self.timeout and time_elapsed > self.timeout:
                            self.msg('listener exit due to --timeout %s'
                                    % self.timeout)
                            break

                        if self.idle_timeout:
                            idle_time = 0
                            if child_count == 0:
                                idle_time = time.time() - last_active_time
                            else:
                                idle_time = 0
                                last_active_time = time.time()

                            if idle_time > self.idle_timeout and child_count == 0:
                                self.msg('listener exit due to --idle-timeout %s'
                                            % self.idle_timeout)
                                break

                        try:
                            self.poll()

                            ready = select.select([lsock], [], [], 1)[0]
                            if lsock in ready:
                                startsock, address = lsock.accept()
                            else:
                                continue
                        except self.Terminate:
                            raise
                        except Exception:
                            _, exc, _ = sys.exc_info()
                            if hasattr(exc, 'errno'):
                                err = exc.errno
                            elif hasattr(exc, 'args'):
                                err = exc.args[0]
                            else:
                                err = exc[0]
                            if err == errno.EINTR:
                                self.vmsg("Ignoring interrupted syscall")
                                continue
                            else:
                                raise

                        if self.run_once:
                            # Run in same process if run_once
                            self.top_new_client(startsock, address)
                            if self.ws_connection :
                                self.msg('%s: exiting due to --run-once'
                                        % address[0])
                                break
                        elif multiprocessing:
                            self.vmsg('%s: new handler Process' % address[0])
                            p = multiprocessing.Process(
                                    target=self.top_new_client,
                                    args=(startsock, address))
                            p.start()
                            # child will not return
                        else:
                            # python 2.4
                            self.vmsg('%s: forking handler' % address[0])
                            pid = os.fork()
                            if pid == 0:
                                # child handler process
                                self.top_new_client(startsock, address)
                                break  # child process exits

                        # parent process
                        self.handler_id += 1

                    except (self.Terminate, SystemExit, KeyboardInterrupt):
                        self.msg("In exit")
                        # terminate all child processes
                        if multiprocessing and not self.run_once:
                            children = multiprocessing.active_children()

                            for child in children:
                                self.msg("Terminating child %s" % child.pid)
                                child.terminate()

                        break
                    except Exception:
                        exc = sys.exc_info()[1]
                        self.msg("handler exception: %s", str(exc))
                        self.vmsg("exception", exc_info=True)

                finally:
                    if startsock:
                        startsock.close()
        finally:
            # Close listen port
            self.vmsg("Closing socket listening at %s:%s",
                      self.listen_host, self.listen_port)
            lsock.close()

            # Restore signals
            for sig, func in original_signals.items():
                signal.signal(sig, func)


