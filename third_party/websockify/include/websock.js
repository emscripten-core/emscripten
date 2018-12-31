/*
 * Websock: high-performance binary WebSockets
 * Copyright (C) 2012 Joel Martin
 * Licensed under MPL 2.0 (see LICENSE.txt)
 *
 * Websock is similar to the standard WebSocket object but Websock
 * enables communication with raw TCP sockets (i.e. the binary stream)
 * via websockify. This is accomplished by base64 encoding the data
 * stream between Websock and websockify.
 *
 * Websock has built-in receive queue buffering; the message event
 * does not contain actual data but is simply a notification that
 * there is new data available. Several rQ* methods are available to
 * read binary data off of the receive queue.
 */

/*jslint browser: true, bitwise: false, plusplus: false */
/*global Util, Base64 */


// Load Flash WebSocket emulator if needed

// To force WebSocket emulator even when native WebSocket available
//window.WEB_SOCKET_FORCE_FLASH = true;
// To enable WebSocket emulator debug:
//window.WEB_SOCKET_DEBUG=1;

if (window.WebSocket && !window.WEB_SOCKET_FORCE_FLASH) {
    Websock_native = true;
} else if (window.MozWebSocket && !window.WEB_SOCKET_FORCE_FLASH) {
    Websock_native = true;
    window.WebSocket = window.MozWebSocket;
} else {
    /* no builtin WebSocket so load web_socket.js */

    Websock_native = false;
    (function () {
        window.WEB_SOCKET_SWF_LOCATION = Util.get_include_uri() +
                    "web-socket-js/WebSocketMain.swf";
        if (Util.Engine.trident) {
            Util.Debug("Forcing uncached load of WebSocketMain.swf");
            window.WEB_SOCKET_SWF_LOCATION += "?" + Math.random();
        }
        Util.load_scripts(["web-socket-js/swfobject.js",
                           "web-socket-js/web_socket.js"]);
    }());
}


