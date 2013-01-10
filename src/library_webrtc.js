/*
 * WebRTC support.
 */

function callback(thing, method, args) {
if(method in thing && 'function' === typeof thing[method]) {
  thing[method].apply(thing, args);
}
}

function Offer(url, optMetadata, optExpires) {
var offer = this;
this.onpending = null;
this.oncomplete = null;
this.onerror = null;
this.id = undefined;
this.pc = new mozRTCPeerConnection();

function fail(error) {
  if(offer.onerror && 'function' === typeof offer.onerror) {
    if (!(error instanceof Error))
      error = new Error(error);
    offer.onerror.call(null, error);
  }
}

//XXX: This is a hack until RTCPeerConnection provides API for
// adding a network flow for DataChannels
navigator.mozGetUserMedia({audio: true, fake: true}, function(stream) {
  // Now we have a fake stream.
  offer.pc.addStream(stream);
  offer.pc.createOffer(function(rtc_offer) {
    // Now we have an offer SDP to give.
    var xhr = new XMLHttpRequest();
    xhr.open('POST', url + '/offer');
    xhr.setRequestHeader('content-type', 'application/json');
    xhr.onreadystatechange = function() {
      if(4 !== xhr.readyState) {
        return;
      }
      if(201 === xhr.status) {
        var response = JSON.parse(xhr.responseText);
        offer.id = response.id;
        if(offer.onpending && 'function' === typeof offer.onpending) {
          offer.onpending.call(null, url + '/offer/' + offer.id);
        }
        var answer = new EventSource(url + '/offer/' + offer.id + '/answer');
        answer.onmessage = function(e) {
          answer.close();
          // Now we have the answer SDP from the other end, so
          // set it as the remote description.
          var rtc_answer = {'type':'answer', 'sdp': JSON.parse(e.data).value};
          offer.pc.setRemoteDescription(rtc_answer, function() {
            // Now the remote description is set.
            if(offer.oncomplete && 'function' === typeof offer.oncomplete) {
              offer.oncomplete.call(null, offer.pc);
            }
          },
                                        function(err) { fail(err); });
        };
        answer.onerror = function(e) {
          if (e.readyState == EventSource.CLOSED) {
            // Connection was closed.
            return;
          }
          if(offer.onerror && 'function' === typeof offer.onerror) {
            offer.onerror.call(null, e);
          }
        };
      } else {
        if(offer.onerror && 'function' === typeof offer.onerror) {
          offer.onerror.call(null, new Error(xhr.statusText));
        }
      }
    };
    xhr.send(JSON.stringify({offer: rtc_offer.sdp}));
  },
                       function(err) { fail(err); });
}, function(err) { fail(err); });
}

function Answer(url) {
var answer = this;
this.oncomplete = null;
this.onerror = null;

function fail(error) {
  if(answer.onerror && 'function' === typeof answer.onerror) {
    if (!(error instanceof Error))
      error = new Error(error);
    answer.onerror.call(null, error);
  }
}

var getXhr = new XMLHttpRequest();
getXhr.open('GET', url + '/offer');
getXhr.onreadystatechange = function() {
  if(4 !== getXhr.readyState) {
    return;
  }
  if(200 === getXhr.status) {
    var response = JSON.parse(getXhr.responseText);
    var offer = {'type': 'offer', 'sdp': response.value};
    answer.pc = new mozRTCPeerConnection();

    //XXX: This is a hack until RTCPeerConnection provides API for
    // adding a network flow for DataChannels
    navigator.mozGetUserMedia({audio: true, fake: true}, function(stream) {
      // Now we have a fake stream.
      answer.pc.addStream(stream);
      answer.pc.setRemoteDescription(offer, function() {
        // Now the remote description is set.
        answer.pc.createAnswer(function(rtc_answer) {
          // Now we have an answer SDP to give.
          // First set it as local description.
          answer.pc.setLocalDescription(rtc_answer, function() {
            // Now the local description is set, so pass the answer
            // SDP back to the broker.
            var postXhr = new XMLHttpRequest();
            postXhr.open('POST', url + '/answer');
            postXhr.setRequestHeader('content-type', 'application/json');
            postXhr.onreadystatechange = function() {
              if(4 !== postXhr.readyState) {
                return;
              }
              if(200 === postXhr.status) {
                if(answer.oncomplete && 'function' === typeof answer.oncomplete) {
                  answer.oncomplete.call(null, answer.pc);
                }
              } else {
                if(answer.onerror && 'function' === typeof answer.onerror) {
                  answer.onerror.call(null, new Error(postXhr.statusText));
                }
              }
            };
            postXhr.send(JSON.stringify({value: rtc_answer.sdp}));
          }, function(err) { fail(err); });
        }, function(err) { fail(err); });
      },
                                     function(err) { fail(err); });
    }, function(err) { fail(err); });
  } else {
    if(answer.onerror && 'function' === typeof answer.onerror) {
      answer.onerror.call(null, new Error(getXhr.statusText));
    }
  }
};
getXhr.send(null);
}

