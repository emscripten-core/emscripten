#!/usr/bin/env python

'''
A WebSocket server that echos back whatever it receives from the client.
Copyright 2010 Joel Martin
Licensed under LGPL version 3 (see docs/LICENSE.LGPL-3)

You can make a cert/key with openssl using:
openssl req -new -x509 -days 365 -nodes -out self.pem -keyout self.pem
as taken from http://docs.python.org/dev/library/ssl.html#certificates
'''

import os, sys, select, optparse
sys.path.insert(0,os.path.dirname(__file__) + "/../")
from websocket import WebSocketServer

class WebSocketEcho(WebSocketServer):
    """
    WebSockets server that echos back whatever is received from the
    client.  """
    buffer_size = 8096

    def new_client(self):
        """
        Echo back whatever is received.
        """

        cqueue = []
        c_pend = 0
        cpartial = ""
        rlist = [self.client]

        while True:
            wlist = []

            if cqueue or c_pend: wlist.append(self.client)
            ins, outs, excepts = select.select(rlist, wlist, [], 1)
            if excepts: raise Exception("Socket exception")

            if self.client in outs:
                # Send queued target data to the client
                c_pend = self.send_frames(cqueue)
                cqueue = []

            if self.client in ins:
                # Receive client data, decode it, and send it back
                frames, closed = self.recv_frames()
                cqueue.extend(frames)

                if closed:
                    self.send_close()
                    raise self.EClose(closed)

if __name__ == '__main__':
    parser = optparse.OptionParser(usage="%prog [options] listen_port")
    parser.add_option("--verbose", "-v", action="store_true",
            help="verbose messages and per frame traffic")
    parser.add_option("--cert", default="self.pem",
            help="SSL certificate file")
    parser.add_option("--key", default=None,
            help="SSL key file (if separate from cert)")
    parser.add_option("--ssl-only", action="store_true",
            help="disallow non-encrypted connections")
    (opts, args) = parser.parse_args()

    try:
        if len(args) != 1: raise
        opts.listen_port = int(args[0])
    except:
        parser.error("Invalid arguments")

    opts.web = "."
    server = WebSocketEcho(**opts.__dict__)
    server.start_server()

