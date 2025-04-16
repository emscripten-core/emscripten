/*
The MIT License

Copyright (c) 2012, Mozilla Foundation
Copyright (c) 2012, Alan Kligman
All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

(function() {

  /* Notes
   *
   * - Continue using prefixed names for now.
   *
   */

  var io = SocketIO;
  var webrtcSupported = true;

  var RTCPeerConnection;
  if(window.mozRTCPeerConnection)
    RTCPeerConnection = window.mozRTCPeerConnection;
  else if(window.webkitRTCPeerConnection)
    RTCPeerConnection = window.webkitRTCPeerConnection;
  else if(window.RTCPeerConnection)
    RTCPeerConnection = window.RTCPeerConnection
  else
    webrtcSupported = false;

  var RTCSessionDescription;
  if(window.mozRTCSessionDescription)
    RTCSessionDescription = window.mozRTCSessionDescription;
  else if(window.webkitRTCSessionDescription)
    RTCSessionDescription = window.webkitRTCSessionDescription;
  else if(window.RTCSessionDescription)
    RTCSessionDescription = window.RTCSessionDescription
  else
    webrtcSupported = false;

  var RTCIceCandidate;
  if(window.mozRTCIceCandidate)
    RTCIceCandidate = window.mozRTCIceCandidate;
  else if(window.webkitRTCIceCandidate)
    RTCIceCandidate = window.webkitRTCIceCandidate;
  else if(window.RTCIceCandidate)
    RTCIceCandidate = window.RTCIceCandidate;
  else
    webrtcSupported = false;

  var getUserMedia;
  if(!navigator.getUserMedia) {
    if(navigator.mozGetUserMedia)
      getUserMedia = navigator.mozGetUserMedia.bind(navigator);
    else if(navigator.webkitGetUserMedia)
      getUserMedia = navigator.webkitGetUserMedia.bind(navigator);
    else
      webrtcSupported = false;
  } else {
    getUserMedia = navigator.getUserMedia.bind(navigator);
  }

  // FIXME: browser detection is gross, but I don't see another way to do this
  var RTCConnectProtocol;
  if(window.mozRTCPeerConnection) {
    RTCConnectProtocol = mozRTCConnectProtocol;
  } else if(window.webkitRTCPeerConnection) {
    RTCConnectProtocol = webkitRTCConnectProtocol;
  } else {
    webrtcSupported = false;
  }

  function callback(object, method, args) {
    if(!Array.isArray(args))
      args = [args];
    if(method in object && 'function' === typeof object[method]) {
      object[method].apply(object, args);
    }
  };

  function fail(object, method, error) {
    if (!(error instanceof Error))
      error = new Error(error);
    callback(object, method, [error]);
  };

  function defer(queue, object, method, args) {
    if(queue) {
      queue.push([object, method, args]);
      return true;
    } else {
      return false;
    }
  };

  function processDeferredQueue(queue) {
    while(queue.length) {
      var deferred = queue.shift();
      callback(deferred[0], deferred[1], deferred[2]);
    }
  };

  var ONE_SECOND = 1000; // milliseconds
  var DEFAULT_CONNECTION_TIMEOUT = 10 * ONE_SECOND;
  var DEFAULT_PING_TIMEOUT = 1 * ONE_SECOND;
  var RELIABLE_CHANNEL_OPTIONS = {
    reliable: false
  };
  var UNRELIABLE_CHANNEL_OPTIONS = {
    outOfOrderAllowed: true,
    maxRetransmitNum: 0,
    reliable: false
  };

  function PendingConnectionAbortError(message) {
    this.name = "PendingConnectionAbortError";
    this.message = (message || "");
  };
  PendingConnectionAbortError.prototype = Error.prototype;

  function ConnectionFailedError(message) {
    this.name = "ConnectionFailedError";
    this.message = (message || "");
  };
  ConnectionFailedError.prototype = Error.prototype;

  var E = {
    PendingConnectionAbortError: PendingConnectionAbortError,
    ConnectionFailedError: ConnectionFailedError
  };

  function WebSocketBroker(brokerUrl) {
    this.brokerUrl = brokerUrl;
    this.state = WebSocketBroker.OFFLINE;

    this.onstatechange = null;
    this.onreceive = null;
    this.onerror = null;

    this.socket = null;
    this.route = null;
  };

  // States
  WebSocketBroker.OFFLINE     = 0x00;
  WebSocketBroker.CONNECTING  = 0x01;
  WebSocketBroker.CONNECTED   = 0x02;
  // Flags
  WebSocketBroker.ROUTED      = 0x10;
  WebSocketBroker.LISTENING   = 0x20;

  WebSocketBroker.prototype.setState = function setState(state, clearFlags) {
    var clear = clearFlags ? 0x00 : 0xF0;
    this.state &= clear >>> 0;
    this.state |= state >>> 0;
    callback(this, 'onstatechange', [this.state, (state | (clear & 0x0)) >>> 0]);
  };
  WebSocketBroker.prototype.setFlag = function setFlag(flag) {
    this.state = (this.state | flag) >>> 0;
    callback(this, 'onstatechange', [this.state, flag])
  };
  WebSocketBroker.prototype.clearFlag = function clearFlag(flag) {
    flag = (~flag) >>> 0;
    this.state = (this.state & flag) >>> 0;
    callback(this, 'onstatechange', [this.state, flag])
  };
  WebSocketBroker.prototype.checkState = function checkState(mask) {
    return !!(this.state & mask);
  };
  WebSocketBroker.prototype.connect = function connect() {
    var that = this;
    var socket = io.connect(this.brokerUrl + '/peer', {
      'sync disconnect on unload': true // partially fixes 'interrupted while page loading' warning
    });

    socket.on('connecting', function onconnecting() {
      that.setState(WebSocketBroker.CONNECTING, true);
    });

    socket.on('connect', function onconnect() {
      that.setState(WebSocketBroker.CONNECTED, true);
    });

    socket.on('connect_failed', function onconnect_failed() {
      that.setState(WebSocketBroker.OFFLINE, true);
    });

    socket.on('route', function onroute(route) {
      that.route = route;
      that.setFlag(WebSocketBroker.ROUTED);
    });

    socket.on('disconnect', function ondisconnect() {
      that.setState(WebSocketBroker.OFFLINE, true);
    });

    socket.on('error', function onerror(error) {
      fail(that, 'onerror', error);
    });

    socket.on('receive', function onreceive(message) {
      var from = message['from'];
      var data = message['data'];
      callback(that, 'onreceive', [from, data]);
    });

    this.socket = socket;
  };
  WebSocketBroker.prototype.disconnect = function disconnect() {
    if(this.checkState(WebSocketBroker.CONNECTED)) {
      this.socket.disconnect();
      this.setState(WebSocketBroker.OFFLINE, true);
      return true;
    } else {
      return false;
    }
  };
  WebSocketBroker.prototype.listen = function listen(options) {
    var that = this;
    if(this.checkState(WebSocketBroker.CONNECTED)) {
      this.socket.emit('listen', options, function onresponse(response) {
        if(response && response['error']) {
          var error = new Error(response['error']);
          fail(that, 'onerror', error);
        } else {
          that.setFlag(WebSocketBroker.LISTENING);
        }
      });
    }
  };
  WebSocketBroker.prototype.ignore = function ignore() {
    var that = this;
    if(this.checkState(WebSocketBroker.CONNECTED)) {
      this.socket.emit('ignore', null, function onresponse(response) {
        if(response && response['error']) {
          var error = new Error(response['error']);
          fail(that, 'onerror', error)
        } else {
          that.clearFlag(WebSocketBroker.LISTENING);
        }
      });
    }
  };
  WebSocketBroker.prototype.send = function send(to, message) {
    var that = this;
    if(this.checkState(WebSocketBroker.CONNECTED)) {
      this.socket.emit('send', {'to': to, 'data': message}, function onresponse(response) {
        if(response && response['error']) {
          var error = new Error(response['error']);
          fail(that, 'onerror', error)
        }
      });
    };
  };

  var dataChannels = {
    'reliable': 'RELIABLE',
    'unreliable': 'UNRELIABLE',
    '@control': 'RELIABLE'
  };
  var nextDataConnectionPort = 1;
  function CommonRTCConnectProtocol() {
    // FIXME: these timeouts should be configurable
    this.connectionTimeout = 10 * ONE_SECOND;
    this.pingTimeout = 1 * ONE_SECOND;
  };
  CommonRTCConnectProtocol.prototype.process = function process(message) {
    var that = this;

    var type = message['type'];
    switch (type) {
      case 'ice':
        var candidate = JSON.parse(message['candidate']);
        if(candidate)
          this.handleIce(candidate);
        break;

      case 'offer':
        that.ports.remote = message['port'];
        var offer = {
          'type': 'offer',
          'sdp': message['description']
        };
        this.handleOffer(offer);
        break;

      case 'answer':
        that.ports.remote = message['port'];
        var answer = {
          'type': 'answer',
          'sdp': message['description']
        };
        this.handleAnswer(answer);
        break;

      case 'abort':
        this.handleAbort();
        break;

      default:
        fail(this, 'onerror', 'unknown message');
    }
  };
  CommonRTCConnectProtocol.prototype.handleAbort = function handleAbort() {
    fail(this, 'onerror', new Error(E.RTCConnectProtocolAbort));
  };
  CommonRTCConnectProtocol.prototype.initialize = function initialize(cb) {
    var that = this;

    if(this.peerConnection)
      return cb();

    // FIXME: peer connection servers should be configurable
    this.peerConnection = new RTCPeerConnection(this.connectionServers, this.connectionOptions);
    this.peerConnection.onicecandidate = function(event) {
      var message = {
        'type': 'ice',
        'candidate': JSON.stringify(event.candidate)
      };
      callback(that, 'onmessage', message);
    };
    this.peerConnection.onaddstream = function(event) {
      that.streams['remote'] = event.stream;
    };
    this.peerConnection.onstatechange = function(event) {
      console.log(event.target.readyState);
    };

    function createStream(useFake) {
      useFake = (!useVideo && !useAudio) ? true : useFake;
      var useVideo = !!that.options['video'];
      var useAudio = !!that.options['audio'];
      var mediaOptions = {
        video: useVideo,
        audio: (!useVideo && !useAudio) ? true : useAudio,
        fake: useFake
      };
      getUserMedia(mediaOptions,
        function(stream) {
          that.peerConnection.addStream(stream);
          that.streams['local'] = stream;
          cb();
        },
        function(error) {
          console.error('!', error);
          if(!useFake)
            createStream(true);
          else
            fail(that, 'onerror', error);
        }
      );
    }

    createStream();
  };
  CommonRTCConnectProtocol.prototype.handleIce = function handleIce(candidate) {
    var that = this;

    function setIce() {
      if(!that.peerConnection.remoteDescription) {
        return
      }
      that.peerConnection.addIceCandidate(new RTCIceCandidate(candidate),
        function() {},
        function(error) {
          fail(that, 'onerror', error);
        }
      );
    };

    this.initialize(setIce);
  };
  CommonRTCConnectProtocol.prototype.initiate = function initiate() {
    var that = this;
    this.initiator = true;

    function createDataChannels() {
      var labels = Object.keys(dataChannels);
      labels.forEach(function(label) {
        var channelOptions = that.channelOptions[dataChannels[label]];
        var channel = that._pending[label] = that.peerConnection.createDataChannel(label, channelOptions);
        channel.binaryType = that.options['binaryType'];
        channel.onopen = function() {
          that.channels[label] = channel;
          delete that._pending[label];
          if(Object.keys(that.channels).length === labels.length) {
            that.complete = true;
            callback(that, 'oncomplete', []);
          }
        };
        channel.onerror = function(error) {
          console.error(error);
          fail(that, 'onerror', error);
        };
      });
      createOffer();
    };

    function createOffer() {
      that.peerConnection.createOffer(setLocal,
        function(error) {
          fail(that, 'onerror', error);
        }
      );
    };

    function setLocal(description) {
      that.peerConnection.setLocalDescription(new RTCSessionDescription(description), complete,
        function(error) {
          fail(that, 'onerror', error);
        }
      );

      function complete() {
        var message = {
          'type': 'offer',
          'description': description['sdp'],
          'port': that.ports.local
        };
        callback(that, 'onmessage', message);
      };
    };

    this.initialize(createDataChannels);
  };
  CommonRTCConnectProtocol.prototype.handleOffer = function handleOffer(offer) {
    var that = this;

    function handleDataChannels() {
      var labels = Object.keys(dataChannels);
      that.peerConnection.ondatachannel = function(event) {
        var channel = event.channel;
        var label = channel.label;
        that._pending[label] = channel;
        channel.binaryType = that.options['binaryType'];
        channel.onopen = function() {
          that.channels[label] = channel;
          delete that._pending[label];
          if(Object.keys(that.channels).length === labels.length) {
            that.complete = true;
            callback(that, 'oncomplete', []);
          }
        };
        channel.onerror = function(error) {
          console.error(error);
          fail(that, 'onerror', error);
        };
      };
      setRemote();
    };

    function setRemote() {
      that.peerConnection.setRemoteDescription(new RTCSessionDescription(offer), createAnswer,
        function(error) {
          fail(that, 'onerror', error);
        }
      );
    };

    function createAnswer() {
      that.peerConnection.createAnswer(setLocal,
        function(error) {
          fail(that, 'onerror', error);
        }
      );
    };

    function setLocal(description) {
      that.peerConnection.setLocalDescription(new RTCSessionDescription(description), complete,
        function(error) {
          fail(that, 'onerror', error);
        }
      );

      function complete() {
        var message = {
          'type': 'answer',
          'description': description['sdp'],
          'port': that.ports.local
        };
        callback(that, 'onmessage', message);
      };
    };

    this.initialize(handleDataChannels);
  };
  CommonRTCConnectProtocol.prototype.handleAnswer = function handleAnswer(answer) {
    var that = this;

    function setRemote() {
      that.peerConnection.setRemoteDescription(new RTCSessionDescription(answer), complete,
        function(error) {
          fail(that, 'onerror', error);
        }
      );
    };

    function complete() {
    };

    this.initialize(setRemote);
  };

  function mozRTCConnectProtocol(options) {
    this.options = options;
    this.onmessage = null;
    this.oncomplete = null;
    this.onerror = null;

    this.complete = false;
    this.ports = {
      local: nextDataConnectionPort ++,
      remote: null
    };
    this.streams = {
      local: null,
      remote: null
    };
    this.initiator = false;

    this.peerConnection = null;
    this.channels = {};
    this._pending = {};
    this.connectionServers = null;
    this.connectionOptions = null;
    this.channelOptions = {
      RELIABLE: {
        // defaults
      },
      UNRELIABLE: {
        outOfOrderAllowed: true,
        maxRetransmitNum: 0
      }
    };
  };
  mozRTCConnectProtocol.prototype = new CommonRTCConnectProtocol();
  mozRTCConnectProtocol.prototype.constructor = mozRTCConnectProtocol;

  function webkitRTCConnectProtocol(options) {
    this.options = options;
    this.onmessage = null;
    this.oncomplete = null;
    this.onerror = null;

    this.complete = false;
    this.ports = {
      local: nextDataConnectionPort ++,
      remote: null
    };
    this.streams = {
      local: null,
      remote: null
    };
    this.initiator = false;

    this.peerConnection = null;
    this.channels = {};
    this._pending = {};
    this.connectionServers = {iceServers:[{url:'stun:23.21.150.121'}]};
    this.connectionOptions = {
      'optional': [{ 'RtpDataChannels': true }]
    };
    this.channelOptions = {
      RELIABLE: {
        // FIXME: reliable channels do not work in chrome yet
        reliable: false
      },
      UNRELIABLE: {
        reliable: false
      }
    };
  };
  webkitRTCConnectProtocol.prototype = new CommonRTCConnectProtocol();
  webkitRTCConnectProtocol.prototype.constructor = webkitRTCConnectProtocol;

  // FIXME: this could use a cleanup
  var nextConnectionId = 1;
  function Connection(options, peerConnection, streams, channels) {
    var that = this;
    this.id = nextConnectionId ++;
    this.streams = streams;
    this.connected = false;
    this.messageFlag = false;

    this.onmessage = null;
    this.ondisconnect = null;
    this.onerror = null;

    this.peerConnection = peerConnection;

    // DataChannels
    this.channels = channels;

    this.connectionTimer = null;
    this.pingTimer = null;

    function handleConnectionTimerExpired() {
      if(!that.connected)
        return
      this.connectionTimer = null;
      if(false === that.messageFlag) {
        that.channels['@control'].send('ping');
        this.pingTimer = window.setTimeout(handlePingTimerExpired, options['pingTimeout']);
      } else {
        that.messageFlag = false;
        this.connectionTimer = window.setTimeout(handleConnectionTimerExpired, options['connectionTimeout']);
      }
    };
    function handlePingTimerExpired() {
      if(!that.connected)
        return
      this.pingTimer = null;
      if(false === that.messageFlag) {
        that.connected = false;
        that.close();
      } else {
        that.messageFlag = false;
        this.connectionTimer = window.setTimeout(handleConnectionTimerExpired, options['connectionTimeout']);
      }
    };

    Object.keys(this.channels).forEach(function(label) {
      var channel = that.channels[label];
      if(label.match('^@')) // check for internal channels
        return;

      channel.onmessage = function onmessage(message) {
        that.messageFlag = true;
        callback(that, 'onmessage', [label, message]);
      };
    });
    this.channels['@control'].onmessage = function onmessage(message) {
      that.messageFlag = true;
      if(that.connected) {
        var data = message.data;
        if('ping' === data) {
          that.channels['@control'].send('pong');
        } else if('pong' === data) {
          // ok
        } else if('quit' === data) {
          that.close();
        }
      }
    };

    this.connected = true;
    this.connectionTimer = window.setTimeout(handleConnectionTimerExpired, options['connectionTimeout']);
  };
  Connection.prototype.close = function close() {
    console.log('close connection');
    if(this.connected) {
      this.channels['@control'].send('quit');
    }
    this.connected = false;
    this.peerConnection.close();
    if(this.connectionTimer) {
      window.clearInterval(this.connectionTimer);
      this.connectionTimer = null;
    }
    if(this.pingTimer) {
      window.clearInterval(this.pingTimer);
      this.pingTimer = null;
    }
    this.peerConnection = null;
    callback(this, 'ondisconnect', []);
  };
  Connection.prototype.send = function send(label, message) {
    this.channels[label].send(message);
  };

  function PendingConnection(route, incoming) {
    this.route = route;
    this.incoming = incoming;
    this.proceed = true;
  };
  PendingConnection.prototype.accept = function accept() {
    this.proceed = true;
  };
  PendingConnection.prototype.reject = function reject() {
    this.proceed = false;
  };

  function Peer(brokerUrl, options) {
    if(!webrtcSupported)
      throw new Error("WebRTC not supported");

    var that = this;
    this.brokerUrl = brokerUrl;
    this.options = options = options || {};
    options['binaryType'] = options['binaryType'] || 'arraybuffer';
    options['connectionTimeout'] = options['connectionTimeout'] || 10 * ONE_SECOND;
    options['pingTimeout'] = options['pingTimeout'] || 1 * ONE_SECOND;

    this.onconnection = null;
    this.onpending = null;
    this.onroute = null;
    this.onerror = null;

    this.broker = new WebSocketBroker(brokerUrl);
    this.pending = {};

    this.queues = {
      connected: [],
      listening: []
    };

    this.broker.onstatechange = function onstatechange(state, mask) {
      if(that.queues.connected.length && that.broker.checkState(WebSocketBroker.ROUTED)) {
        processDeferredQueue(that.queues.connected);
        if(that.queues.listening.length && that.broker.checkState(WebSocketBroker.LISTENING)) {
          processDeferredQueue(that.queues.listening);
        }
      }
      if(mask & WebSocketBroker.ROUTED) {
        callback(that, 'onroute', that.broker.route);
      }
    };

    this.broker.onreceive = function onreceive(from, message) {
      var handshake;
      if(!that.pending.hasOwnProperty(from)) {
        if(!that.broker.checkState(WebSocketBroker.LISTENING)) {
          return;
        }

        var pendingConnection = new PendingConnection(from, /*incoming*/ true);
        callback(that, 'onpending', [pendingConnection]);
        if(!pendingConnection['proceed'])
          return;

        var handshake = that.pending[from] = new RTCConnectProtocol(that.options);
        handshake.oncomplete = function() {
          var connection = new Connection(that.options, handshake.peerConnection, handshake.streams, handshake.channels);
          connection['route'] = from;
          delete that.pending[from];
          callback(that, 'onconnection', [connection]);
        };
        handshake.onmessage = function(message) {
          that.broker.send(from, message);
        };
        handshake.onerror = function(error) {
          delete that.pending[from];
          callback(that, 'onerror', [error]);
        };
      } else {
        handshake = that.pending[from];
      }
      handshake.process(message);
    };

    this.broker.connect();
  };
  Peer.prototype.listen = function listen(options) {
    if(!this.broker.checkState(WebSocketBroker.ROUTED))
      return defer(this.queues.connected, this, 'listen', [options]);

    options = options || {};
    options['url'] = options['url'] || window.location.toString();
    options['listed'] = (undefined !== options['listed']) ? options['listed'] : true;
    options['metadata'] = options['metadata'] || {};

    this.broker.listen(options);
  };
  Peer.prototype.ignore = function ignore() {
    throw new Error('not implemented');
  };
  Peer.prototype.connect = function connect(route) {
    if(!this.broker.checkState(WebSocketBroker.ROUTED))
      return defer(this.queues.connected, this, 'connect', [route]);

    var that = this;

    if(this.pending.hasOwnProperty(route))
      throw new Error('already connecting to this host'); // FIXME: we can handle this better

    var pendingConnection = new PendingConnection(route, /*incoming*/ false);
    callback(that, 'onpending', [pendingConnection]);
    if(!pendingConnection['proceed'])
      return;

    var handshake = this.pending[route] = new RTCConnectProtocol(this.options);
    handshake.oncomplete = function() {
      var connection = new Connection(this.options, handshake.peerConnection, handshake.streams, handshake.channels);
      connection['route'] = route;
      delete that.pending[route];
      callback(that, 'onconnection', [connection]);
    };
    handshake.onmessage = function(message) {
      that.broker.send(route, message);
    };
    handshake.onerror = function(error) {
      delete that.pending[route];
      fail(that, 'onerror', error);
    };

    handshake.initiate();
  };
  Peer.prototype.close = function close() {
    this.broker.disconnect();
  };
  Peer.E = E;

  return Peer;

})();