// DataPeer wraps the Offer/Answer classes even further.
// It adds two DataChannels to the PeerConnection: a reliable (TCP-like)
// channel and an unreliable (UDP-like) channel, named .reliable and
// .unreliable respectively.
// There are four callbacks supported by DataPeer:
// onconnect: Called when both DataChannels are open.
// onerror: Called whenever some sort of error happens.
// onreliablemessage: Called when data arrives on the reliable channel.
// onunreliablemessage: Called when data arrives on the unreliable channel.
//TODO: support closing connection, handle closed events.
var default_broker = "http://webrtc-broker.herokuapp.com";
function DataPeer(broker, offer_id) {
this.connected = false;
this.onoffercreated = null;
this.onconnect = null;
this.ondisconnect = null;
this.onreliablemessge = null;
this.onunreliablemessage = null;
this.reliable = null;
this.unreliable = null;

var self = this;
if (!broker)
  broker = default_broker;

if (offer_id) {
  // Connecting to an offered connection.
  var answer = new Answer(broker + "/offer/" + offer_id);
  answer.oncomplete = function(pc) {
    console.log("answer.oncomplete");
    // Connect up some event listeners.
    pc.ondatachannel = function(channel) {
      console.log("pc.ondatachannel: " + channel.label);
      if (channel.label == "reliable") {
        if (self.reliable) {
          callback(self, "onerror",
                   ["Too many reliable DataChannels open!"]);
          return;
        }
        self.reliable = channel;
      } else if (channel.label == "unreliable") {
        if (self.unreliable) {
          callback(self, "onerror",
                   ["Too many unreliable DataChannels open!"]);
          return;
        }
        self.unreliable = channel;
      } else {
        console.log("unknown DataChannel " + channel.label);
        return;
      }
      if (self.reliable && self.unreliable) {
        self.connected = true;
        callback(self, "onconnect", []);
      }
      channel.binaryType = "blob";
      channel.onmessage = function(event) {
        callback(self,
                 channel.reliable ? "onreliablemessage"
                                  : "onunreliablemessage",
                 [event]);
      };
    };

    //XXX: this is a hack until the DataChannel APIs are finalized.
    pc.connectDataConnection(8001,8000);
  };
  answer.onerror = function(e) {
    callback(self, 'onerror', [e]);
  };
} else {
  // Creating a new offer
  var offer = new Offer(broker);
  offer.onpending = function(url) {
    console.log("offer.onpending");
    callback(self, 'onoffercreated', [url, offer.id]);
  };
  offer.oncomplete = function(pc) {
    console.log("offer.oncomplete");
    function datachannelopen() {
      self.channels_open++;
      if (self.channels_open == 2) {
        self.connected = true;
        callback(self, "onconnect", []);
      }
    }
    pc.onconnection = function() {
      self.channels_open = 0;
      self.reliable = pc.createDataChannel("reliable", {});
      self.reliable.binaryType = "blob";
      self.reliable.onmessage = function(event) {
        callback(self, "onreliablemessage", [event]);
      };
      self.reliable.onopen = datachannelopen;

      self.unreliable = pc.createDataChannel("unreliable",
                                             {outOfOrderAllowed: true,
                                              maxRetransmitNum: 0});
      self.unreliable.binaryType = "blob";
      self.unreliable.onmessage = function(event) {
        callback(self, "onunreliablemessage", [event]);
      };
      self.unreliable.onopen = datachannelopen;
    };
    //XXX: this is a hack until the DataChannel APIs are finalized.
    pc.connectDataConnection(8000,8001);
  };
  offer.onerror = function(e) {
    callback(self, 'onerror', [e]);
  };
};
}

var LibraryWebRTC = {
  $WebRTC: {
    Offer: Offer,
	Answer: Answer,
	DataPeer: DataPeer
  }
}

mergeInto(LibraryManager.library, LibraryWebRTC);