// A WebSocket to TCP socket proxy
// Copyright 2012 Joel Martin
// Licensed under LGPL version 3 (see docs/LICENSE.LGPL-3)

// Known to work with node 0.6
// Requires node modules: ws, base64, optimist and policyfile
//     npm install ws base64 optimist policyfile

var argv = require('optimist').argv,
    net = require('net'),
    http = require('http'),
    url = require('url'),
    path = require('path'),
    fs = require('fs'),
    policyfile = require('policyfile'),

    base64 = require('base64/build/Release/base64'),
    Buffer = require('buffer').Buffer,
    WebSocketServer = require('ws').Server,

    httpServer, wsServer,
    source_host, source_port, target_host, target_port,
    web_path = null;


// Handle new WebSocket client
new_client = function(client) {
    console.log('WebSocket client connected');
    //console.log('protocol: ' + client.protocol);

    var target = net.createConnection(target_port,target_host);
    target.on('begin', function() {
        console.log('connected to target');
    });
    target.on('data', function(data) {
        client.send(base64.encode(new Buffer(data)));                     
    });
    target.on('end', function() {
        console.log('target disconnected');
    });

    client.on('message', function(msg) {
        //console.log('got some message');
        target.write(base64.decode(msg),'binary');
    });
    client.on('close', function(code, reason) {
        console.log('WebSocket client disconnected: ' + code + ' [' + reason + ']');
    });
    client.on('error', function(a) {
        console.log('WebSocket client error: ' + a);
    });
};


// Send an HTTP error response
http_error = function (response, code, msg) {
    response.writeHead(code, {"Content-Type": "text/plain"});
    response.write(msg + "\n");
    response.end();
    return;
}

// Process an HTTP static file request
http_request = function (request, response) {
//    console.log("pathname: " + url.parse(req.url).pathname);
//    res.writeHead(200, {'Content-Type': 'text/plain'});
//    res.end('okay');

    if (! argv.web) {
        return http_error(response, 403, "403 Permission Denied");
    }

    var uri = url.parse(request.url).pathname
        , filename = path.join(argv.web, uri);
    
    fs.exists(filename, function(exists) {
        if(!exists) {
            return http_error(response, 404, "404 Not Found");
        }

        if (fs.statSync(filename).isDirectory()) {
            filename += '/index.html';
        }

        fs.readFile(filename, "binary", function(err, file) {
            if(err) {
                return http_error(response, 500, err);
            }

            response.writeHead(200);
            response.write(file, "binary");
            response.end();
        });
    });
};

// parse source and target arguments into parts
try {
    source_arg = argv._[0].toString();
    target_arg = argv._[1].toString();

    var idx;
    idx = source_arg.indexOf(":");
    if (idx >= 0) {
        source_host = source_arg.slice(0, idx);
        source_port = parseInt(source_arg.slice(idx+1), 10);
    } else {
        source_host = "";
        source_port = parseInt(source_arg, 10);
    }

    idx = target_arg.indexOf(":");
    if (idx < 0) {
        throw("target must be host:port");
    }
    target_host = target_arg.slice(0, idx);
    target_port = parseInt(target_arg.slice(idx+1), 10);

    if (isNaN(source_port) || isNaN(target_port)) {
        throw("illegal port");
    }
} catch(e) {
    console.error("wsproxy.py [--web web_dir] [source_addr:]source_port target_addr:target_port");
    process.exit(2);
}

console.log("WebSocket settings: ");
console.log("    - proxying from " + source_host + ":" + source_port +
            " to " + target_host + ":" + target_port);
if (argv.web) {
    console.log("    - Web server active. Serving: " + argv.web);
}

httpServer = http.createServer(http_request);
httpServer.listen(source_port, function() {
    wsServer = new WebSocketServer({server: httpServer});
    wsServer.on('connection', new_client);
});

// Attach Flash policyfile answer service
policyfile.createServer().listen(-1, httpServer);
