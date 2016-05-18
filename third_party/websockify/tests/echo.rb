#!/usr/bin/env ruby

# A WebSocket server that echos back whatever it receives from the client.
# Copyright 2011 Joel Martin
# Licensed under LGPL version 3 (see docs/LICENSE.LGPL-3)

require 'socket'
$: << "other"
$: << "../other"
require 'websocket'

class WebSocketEcho < WebSocketServer

  # Echo back whatever is received    
  def new_websocket_client(client)

    cqueue = []
    c_pend = 0
    rlist = [client]

    loop do
      wlist = []

      if cqueue.length > 0 or c_pend
        wlist << client
      end

      ins, outs, excepts = IO.select(rlist, wlist, nil, 1)
      if excepts.length > 0
        raise Exception, "Socket exception"
      end

      if outs.include?(client)
        # Send queued data to the client
        c_pend = send_frames(cqueue)
        cqueue = []
      end

      if ins.include?(client)
        # Receive client data, decode it, and send it back
        frames, closed = recv_frames
        cqueue += frames

        if closed
          raise EClose, closed
        end
      end

    end  # loop
  end
end

port = ARGV[0].to_i || 8080
puts "Starting server on port #{port}"
server_cert = nil
server_key = nil
if ARGV.length > 2
  server_cert = ARGV[1]
  server_key = ARGV[2]
end

server = WebSocketEcho.new('listen_port' => port, 'verbose' => true, 
  'server_cert' => server_cert, 'server_key' => server_key)
server.start
server.join

puts "Server has been terminated"

# vim: sw=2
