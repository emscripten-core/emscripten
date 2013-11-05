#!/usr/bin/python

'''
Display UTF-8 encoding for 0-255.'''

import sys, os, socket, ssl, time, traceback
from select import select

sys.path.insert(0,os.path.dirname(__file__) + "/../")
from websocket import WebSocketServer

if __name__ == '__main__':
    print "val: hixie | hybi_base64 | hybi_binary"
    for c in range(0, 256):
        hixie = WebSocketServer.encode_hixie(chr(c))
        hybi_base64 = WebSocketServer.encode_hybi(chr(c), opcode=1,
                base64=True)
        hybi_binary = WebSocketServer.encode_hybi(chr(c), opcode=2,
                base64=False)
        print "%d: %s | %s | %s" % (c, repr(hixie), repr(hybi_base64),
                repr(hybi_binary))

