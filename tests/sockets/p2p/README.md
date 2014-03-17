# WebRTC peer-to-peer

This is a browser JS library that makes it easy to manage RTC peer connections, streams and data channels.
It's currently used in [emscripten](http://github.com/kripken/emscripten) to provide data transport for the posix sockets implementation.

## Requirements

You will need either Firefox [Nightly](http://nightly.mozilla.org/), or Chrome [Canary](https://www.google.com/intl/en/chrome/browser/canary.html).
You can also use Chrome [Dev Channel](http://www.chromium.org/getting-involved/dev-channel).

## What it does

* Firefox (nightly) and Chrome (dev/canary) supported
* Binary transport using arraybuffers (Firefox only!)
* Multiple connections
* Broker service (on nodejitsu), or run your own
* Connection timeouts

## What it doesn't do (yet!)

* Interoperability between Firefox and Chrome
* Peer brokering for establishing new connections through existing peer-to-peer

## Quick start

Setting up a peer is easy. The code below will create a new peer and listen for incoming connections.
The `onconnection` handler is called each time a new connection is ready.

````javascript
// Create a new Peer
var peer = new Peer(
  'http://webrtcb.jit.su:80', // You can use this broker if you don't want to set one up
  {
    binaryType: 'arraybuffer',
    video: false,
    audio: false
  }
);

// Listen for incoming connections
peer.listen();

var connections = {};

// Handle new connections
peer.onconnection = function(connection) {
  // Store connections here so we can use them later
  connections[connection.id] = connection; // Each connection has a unique ID

  connection.ondisconnect = function(reason) {
    delete connections[connection.id];
  };
  
  connection.onerror = function(error) {
    console.error(error);
  };
  
  // Handle messages from this channel
  // The label will be 'reliable' or 'unreliable', depending on how it was received
  connection.onmessage = function(label, message) {
    console.log(label, message);
  };
  
  // Sends a message to the other peer using the reliable data channel
  connection.send('reliable', 'hi!'); 
  
  // The connection exposes the underlying media streams
  // You can attach them to DOM elements to get video/audio, if available
  console.log(connection.streams.local, connection.streams.remote);
  
  // Closes the connection
  // This will cause `ondisconnect` to fire
  connection.close();
};

// Print our route when it's available
peer.onroute = function(route) {
  // This is our routing address from the broker
  // It's used by peers who wish to connect with us
  console.log('route:', route);
};

peer.onerror = function(error) {
  console.log(error);
};
````

Another peer can connect easily to the one we made above by calling `connect()` on its routing address.

````javascript
peer.connect(route);
````

## Demo

There are some files in the `demo` directory that offer an example. 
You can load it [here](http://js-platform.github.com/p2p/examples/data-demo.html) and open the `connect` URL in another window.
For this example, the `route` is added to the URL query string so that the other peer can parse it and connect when the page loads, so all you need to share is the URL.