function Websock() {
"use strict";

var api = {},         // Public API
    websocket = null, // WebSocket object
    mode = 'base64',  // Current WebSocket mode: 'binary', 'base64'
    rQ = [],          // Receive queue
    rQi = 0,          // Receive queue index
    rQmax = 10000,    // Max receive queue size before compacting
    sQ = [],          // Send queue

    eventHandlers = {
        'message' : function() {},
        'open'    : function() {},
        'close'   : function() {},
        'error'   : function() {}
    },

    test_mode = false;


//
// Queue public functions
//

function get_sQ() {
    return sQ;
}

function get_rQ() {
    return rQ;
}
function get_rQi() {
    return rQi;
}
function set_rQi(val) {
    rQi = val;
}

function rQlen() {
    return rQ.length - rQi;
}

function rQpeek8() {
    return (rQ[rQi]      );
}
function rQshift8() {
    return (rQ[rQi++]      );
}
function rQunshift8(num) {
    if (rQi === 0) {
        rQ.unshift(num);
    } else {
        rQi -= 1;
        rQ[rQi] = num;
    }

}
function rQshift16() {
    return (rQ[rQi++] <<  8) +
           (rQ[rQi++]      );
}
function rQshift32() {
    return (rQ[rQi++] << 24) +
           (rQ[rQi++] << 16) +
           (rQ[rQi++] <<  8) +
           (rQ[rQi++]      );
}
function rQshiftStr(len) {
    if (typeof(len) === 'undefined') { len = rQlen(); }
    var arr = rQ.slice(rQi, rQi + len);
    rQi += len;
    return String.fromCharCode.apply(null, arr);
}
function rQshiftBytes(len) {
    if (typeof(len) === 'undefined') { len = rQlen(); }
    rQi += len;
    return rQ.slice(rQi-len, rQi);
}

function rQslice(start, end) {
    if (end) {
        return rQ.slice(rQi + start, rQi + end);
    } else {
        return rQ.slice(rQi + start);
    }
}

// Check to see if we must wait for 'num' bytes (default to FBU.bytes)
// to be available in the receive queue. Return true if we need to
// wait (and possibly print a debug message), otherwise false.
function rQwait(msg, num, goback) {
    var rQlen = rQ.length - rQi; // Skip rQlen() function call
    if (rQlen < num) {
        if (goback) {
            if (rQi < goback) {
                throw("rQwait cannot backup " + goback + " bytes");
            }
            rQi -= goback;
        }
        //Util.Debug("   waiting for " + (num-rQlen) +
        //           " " + msg + " byte(s)");
        return true;  // true means need more data
    }
    return false;
}

//
// Private utility routines
//

function encode_message() {
    if (mode === 'binary') {
        // Put in a binary arraybuffer
        return (new Uint8Array(sQ)).buffer;
    } else {
        // base64 encode
        return Base64.encode(sQ);
    }
}

function decode_message(data) {
    //Util.Debug(">> decode_message: " + data);
    if (mode === 'binary') {
        // push arraybuffer values onto the end
        var u8 = new Uint8Array(data);
        for (var i = 0; i < u8.length; i++) {
            rQ.push(u8[i]);
        }
    } else {
        // base64 decode and concat to the end
        rQ = rQ.concat(Base64.decode(data, 0));
    }
    //Util.Debug(">> decode_message, rQ: " + rQ);
}


//
// Public Send functions
//

function flush() {
    if (websocket.bufferedAmount !== 0) {
        Util.Debug("bufferedAmount: " + websocket.bufferedAmount);
    }
    if (websocket.bufferedAmount < api.maxBufferedAmount) {
        //Util.Debug("arr: " + arr);
        //Util.Debug("sQ: " + sQ);
        if (sQ.length > 0) {
            websocket.send(encode_message(sQ));
            sQ = [];
        }
        return true;
    } else {
        Util.Info("Delaying send, bufferedAmount: " +
                websocket.bufferedAmount);
        return false;
    }
}

// overridable for testing
function send(arr) {
    //Util.Debug(">> send_array: " + arr);
    sQ = sQ.concat(arr);
    return flush();
}

function send_string(str) {
    //Util.Debug(">> send_string: " + str);
    api.send(str.split('').map(
        function (chr) { return chr.charCodeAt(0); } ) );
}

//
// Other public functions

function recv_message(e) {
    //Util.Debug(">> recv_message: " + e.data.length);

    try {
        decode_message(e.data);
        if (rQlen() > 0) {
            eventHandlers.message();
            // Compact the receive queue
            if (rQ.length > rQmax) {
                //Util.Debug("Compacting receive queue");
                rQ = rQ.slice(rQi);
                rQi = 0;
            }
        } else {
            Util.Debug("Ignoring empty message");
        }
    } catch (exc) {
        if (typeof exc.stack !== 'undefined') {
            Util.Warn("recv_message, caught exception: " + exc.stack);
        } else if (typeof exc.description !== 'undefined') {
            Util.Warn("recv_message, caught exception: " + exc.description);
        } else {
            Util.Warn("recv_message, caught exception:" + exc);
        }
        if (typeof exc.name !== 'undefined') {
            eventHandlers.error(exc.name + ": " + exc.message);
        } else {
            eventHandlers.error(exc);
        }
    }
    //Util.Debug("<< recv_message");
}


// Set event handlers
function on(evt, handler) { 
    eventHandlers[evt] = handler;
}

function init(protocols, ws_schema) {
    rQ         = [];
    rQi        = 0;
    sQ         = [];
    websocket  = null;

    var bt = false,
        wsbt = false,
        try_binary = false;

    // Check for full typed array support
    if (('Uint8Array' in window) &&
        ('set' in Uint8Array.prototype)) {
        bt = true;
    }
    // Check for full binary type support in WebSocket
    // Inspired by:
    // https://github.com/Modernizr/Modernizr/issues/370
    // https://github.com/Modernizr/Modernizr/blob/master/feature-detects/websockets/binary.js
    try {
        if (bt && ('binaryType' in WebSocket.prototype ||
                   !!(new WebSocket(ws_schema + '://.').binaryType))) {
            Util.Info("Detected binaryType support in WebSockets");
            wsbt = true;
        }
    } catch (exc) {
        // Just ignore failed test localhost connections
    }

    // Default protocols if not specified
    if (typeof(protocols) === "undefined") {
        if (wsbt) {
            protocols = ['binary', 'base64'];
        } else {
            protocols = 'base64';
        }
    }

    // If no binary support, make sure it was not requested
    if (!wsbt) {
        if (protocols === 'binary') {
            throw("WebSocket binary sub-protocol requested but not supported");
        }
        if (typeof(protocols) === "object") {
            var new_protocols = [];
            for (var i = 0; i < protocols.length; i++) {
                if (protocols[i] === 'binary') {
                    Util.Error("Skipping unsupported WebSocket binary sub-protocol");
                } else {
                    new_protocols.push(protocols[i]);
                }
            }
            if (new_protocols.length > 0) {
                protocols = new_protocols;
            } else {
                throw("Only WebSocket binary sub-protocol was requested and not supported.");
            }
        }
    }

    return protocols;
}

function open(uri, protocols) {
    var ws_schema = uri.match(/^([a-z]+):\/\//)[1];
    protocols = init(protocols, ws_schema);

    if (test_mode) {
        websocket = {};
    } else {
        websocket = new WebSocket(uri, protocols);
        if (protocols.indexOf('binary') >= 0) {
            websocket.binaryType = 'arraybuffer';
        }
    }

    websocket.onmessage = recv_message;
    websocket.onopen = function() {
        Util.Debug(">> WebSock.onopen");
        if (websocket.protocol) {
            mode = websocket.protocol;
            Util.Info("Server chose sub-protocol: " + websocket.protocol);
        } else {
            mode = 'base64';
            Util.Error("Server select no sub-protocol!: " + websocket.protocol);
        }
        eventHandlers.open();
        Util.Debug("<< WebSock.onopen");
    };
    websocket.onclose = function(e) {
        Util.Debug(">> WebSock.onclose");
        eventHandlers.close(e);
        Util.Debug("<< WebSock.onclose");
    };
    websocket.onerror = function(e) {
        Util.Debug(">> WebSock.onerror: " + e);
        eventHandlers.error(e);
        Util.Debug("<< WebSock.onerror");
    };
}

function close() {
    if (websocket) {
        if ((websocket.readyState === WebSocket.OPEN) ||
            (websocket.readyState === WebSocket.CONNECTING)) {
            Util.Info("Closing WebSocket connection");
            websocket.close();
        }
        websocket.onmessage = function (e) { return; };
    }
}

// Override internal functions for testing
// Takes a send function, returns reference to recv function
function testMode(override_send, data_mode) {
    test_mode = true;
    mode = data_mode;
    api.send = override_send;
    api.close = function () {};
    return recv_message;
}

function constructor() {
    // Configuration settings
    api.maxBufferedAmount = 200;

    // Direct access to send and receive queues
    api.get_sQ       = get_sQ;
    api.get_rQ       = get_rQ;
    api.get_rQi      = get_rQi;
    api.set_rQi      = set_rQi;

    // Routines to read from the receive queue
    api.rQlen        = rQlen;
    api.rQpeek8      = rQpeek8;
    api.rQshift8     = rQshift8;
    api.rQunshift8   = rQunshift8;
    api.rQshift16    = rQshift16;
    api.rQshift32    = rQshift32;
    api.rQshiftStr   = rQshiftStr;
    api.rQshiftBytes = rQshiftBytes;
    api.rQslice      = rQslice;
    api.rQwait       = rQwait;

    api.flush        = flush;
    api.send         = send;
    api.send_string  = send_string;

    api.on           = on;
    api.init         = init;
    api.open         = open;
    api.close        = close;
    api.testMode     = testMode;

    return api;
}

return constructor();

}
