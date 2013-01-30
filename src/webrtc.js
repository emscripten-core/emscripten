(function() {

  function callback(thing, method, args) {
    if(method in thing && 'function' === typeof thing[method]) {
      thing[method].apply(thing, args);
    }
  }

  function Initiator(brokerUrl, sid)
  {
    var initiator = this;

    initiator.channel = undefined;
    initiator.cid = undefined;
    initiator.key = undefined;
    initiator.sid = sid;

    initiator.onpending = null;
    initiator.oncomplete = null;
    initiator.onerror = null;

    function fail(error) {
      if(initiator.onerror && 'function' === typeof initiator.onerror) {
        if (!(error instanceof Error))
          error = new Error(error);
        initiator.onerror.call(null, error);
      }
    };

    function sendMessage(target, origin, message, ok, err) {
      var url = brokerUrl + '/send/' + target;
      var xhr = new XMLHttpRequest();

      var request = {
        'origin': origin,
        'key': initiator.key,
        'message': message
      };

      xhr.open('POST', url);
      xhr.setRequestHeader('content-type', 'application/json');
      xhr.onreadystatechange = function() {
        if(4 !== xhr.readyState) return;
        if(!(200 === xhr.status || 201 === xhr.status)) {
          if(err && 'function' === typeof err) {
            err(xhr.statusText + ': ' + xhr.responseText);
          }
        }

        if(ok && 'function' === typeof ok) {
          ok.call(null);
        }
      };
      xhr.send(JSON.stringify(request));
    }

    function handleMessage(target, origin, message) {
      if(message.hasOwnProperty('answer')) {
        initiator.channel.close();
        var answer = message['answer'];
        peerConnection.setRemoteDescription({
          'type': 'answer',
          'sdp': answer
        }, function() {
          callback(initiator, 'oncomplete', [peerConnection]);
        }, fail);        
      }
    };

    function sendOffer(target, origin, peerConnection) {
      // FIXME: make this portable
      navigator.mozGetUserMedia({audio: true, fake: true}, function(stream) {
        peerConnection.addStream(stream);
        peerConnection.createOffer(function(offer) {
          var message = {
            'offer': offer.sdp
          };
          sendMessage(target, origin, message, undefined, fail);
        }, fail);
      }, fail);
    };

    function createChannel() {
      var channel = new EventSource(brokerUrl + '/channel');
      channel.addEventListener('channel', function(event)
      {
        console.log('initiator: channel message');
        var data = JSON.parse(event.data);
        initiator.cid = data['cid'];
        initiator.key = data['key'];

        sendOffer(initiator.sid, initiator.cid, peerConnection);
      });
      channel.addEventListener('message', function(event)
      {
        console.log('initiator: application message');
        var data = JSON.parse(event.data);
        var origin = data['origin'];
        var target = data['target'];
        var message = data['message'];

        handleMessage(target, origin, message);
      });
      initiator.channel = channel;
    };
    
    var peerConnection = new mozRTCPeerConnection();
    createChannel();
  };

  function Responder(brokerUrl, options)
  {    
    var responder = this;
    options = options || {};
    options['list'] = (undefined !== options['list']) ? options['list'] : false;
    options['metadata'] = (undefined !== options['metadata']) ? options['metadata'] : {};
    options['url'] = (undefined !== options['url']) ? options['url'] : window.location.toString();    

    responder.channel = undefined;
    responder.cid = undefined;
    responder.key = undefined;
    responder.sid = undefined;

    responder.onready = null;
    responder.onpending = null;
    responder.oncomplete = null;
    responder.onerror = null;

    function fail(error) {
      if(responder.onerror && 'function' === typeof responder.onerror) {
        if (!(error instanceof Error))
          error = new Error(error);
        responder.onerror.call(null, error);
      }
    };

    function sendMessage(target, origin, message, ok, err) {
      var url = brokerUrl + '/send/' + target;
      var xhr = new XMLHttpRequest();

      var request = {
        'origin': origin,
        'key': responder.key,
        'message': message
      };
      
      xhr.open('POST', url);
      xhr.setRequestHeader('content-type', 'application/json');
      xhr.onreadystatechange = function() {
        if(4 !== xhr.readyState) return;
        if(!(200 === xhr.status || 201 === xhr.status)) {
          if(err && 'function' === typeof err) {
            err(xhr.statusText + ': ' + xhr.responseText);
          }
        }

        if(ok && 'function' === typeof ok) {
          ok.call(null);
        }
      };
      xhr.send(JSON.stringify(request));
    }

    function handleMessage(target, origin, message) {
      if(message.hasOwnProperty('offer')) {
        var offer = message['offer'];
        var peerConnection = new mozRTCPeerConnection();
        navigator.mozGetUserMedia({audio: true, fake: true}, function(stream) {
          peerConnection.addStream(stream);
          peerConnection.setRemoteDescription({
            'type': 'offer',
            'sdp': offer
          }, function() {
            peerConnection.createAnswer(function(answer) {
              peerConnection.setLocalDescription(answer, function() {
                var message = {
                  'answer': answer.sdp
                };
                sendMessage(origin, target, message, function() {
                  callback(responder, 'oncomplete', [peerConnection]);
                }, fail);
              }, fail);
            }, fail);
          }, fail);
        }, fail);
      }
    };

    function createSession() {
      var url = brokerUrl + '/session';
      var xhr = new XMLHttpRequest();
      var request = {
        'cid': responder.cid,
        'key': responder.key,
        'list': options['list'],
        'metadata': options['metadata'],
        'url': options['url']
      };

      xhr.open('POST', url);
      xhr.setRequestHeader('content-type', 'application/json');
      xhr.onreadystatechange = function() {
        if(4 !== xhr.readyState) return;
        if(!201 === xhr.status) fail(xhr.statusText);

        var response = JSON.parse(xhr.responseText);
        responder.sid = response['sid'];

        callback(responder, 'onready', [responder.sid]);
      };
      xhr.send(JSON.stringify(request));
    };

    function createChannel() {
      var channel = new EventSource(brokerUrl + '/channel');
      channel.addEventListener('channel', function(event)
      {
        console.log('responder: channel message');
        var data = JSON.parse(event.data);
        var cid = data['cid'];
        var key = data['key'];

        responder.cid = cid;
        responder.key = key;

        createSession();        
      });
      channel.addEventListener('message', function(event)
      {
        console.log('responder: application message');
        var data = JSON.parse(event.data);
        var origin = data['origin'];
        var target = data['target'];
        var message = data['message'];

        handleMessage(target, origin, message);
      });
      responder.channel = channel;
    };    

    createChannel();    
  };
  Responder.prototype.close = function close() {
    this.channel.close();
  };

  function Host(brokerUrl, options) {
    var host = this;

    options = options || {};
    options['binaryType'] = (undefined !== options['binaryType']) ? options['binaryType'] : 'arraybuffer';

    host.connected = false;
    host.reliable = {
      channel: null,
      onmessage: null,
      send: null
    };
    host.unreliable = {
      channel: null,
      onmessage: null,
      send: null
    };

    host.onready = null;
    host.onconnect = null;
    host.ondisconnect = null;
    host.onerror = null;

    var responder = new Responder(brokerUrl, options);
    responder.oncomplete = function(peerConnection) {
      console.log('responder.oncomplete', peerConnection);
      responder.close();
      peerConnection.ondatachannel = function(channel) {
        if('reliable' === channel.label) {
          host.reliable.channel = channel;
        } else if('unreliable' === channel.label) {
          host.unreliable.channel = channel;
        } else {
          console.error('unknown data channel' + channel.label);
          return;
        }
        if(host.reliable.channel && host.unreliable.channel) {
          host.connected = true;
          callback(host, 'onconnect', []);
        }
        channel.binaryType = options['binaryType'];
        channel.onmessage = function(event) {
          if(channel.reliable) {
            callback(host.reliable, 'onmessage', [event]);
          } else {
            callback(host.unreliable, 'onmessage', [event]);
          }
        };
      };
      peerConnection.connectDataConnection(8001, 8000);
    };
    responder.onready = function(sid) {
      callback(host, 'onready', [sid]);
    };
    responder.onerror = function(error) {
      callback(host, 'onerror', [error]);
    };
  };

  var RELIABLE_CHANNEL_OPTIONS = {};
  var UNRELIABLE_CHANNEL_OPTIONS = {
    outOfOrderAllowed: true,
    maxRetransmitNum: 0
  };

  function Peer(brokerUrl, sid, options) {
    var peer = this;

    options = options || {};
    options['binaryType'] = (undefined !== options['binaryType']) ? options['binaryType'] : 'arraybuffer';

    peer.connected = false;
    peer.reliable = {
      channel: null,
      onmessage: null,
      send: null
    };
    peer.unreliable = {
      channel: null,
      onmessage: null,
      send: null
    };

    peer.onconnect = null;
    peer.ondisconnect = null;
    peer.onerror = null;

    var initiator = new Initiator(brokerUrl, sid);
    initiator.oncomplete = function(peerConnection) {
      console.log('initiator.oncomplete', peerConnection);
      peerConnection.onconnection = function() {        
        var reliable = peerConnection.createDataChannel('reliable', RELIABLE_CHANNEL_OPTIONS);
        var unreliable = peerConnection.createDataChannel('unreliable', UNRELIABLE_CHANNEL_OPTIONS);

        reliable.binaryType = options['binaryType'];
        unreliable.binaryType = options['binaryType'];

        var opened = 0;
        function handleOpen(event) {
          if(2 === ++ opened) {
            peer.connected = true;
            callback(peer, 'onconnect', []);
          }
        }
        reliable.onopen = handleOpen;
        unreliable.onopen = handleOpen;

        reliable.onmessage = function(event) {
          callback(peer.reliable, 'onmessage', [event]);
        };
        unreliable.onmessage = function(event) {
          callback(peer.unreliable, 'onmessage', [event]);
        }

        peer.reliable.channel = reliable;
        peer.unreliable.channel = unreliable;
      };
      peerConnection.connectDataConnection(8000, 8001);
    };
    initiator.onerror = function(error) {
      callback(peer, 'onerror', [error]);
    };
  };

  return {
    Host: Host,
    Peer: Peer
  };

})();