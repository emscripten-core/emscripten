/*
 * WebSockets IRC client
 * Copyright (C) 2011 Joel Martin
 * Licensed under LGPL-3 (see LICENSE.txt)
 *
 * Includes VT100.js from:
 *   http://code.google.com/p/sshconsole
 * Which was modified from:
 *   http://fzort.org/bi/o.php#vt100_js

 * IRC Client protocol:
 *   http://www.faqs.org/rfcs/rfc2812.html   
 */


function IRC(target, connect_callback, disconnect_callback) {

var that = {},  // Public API interface
    vt100, ws, sQ = [],
    state = "unconnected",
    irc_nick, irc_channel,
    termType = "VT100";


Array.prototype.pushStr = function (str) {
    var n = str.length;
    for (var i=0; i < n; i++) {
        this.push(str.charCodeAt(i));
    }
}

function do_send() {
    if (sQ.length > 0) {
        Util.Debug("Sending " + sQ);
        ws.send(sQ);
        sQ = [];
    }
}

function do_recv() {
    console.log(">> do_recv");
    var rQ, rQi, i;

    while (ws.rQlen() > 1) {
        rQ = ws.get_rQ();
        rQi = ws.get_rQi();
        for (i = rQi; i < rQ.length; i++) {
            if (rQ[i] === 10) {
                break;
            }
        }
        if (i >= rQ.length) {
            // No line break found
            break;
        }
        recvMsg(ws.rQshiftStr((i-rQi) + 1));
    }
    //console.log("<< do_recv");
}

// Handle an IRC message
function recvMsg(msg) {
    Util.Debug(">> recvMsg('" + msg + "')");

    var tokens = msg.split(' '), in_params = true,
        prefix, command, params = [], trailing = [];

    Util.Info("   tokens: " + tokens);

    if (tokens[0].charAt(0) === ":") {
        prefix = tokens.shift();
    }

    command = tokens.shift();

    while (tokens.length > 0) {
        if (tokens[0].charAt(0) === ":") {
            in_params = false;
        }
        if (in_params) {
            params.push(tokens.shift());
        } else {
            trailing.push(tokens.shift());
        }
    }

    Util.Info("   prefix: " + prefix);
    Util.Info("   command: " + command);
    Util.Info("   params: " + params);
    Util.Info("   trailing: " + trailing);

    // Show raw received
    vt100.write(msg);

    switch (command) {
    case "004":
        state = "registered";
        vt100.write("Joining channel #" + irc_channel);
        sendCmd("JOIN #" + irc_channel);
        break;
    case "JOIN":
        state = "joined";
        vt100.write("Joined channel #" + irc_channel);
        break;
        
    }

    Util.Debug("<< recvMsg('" + msg + "')");
}

function sendCmd(msg) {
    Util.Info("Sending: " + msg);
    sQ.pushStr(msg + "\r\n");
    do_send();
}

that.sendMsg = function(msg) {
    // TODO parse into message
    sendCmd("PRIVMSG #" + irc_channel + " :" + msg);
}


that.connect = function(host, port, encrypt, nick, channel) {
    var host = host,
        port = port,
        scheme = "ws://", uri;

    irc_nick = nick;
    irc_channel = channel;

    Util.Debug(">> connect");
    if ((!host) || (!port)) {
        alert("must set host and port");
        return false;
    }

    if (ws) {
        ws.close();
    }

    if (encrypt) {
        scheme = "wss://";
    }
    uri = scheme + host + ":" + port;
    Util.Info("connecting to " + uri);

    ws.open(uri);

    Util.Debug("<< connect");

    return true;
}

that.disconnect = function() {
    Util.Debug(">> disconnect");
    if (ws) {
        ws.close();
    }

    disconnect_callback();
    Util.Debug("<< disconnect");
}


function constructor() {
    /* Initialize Websock object */
    ws = new Websock();

    ws.on('message', do_recv);
    ws.on('open', function(e) {
        Util.Info(">> WebSockets.onopen");
        // Send registration commands
        state = "connected";
        sendCmd("NICK " + irc_nick);
        // TODO: how to determine this?
        sendCmd("USER joelm 0 * :Joel Martin");
        connect_callback();
        Util.Info("<< WebSockets.onopen");
    });
    ws.on('close', function(e) {
        Util.Info(">> WebSockets.onclose");
        that.disconnect();
        Util.Info("<< WebSockets.onclose");
    });
    ws.on('error', function(e) {
        Util.Info(">> WebSockets.onerror");
        that.disconnect();
        Util.Info("<< WebSockets.onerror");
    });

    /* Initialize the terminal emulator/renderer */

    vt100 = new VT100(80, 24, target);

    // Show cursor
    vt100.curs_set(true, false);

    /*
     * Override VT100 I/O routines
     */

    // Set handler for sending characters
    vt100.getch(
        function send_chr(chr, vt) {
            var i;
            Util.Debug(">> send_chr: " + chr);
            for (i = 0; i < chr.length; i++) {
                sQ.push(chr.charCodeAt(i));
            }
            do_send();
            vt100.getch(send_chr);
        }
    );

    vt100.debug = function(message) {
        Util.Debug(message + "\n");
    }

    vt100.warn = function(message) {
        Util.Warn(message + "\n");
    }

    vt100.curs_set = function(vis, grab, eventist)
    {
        this.debug("curs_set:: vis: " + vis + ", grab: " + grab);
        if (vis !== undefined)
            this.cursor_vis_ = (vis > 0);
    }

    return that;
}

return constructor(); // Return the public API interface

} // End of Telnet()
