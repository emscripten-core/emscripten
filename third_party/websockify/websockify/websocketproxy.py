#!/usr/bin/env python

'''
A WebSocket to TCP socket proxy with support for "wss://" encryption.
Copyright 2011 Joel Martin
Licensed under LGPL version 3 (see docs/LICENSE.LGPL-3)

You can make a cert/key with openssl using:
openssl req -new -x509 -days 365 -nodes -out self.pem -keyout self.pem
as taken from http://docs.python.org/dev/library/ssl.html#certificates

'''

import signal, socket, optparse, time, os, sys, subprocess
from select import select
import websocket
try:
    from urllib.parse import parse_qs, urlparse
except:
    from cgi import parse_qs
    from urlparse import urlparse

class WebSocketProxy(websocket.WebSocketServer):
    """
    Proxy traffic to and from a WebSockets client to a normal TCP
    socket server target. All traffic to/from the client is base64
    encoded/decoded to allow binary data to be sent/received to/from
    the target.
    """

    buffer_size = 65536

    traffic_legend = """
Traffic Legend:
    }  - Client receive
    }. - Client receive partial
    {  - Target receive

    >  - Target send
    >. - Target send partial
    <  - Client send
    <. - Client send partial
"""

    def __init__(self, *args, **kwargs):
        # Save off proxy specific options
        self.target_host    = kwargs.pop('target_host', None)
        self.target_port    = kwargs.pop('target_port', None)
        self.wrap_cmd       = kwargs.pop('wrap_cmd', None)
        self.wrap_mode      = kwargs.pop('wrap_mode', None)
        self.unix_target    = kwargs.pop('unix_target', None)
        self.ssl_target     = kwargs.pop('ssl_target', None)
        self.target_cfg     = kwargs.pop('target_cfg', None)
        # Last 3 timestamps command was run
        self.wrap_times    = [0, 0, 0]

        if self.wrap_cmd:
            rebinder_path = ['./', os.path.dirname(sys.argv[0])]
            self.rebinder = None

            for rdir in rebinder_path:
                rpath = os.path.join(rdir, "rebind.so")
                if os.path.exists(rpath):
                    self.rebinder = rpath
                    break

            if not self.rebinder:
                raise Exception("rebind.so not found, perhaps you need to run make")
            self.rebinder = os.path.abspath(self.rebinder)

            self.target_host = "127.0.0.1"  # Loopback
            # Find a free high port
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.bind(('', 0))
            self.target_port = sock.getsockname()[1]
            sock.close()

            os.environ.update({
                "LD_PRELOAD": self.rebinder,
                "REBIND_OLD_PORT": str(kwargs['listen_port']),
                "REBIND_NEW_PORT": str(self.target_port)})

        if self.target_cfg:
            self.target_cfg = os.path.abspath(self.target_cfg)

        websocket.WebSocketServer.__init__(self, *args, **kwargs)

    def run_wrap_cmd(self):
        print("Starting '%s'" % " ".join(self.wrap_cmd))
        self.wrap_times.append(time.time())
        self.wrap_times.pop(0)
        self.cmd = subprocess.Popen(
                self.wrap_cmd, env=os.environ, preexec_fn=_subprocess_setup)
        self.spawn_message = True

    def started(self):
        """
        Called after Websockets server startup (i.e. after daemonize)
        """
        # Need to call wrapped command after daemonization so we can
        # know when the wrapped command exits
        if self.wrap_cmd:
            dst_string = "'%s' (port %s)" % (" ".join(self.wrap_cmd), self.target_port)
        elif self.unix_target:
            dst_string = self.unix_target
        else:
            dst_string = "%s:%s" % (self.target_host, self.target_port)

        if self.target_cfg:
            msg = "  - proxying from %s:%s to targets in %s" % (
                self.listen_host, self.listen_port, self.target_cfg)
        else:
            msg = "  - proxying from %s:%s to %s" % (
                self.listen_host, self.listen_port, dst_string)

        if self.ssl_target:
            msg += " (using SSL)"

        print(msg + "\n")

        if self.wrap_cmd:
            self.run_wrap_cmd()

    def poll(self):
        # If we are wrapping a command, check it's status

        if self.wrap_cmd and self.cmd:
            ret = self.cmd.poll()
            if ret != None:
                self.vmsg("Wrapped command exited (or daemon). Returned %s" % ret)
                self.cmd = None

        if self.wrap_cmd and self.cmd == None:
            # Response to wrapped command being gone
            if self.wrap_mode == "ignore":
                pass
            elif self.wrap_mode == "exit":
                sys.exit(ret)
            elif self.wrap_mode == "respawn":
                now = time.time()
                avg = sum(self.wrap_times)/len(self.wrap_times)
                if (now - avg) < 10:
                    # 3 times in the last 10 seconds
                    if self.spawn_message:
                        print("Command respawning too fast")
                        self.spawn_message = False
                else:
                    self.run_wrap_cmd()

    #
    # Routines above this point are run in the master listener
    # process.
    #

    #
    # Routines below this point are connection handler routines and
    # will be run in a separate forked process for each connection.
    #

    def new_client(self):
        """
        Called after a new WebSocket connection has been established.
        """
        # Checks if we receive a token, and look
        # for a valid target for it then
        if self.target_cfg:
            (self.target_host, self.target_port) = self.get_target(self.target_cfg, self.path)

        # Connect to the target
        if self.wrap_cmd:
            msg = "connecting to command: '%s' (port %s)" % (" ".join(self.wrap_cmd), self.target_port)
        elif self.unix_target:
            msg = "connecting to unix socket: %s" % self.unix_target
        else:
            msg = "connecting to: %s:%s" % (
                                    self.target_host, self.target_port)

        if self.ssl_target:
            msg += " (using SSL)"
        self.msg(msg)

        tsock = self.socket(self.target_host, self.target_port,
                connect=True, use_ssl=self.ssl_target, unix_socket=self.unix_target)

        if self.verbose and not self.daemon:
            print(self.traffic_legend)

        # Start proxying
        try:
            self.do_proxy(tsock)
        except:
            if tsock:
                tsock.shutdown(socket.SHUT_RDWR)
                tsock.close()
                self.vmsg("%s:%s: Closed target" %(
                    self.target_host, self.target_port))
            raise

    def get_target(self, target_cfg, path):
        """
        Parses the path, extracts a token, and looks for a valid
        target for that token in the configuration file(s). Sets
        target_host and target_port if successful
        """
        # The files in targets contain the lines
        # in the form of token: host:port

        # Extract the token parameter from url
        args = parse_qs(urlparse(path)[4]) # 4 is the query from url

        if not args.has_key('token') or not len(args['token']):
            raise self.EClose("Token not present")

        token = args['token'][0].rstrip('\n')

        # target_cfg can be a single config file or directory of
        # config files
        if os.path.isdir(target_cfg):
            cfg_files = [os.path.join(target_cfg, f)
                         for f in os.listdir(target_cfg)]
        else:
            cfg_files = [target_cfg]

        targets = {}
        for f in cfg_files:
            for line in [l.strip() for l in file(f).readlines()]:
                if line and not line.startswith('#'):
                    ttoken, target = line.split(': ')
                    targets[ttoken] = target.strip()

        self.vmsg("Target config: %s" % repr(targets))

        if targets.has_key(token):
            return targets[token].split(':')
        else:
            raise self.EClose("Token '%s' not found" % token)

    def do_proxy(self, target):
        """
        Proxy client WebSocket to normal target socket.
        """
        cqueue = []
        c_pend = 0
        tqueue = []
        rlist = [self.client, target]

        while True:
            wlist = []

            if tqueue: wlist.append(target)
            if cqueue or c_pend: wlist.append(self.client)
            ins, outs, excepts = select(rlist, wlist, [], 1)
            if excepts: raise Exception("Socket exception")

            if self.client in outs:
                # Send queued target data to the client
                c_pend = self.send_frames(cqueue)

                cqueue = []

            if self.client in ins:
                # Receive client data, decode it, and queue for target
                bufs, closed = self.recv_frames()
                tqueue.extend(bufs)

                if closed:
                    # TODO: What about blocking on client socket?
                    self.vmsg("%s:%s: Client closed connection" %(
                        self.target_host, self.target_port))
                    raise self.CClose(closed['code'], closed['reason'])


            if target in outs:
                # Send queued client data to the target
                dat = tqueue.pop(0)
                sent = target.send(dat)
                if sent == len(dat):
                    self.traffic(">")
                else:
                    # requeue the remaining data
                    tqueue.insert(0, dat[sent:])
                    self.traffic(".>")


            if target in ins:
                # Receive target data, encode it and queue for client
                buf = target.recv(self.buffer_size)
                if len(buf) == 0:
                    self.vmsg("%s:%s: Target closed connection" %(
                        self.target_host, self.target_port))
                    raise self.CClose(1000, "Target closed")

                cqueue.append(buf)
                self.traffic("{")



