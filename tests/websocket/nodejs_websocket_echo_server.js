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

var port = 8088;
var ws = require('ws');
var wss = new ws.Server({ port: port });
console.log('WebSocket server listening on ws://localhost:' + port + '/');
wss.on('connection', function(ws) {
  console.log('Client connected!');
  ws.on('message', function(message) {
    if (typeof message === 'string') {
      console.log('received TEXT: ' + message.length + ' characters:');
      console.log('  "' + message + '"');
    } else {
      console.log('received BINARY: ' + message.length + ' bytes:');
      hexDump(message);
    }
    console.log('');
    ws.send(message); // Echo back the received message
  });
});
