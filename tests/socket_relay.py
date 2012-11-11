'''
Listens on 2 ports and relays between them.

Listens to ports A and B. When someone connects to port A, and then
sends some data to port A, that data is sent to someone who
connected to socket B. And so forth.

This is different than say socat which will listen to one port
and then make a connection to another port, and do bidirectional
communication. We need to actually listen on both ports.
'''

import os, sys, socket, time, threading, signal
from subprocess import Popen, PIPE, STDOUT

ports = [int(sys.argv[1]), int(sys.argv[2])]

class Listener(threading.Thread):
  def run(self):
    self.conn = None
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    global ports
    port = ports[0]
    ports = ports[1:]
    print 'listener binding to ', port
    s.bind(('127.0.0.1', port))
    s.listen(1)
    print 'listener', port, 'waiting for connection'
    conn, addr = s.accept()
    self.conn = conn
    while 1:
      time.sleep(0.5)
      print 'listener', port, 'waiting for data'
      data = conn.recv(20*1024)
      if not data:
        continue
      while not self.other.conn:
        print 'listener', port, 'waiting for other connection in order to send data'
        time.sleep(1)
      print 'listener', port, 'sending data', len(data)
      self.other.conn.send(data)

in_listener = Listener()
in_listener.daemon = True
in_listener.start()

out_listener = Listener()
out_listener.daemon = True
out_listener.start()

in_listener.other = out_listener
out_listener.other = in_listener

while 1:
  time.sleep(1)