def _subprocess_setup():
    # Python installs a SIGPIPE handler by default. This is usually not what
    # non-Python successfulbprocesses expect.
    signal.signal(signal.SIGPIPE, signal.SIG_DFL)


def websockify_init():
    usage = "\n    %prog [options]"
    usage += " [source_addr:]source_port [target_addr:target_port]"
    usage += "\n    %prog [options]"
    usage += " [source_addr:]source_port -- WRAP_COMMAND_LINE"
    parser = optparse.OptionParser(usage=usage)
    parser.add_option("--verbose", "-v", action="store_true",
            help="verbose messages and per frame traffic")
    parser.add_option("--record",
            help="record sessions to FILE.[session_number]", metavar="FILE")
    parser.add_option("--daemon", "-D",
            dest="daemon", action="store_true",
            help="become a daemon (background process)")
    parser.add_option("--run-once", action="store_true",
            help="handle a single WebSocket connection and exit")
    parser.add_option("--timeout", type=int, default=0,
            help="after TIMEOUT seconds exit when not connected")
    parser.add_option("--idle-timeout", type=int, default=0,
            help="server exits after TIMEOUT seconds if there are no "
                 "active connections")
    parser.add_option("--cert", default="self.pem",
            help="SSL certificate file")
    parser.add_option("--key", default=None,
            help="SSL key file (if separate from cert)")
    parser.add_option("--ssl-only", action="store_true",
            help="disallow non-encrypted client connections")
    parser.add_option("--ssl-target", action="store_true",
            help="connect to SSL target as SSL client")
    parser.add_option("--unix-target",
            help="connect to unix socket target", metavar="FILE")
    parser.add_option("--web", default=None, metavar="DIR",
            help="run webserver on same port. Serve files from DIR.")
    parser.add_option("--wrap-mode", default="exit", metavar="MODE",
            choices=["exit", "ignore", "respawn"],
            help="action to take when the wrapped program exits "
            "or daemonizes: exit (default), ignore, respawn")
    parser.add_option("--prefer-ipv6", "-6",
            action="store_true", dest="source_is_ipv6",
            help="prefer IPv6 when resolving source_addr")
    parser.add_option("--target-config", metavar="FILE",
            dest="target_cfg",
            help="Configuration file containing valid targets "
            "in the form 'token: host:port' or, alternatively, a "
            "directory containing configuration files of this form")
    (opts, args) = parser.parse_args()

    # Sanity checks
    if len(args) < 2 and not (opts.target_cfg or opts.unix_target):
        parser.error("Too few arguments")
    if sys.argv.count('--'):
        opts.wrap_cmd = args[1:]
    else:
        opts.wrap_cmd = None
        if len(args) > 2:
            parser.error("Too many arguments")

    if not websocket.ssl and opts.ssl_target:
        parser.error("SSL target requested and Python SSL module not loaded.");

    if opts.ssl_only and not os.path.exists(opts.cert):
        parser.error("SSL only and %s not found" % opts.cert)

    # Parse host:port and convert ports to numbers
    if args[0].count(':') > 0:
        opts.listen_host, opts.listen_port = args[0].rsplit(':', 1)
        opts.listen_host = opts.listen_host.strip('[]')
    else:
        opts.listen_host, opts.listen_port = '', args[0]

    try:    opts.listen_port = int(opts.listen_port)
    except: parser.error("Error parsing listen port")

    if opts.wrap_cmd or opts.unix_target or opts.target_cfg:
        opts.target_host = None
        opts.target_port = None
    else:
        if args[1].count(':') > 0:
            opts.target_host, opts.target_port = args[1].rsplit(':', 1)
            opts.target_host = opts.target_host.strip('[]')
        else:
            parser.error("Error parsing target")
        try:    opts.target_port = int(opts.target_port)
        except: parser.error("Error parsing target port")

    # Create and start the WebSockets proxy
    server = WebSocketProxy(**opts.__dict__)
    server.start_server()

if __name__ == '__main__':
    websockify_init()
