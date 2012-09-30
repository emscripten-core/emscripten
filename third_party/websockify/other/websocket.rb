
# Python WebSocket library with support for "wss://" encryption.
# Copyright 2011 Joel Martin
# Licensed under LGPL version 3 (see docs/LICENSE.LGPL-3)
# 
# Supports following protocol versions:
#     - http://tools.ietf.org/html/draft-hixie-thewebsocketprotocol-75
#     - http://tools.ietf.org/html/draft-hixie-thewebsocketprotocol-76
#     - http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-10

require 'gserver'
require 'stringio'
require 'digest/md5'
require 'digest/sha1'
require 'base64'

class EClose < Exception
end

class WebSocketServer < GServer
  @@Buffer_size = 65536

  #
  # WebSocket constants
  #
  @@Server_handshake_hixie = "HTTP/1.1 101 Web Socket Protocol Handshake\r
Upgrade: WebSocket\r
Connection: Upgrade\r
%sWebSocket-Origin: %s\r
%sWebSocket-Location: %s://%s%s\r
"

  @@Server_handshake_hybi = "HTTP/1.1 101 Switching Protocols\r
Upgrade: websocket\r
Connection: Upgrade\r
Sec-WebSocket-Accept: %s\r
"
  @@GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"


  def initialize(opts)
    vmsg "in WebSocketServer.initialize"
    port = opts['listen_port']
    host = opts['listen_host'] || GServer::DEFAULT_HOST

    super(port, host)
   
    @@client_id = 0  # Track client number total on class

    @verbose = opts['verbose']
    @opts = opts
  end
  
  def serve(io)
    @@client_id += 1

    # Initialize per thread state
    t = Thread.current
    t[:my_client_id] = @@client_id
    t[:send_parts] = []
    t[:recv_part] = nil
    t[:base64] = nil

    puts "in serve, client: #{t[:client].inspect}"

    begin
      t[:client] = do_handshake(io)
      new_client(t[:client])
    rescue EClose => e
      msg "Client closed: #{e.message}"
      return
    rescue Exception => e
      msg "Uncaught exception: #{e.message}"
      msg "Trace: #{e.backtrace}"
      return
    end

    msg "Client disconnected"
  end

  #
  # WebSocketServer logging/output functions
  #
  def traffic(token)
    if @verbose then print token; STDOUT.flush; end
  end

  def msg(msg)
    puts "% 3d: %s" % [Thread.current[:my_client_id], msg]
  end

  def vmsg(msg)
    if @verbose then msg(msg) end
  end

  #
  # WebSocketServer general support routines
  #
  def gen_md5(h)
    key1 = h['sec-websocket-key1']
    key2 = h['sec-websocket-key2']
    key3 = h['key3']
    spaces1 = key1.count(" ")
    spaces2 = key2.count(" ")
    num1 = key1.scan(/[0-9]/).join('').to_i / spaces1
    num2 = key2.scan(/[0-9]/).join('').to_i / spaces2

    return Digest::MD5.digest([num1, num2, key3].pack('NNa8'))
  end

  def unmask(buf, hlen, length)
    pstart = hlen + 4
    mask = buf[hlen...hlen+4]
    data = buf[pstart...pstart+length]
    #data = data.bytes.zip(mask.bytes.cycle(length)).map { |d,m| d^m }
    for i in (0...data.length) do
      data[i] ^= mask[i%4]
    end
    return data
  end

  def encode_hybi(buf, opcode, base64=false)
    if base64
      buf = Base64.encode64(buf).gsub(/\n/, '')
    end

    b1 = 0x80 | (opcode & 0x0f) # FIN + opcode
    payload_len = buf.length
    if payload_len <= 125
      header = [b1, payload_len].pack('CC')
    elsif payload_len > 125 && payload_len < 65536
      header = [b1, 126, payload_len].pack('CCn')
    elsif payload_len >= 65536
      header = [b1, 127, payload_len >> 32,
                         payload_len & 0xffffffff].pack('CCNN')
    end

    return [header + buf, header.length, 0]
  end

  def decode_hybi(buf, base64=false)
    f = {'fin'          => 0,
         'opcode'       => 0,
         'hlen'         => 2,
         'length'       => 0,
         'payload'      => nil,
         'left'         => 0,
         'close_code'   => nil,
         'close_reason' => nil}

    blen = buf.length
    f['left'] = blen

    if blen < f['hlen'] then return f end # incomplete frame

    b1, b2 = buf.unpack('CC')
    f['opcode'] = b1 & 0x0f
    f['fin']    = (b1 & 0x80) >> 7
    has_mask    = (b2 & 0x80) >> 7

    f['length'] = b2 & 0x7f

    if f['length'] == 126
      f['hlen'] = 4
      if blen < f['hlen'] then return f end # incomplete frame
      f['length'] = buf.unpack('xxn')[0]
    elsif f['length'] == 127
      f['hlen'] = 10
      if blen < f['hlen'] then return f end # incomplete frame
      top, bottom = buf.unpack('xxNN')
      f['length'] = (top << 32) & bottom
    end

    full_len = f['hlen'] + has_mask * 4 + f['length']

    if blen < full_len then return f end # incomplete frame

    # number of bytes that are part of the next frame(s)
    f['left'] = blen - full_len

    if has_mask > 0
      f['payload'] = unmask(buf, f['hlen'], f['length'])
    else
      f['payload'] = buf[f['hlen']...full_len]
    end

    if base64 and [1, 2].include?(f['opcode'])
      f['payload'] = Base64.decode64(f['payload'])
    end

    # close frame
    if f['opcode'] == 0x08
      if f['length'] >= 2
        f['close_code'] = f['payload'].unpack('n')
      end
      if f['length'] > 3
        f['close_reason'] = f['payload'][2...f['payload'].length]
      end
    end

    return f
  end

  def encode_hixie(buf)
    return ["\x00" + Base64.encode64(buf).gsub(/\n/, '') + "\xff", 1, 1]
  end

  def decode_hixie(buf)
    last = buf.index("\377")
    return {'payload' => Base64.decode64(buf[1...last]),
            'hlen' => 1,
            'length' => last - 1,
            'left' => buf.length - (last + 1)}
  end

  def send_frames(bufs)
    t = Thread.current
    if bufs.length > 0
      encbuf = ""
      bufs.each do |buf|
        if t[:version].start_with?("hybi")
          if t[:base64]
            encbuf, lenhead, lentail = encode_hybi(
              buf, opcode=1, base64=true)
          else
            encbuf, lenhead, lentail = encode_hybi(
              buf, opcode=2, base64=false)
          end
        else
          encbuf, lenhead, lentail = encode_hixie(buf)
        end
      
        t[:send_parts] << encbuf
      end

    end

    while t[:send_parts].length > 0
      buf = t[:send_parts].shift
      sent = t[:client].send(buf, 0)

      if sent == buf.length
        traffic "<"
      else
        traffic "<."
        t[:send_parts].unshift(buf[sent...buf.length])
      end
    end

    return t[:send_parts].length
  end

  # Receive and decode Websocket frames
  # Returns: [bufs_list, closed_string]
  def recv_frames()
    t = Thread.current
    closed = false
    bufs = []

    buf = t[:client].recv(@@Buffer_size)

    if buf.length == 0
      return bufs, "Client closed abrubtly"
    end

    if t[:recv_part]
      buf = t[:recv_part] + buf
      t[:recv_part] = nil
    end

    while buf.length > 0
      if t[:version].start_with?("hybi")
        frame = decode_hybi(buf, base64=t[:base64])

        if frame['payload'] == nil
          traffic "}."
          if frame['left'] > 0
            t[:recv_part] = buf[-frame['left']...buf.length]
          end
          break
        else
          if frame['opcode'] == 0x8
            closed = "Client closed, reason: %s - %s" % [
              frame['close_code'], frame['close_reason']]
            break
          end
        end
      else
        if buf[0...2] == "\xff\x00":
          closed = "Client sent orderly close frame"
          break
        elsif buf[0...2] == "\x00\xff":
          buf = buf[2...buf.length]
          continue # No-op frame
        elsif buf.count("\xff") == 0
          # Partial frame
          traffic "}."
          t[:recv_part] = buf
          break
        end

        frame = decode_hixie(buf)
      end

      #msg "Receive frame: #{frame.inspect}"

      traffic "}"

      bufs << frame['payload']

      if frame['left'] > 0:
        buf = buf[-frame['left']...buf.length]
      else
        buf = ''
      end
    end

    return bufs, closed
  end


  def send_close(code=nil, reason='')
    t = Thread.current
    if t[:version].start_with?("hybi")
      msg = ''
      if code
        msg = [reason.length, code].pack("na8")
      end

      buf, lenh, lent = encode_hybi(msg, opcode=0x08, base64=false)
      t[:client].send(buf, 0)
    elsif t[:version] == "hixie-76"
      buf = "\xff\x00"
      t[:client].send(buf, 0)
    end
  end

  def do_handshake(sock)
  
    t = Thread.current
    stype = ""

    if !IO.select([sock], nil, nil, 3)
      raise EClose, "ignoring socket not ready"
    end

    handshake = sock.recv(1024, Socket::MSG_PEEK)
    #msg "Handshake [#{handshake.inspect}]"

    if handshake == ""
      raise(EClose, "ignoring empty handshake")
    else
      stype = "Plain non-SSL (ws://)"
      scheme = "ws"
      retsock = sock
      sock.recv(1024)
    end

    h = t[:headers] = {}
    hlines = handshake.split("\r\n")
    req_split = hlines.shift.match(/^(\w+) (\/[^\s]*) HTTP\/1\.1$/) 
    t[:path] = req_split[2].strip
    hlines.each do |hline|
      break if hline == ""
      hsplit = hline.match(/^([^:]+):\s*(.+)$/)
      h[hsplit[1].strip.downcase] = hsplit[2]
    end
    #puts "Headers: #{h.inspect}"

    unless h.has_key?('upgrade') &&
       h['upgrade'].downcase == 'websocket'
      raise EClose, "Non-WebSocket connection"
    end

    protocols = h.fetch("sec-websocket-protocol", h["websocket-protocol"])
    ver = h.fetch('sec-websocket-version', nil)

    if ver
      # HyBi/IETF vesrion of the protocol

      # HyBi 07 reports version 7
      # HyBi 08 - 12 report version 8
      # HyBi 13 and up report version 13
      if ['7', '8', '13'].include?(ver)
        t[:version] = "hybi-%02d" % [ver.to_i]
      else
        raise EClose, "Unsupported protocol version %s" % [ver]
      end

      # choose binary if client supports it
      if protocols.include?('binary')
        t[:base64] = false
      elsif protocols.include?('base64')
        t[:base64] = true
      else
        raise EClose, "Client must support 'binary' or 'base64' sub-protocol"
      end

      key = h['sec-websocket-key']

      # Generate the hash value for the accpet header
      accept = Base64.encode64(
        Digest::SHA1.digest(key + @@GUID)).gsub(/\n/, '')

      response = @@Server_handshake_hybi % [accept]

      if t[:base64]
        response += "Sec-WebSocket-Protocol: base64\r\n"
      else
        response += "Sec-WebSocket-Protocol: binary\r\n"
      end
      response += "\r\n"

    else
      # Hixie vesrion of the protocol (75 or 76)
      body = handshake.match(/\r\n\r\n(........)/)
      if body
        h['key3'] = body[1]
        trailer = gen_md5(h)
        pre = "Sec-"
        t[:version] = "hixie-76"
      else
        trailer = ""
        pre = ""
        t[:version] = "hixie-75"
      end
      
      # base64 required for Hixie since payload is only UTF-8
      t[:base64] = true

      response = @@Server_handshake_hixie % [pre, h['origin'], pre,
        "ws", h['host'], t[:path]]

      if protocols && protocols.include?('base64')
        response += "%sWebSocket-Protocol: base64\r\n" % [pre]
      else
        msg "Warning: client does not report 'base64' protocol support"
      end

      response += "\r\n" + trailer
    end

    msg "%s WebSocket connection" % [stype]
    msg "Version %s, base64: '%s'" % [t[:version], t[:base64]]
    if t[:path] then msg "Path: '%s'" % [t[:path]] end

    #puts "sending reponse #{response.inspect}"
    retsock.send(response, 0)

    # Return the WebSocket socket which may be SSL wrapped
    return retsock
  end

end

# vim: sw=2
