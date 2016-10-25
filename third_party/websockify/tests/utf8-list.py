#!/usr/bin/python

'''
Display UTF-8 encoding for 0-255.'''

import sys, os, socket, ssl, time, traceback
from select import select
sys.path.insert(0,os.path.join(os.path.dirname(__file__), ".."))
from websockify.websocket import WebSocketServer

if __name__ == '__main__':
    print "val: hybi_base64 | hybi_binary"
    for c in range(0, 256):
        hybi_base64 = WebSocketServer.encode_hybi(chr(c), opcode=1,
                base64=True)
        hybi_binary = WebSocketServer.encode_hybi(chr(c), opcode=2,
                base64=False)
        print "%d: %s | %s" % (c, repr(hybi_base64), repr(hybi_binary))

