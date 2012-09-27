#!/usr/bin/env ruby

# A WebSocket to TCP socket proxy
# Copyright 2011 Joel Martin
# Licensed under LGPL version 3 (see docs/LICENSE.LGPL-3)

require 'socket'
$: << "other"
$: << "../other"
require 'websocket'
require 'optparse'

# Proxy traffic to and from a WebSockets client to a normal TCP
# socket server target. All traffic to/from the client is base64
# encoded/decoded to allow binary data to be sent/received to/from
# the target.
class WebSocketProxy < WebSocketServer

  @@Traffic_legend = "
Traffic Legend:
    }  - Client receive
    }. - Client receive partial
    {  - Target receive

    >  - Target send
    >. - Target send partial
    <  - Client send
    <. - Client send partial
"


  def initialize(opts)
    vmsg "in WebSocketProxy.initialize"

    super(opts)
    
    @target_host = opts["target_host"]
    @target_port = opts["target_port"]
  end

  # Echo back whatever is received    
  def new_client(client)

    msg "connecting to: %s:%s" % [@target_host, @target_port]
    tsock = TCPSocket.open(@target_host, @target_port)

    if @verbose then puts @@Traffic_legend end

    begin
      do_proxy(client, tsock)
    rescue
      tsock.shutdown(Socket::SHUT_RDWR)
      tsock.close
      raise
    end
  end

  # Proxy client WebSocket to normal target socket.
  def do_proxy(client, target)
    cqueue = []
    c_pend = 0
    tqueue = []
    rlist = [client, target]

    loop do
      wlist = []

      if tqueue.length > 0
        wlist << target
      end
      if cqueue.length > 0 || c_pend > 0
        wlist << client
      end

      ins, outs, excepts = IO.select(rlist, wlist, nil, 0.001)
      if excepts && excepts.length > 0
        raise Exception, "Socket exception"
      end

      # Send queued client data to the target
      if outs && outs.include?(target)
        dat = tqueue.shift
        sent = target.send(dat, 0)
        if sent == dat.length
          traffic ">"
        else
          tqueue.unshift(dat[sent...dat.length])
          traffic ".>"
        end
      end

      # Receive target data and queue for the client
      if ins && ins.include?(target)
        buf = target.recv(@@Buffer_size)
        if buf.length == 0:
          raise EClose, "Target closed"
        end

        cqueue << buf
        traffic "{"
      end

      # Encode and send queued data to the client
      if outs && outs.include?(client)
        c_pend = send_frames(cqueue)
        cqueue = []
      end

      # Receive client data, decode it, and send it back
      if ins && ins.include?(client)
        frames, closed = recv_frames
        tqueue += frames

        if closed
          send_close
          raise EClose, closed
        end
      end

    end  # loop
  end
end

# Parse parameters
opts = {}
parser = OptionParser.new do |o|
  o.on('--verbose', '-v') { |b| opts['verbose'] = b }
  o.parse!
end

if ARGV.length < 2:
  puts "Too few arguments"
  exit 2
end

# Parse host:port and convert ports to numbers
if ARGV[0].count(":") > 0
  opts['listen_host'], _, opts['listen_port'] = ARGV[0].rpartition(':')
else
  opts['listen_host'], opts['listen_port'] = nil, ARGV[0]
end

begin
  opts['listen_port'] = opts['listen_port'].to_i
rescue
  puts "Error parsing listen port"
  exit 2
end

if ARGV[1].count(":") > 0
  opts['target_host'], _, opts['target_port'] = ARGV[1].rpartition(':')
else
  puts "Error parsing target"
  exit 2
end

begin
  opts['target_port'] = opts['target_port'].to_i
rescue
  puts "Error parsing target port"
  exit 2
end

puts "Starting server on #{opts['listen_host']}:#{opts['listen_port']}"
server = WebSocketProxy.new(opts)
server.start(100)
server.join

puts "Server has been terminated"

# vim: sw=2
