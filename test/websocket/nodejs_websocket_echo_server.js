function padHex(number, digits) {
  var s = number.toString(16);
  return Array(Math.max(digits - s.length + 1, 0)).join(0) + s;
}

function hexDump(bytes) {
  var s = ' ';
  for(var i = 0; i < bytes.length; ++i) {
    s += ' ' + padHex(bytes[i], 2);
    if (i % 16 == 15) {
      console.log(s);
      s = ' ';
    }
    else if (i % 8 == 7) s += ' ';
  }
  console.log(s);
}

var decoder = new TextDecoder('utf-8');
var port = 8089;
var ws = require('ws');
var wss = new ws.WebSocketServer({ port: port });
console.log('WebSocket server listening on ws://localhost:' + port + '/');
wss.on('connection', function(ws) {
  console.log('Client connected!');
  ws.on('message', function(message, isBinary) {
    if (!isBinary) {
      var text = decoder.decode((new Uint8Array(message)).buffer);
      console.log('received TEXT: ' + text.length + ' characters:');
      console.log('  "' + text + '"');
    } else {
      console.log('received BINARY: ' + message.length + ' bytes:');
      hexDump(message);
    }
    console.log('');
    ws.send(message, { binary: isBinary }); // Echo back the received message
  });
});
