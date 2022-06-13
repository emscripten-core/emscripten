/*
 * Copyright 2012 The Closure Compiler Authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @fileoverview Definitions for node's net module. Depends on the events and buffer modules.
 * @see http://nodejs.org/api/net.html
 * @see https://github.com/joyent/node/blob/master/lib/net.js
 * @externs
 * @author Daniel Wirtz <dcode@dcode.io>
 */

/**
 BEGIN_NODE_INCLUDE
 var net = require('net');
 END_NODE_INCLUDE
 */

/**
 * @type {Object.<string,*>}
 */
var net = {};

/**
 * @typedef {{allowHalfOpen: ?boolean}}
 */
net.CreateOptions;

/**
 * @param {(net.CreateOptions|function(...))=} options
 * @param {function(...)=} connectionListener
 * @return {net.Server}
 */
net.createServer = function(options, connectionListener) {};

/**
 * @typedef {{port: ?number, host: ?string, localAddress: ?string, path: ?string, allowHalfOpen: ?boolean}}
 */
net.ConnectOptions;

/**
 * @param {net.ConnectOptions|number|string} arg1
 * @param {(function(...)|string)=} arg2
 * @param {function(...)=} arg3
 */
net.connect = function(arg1, arg2, arg3) {};

/**
 * @param {net.ConnectOptions|number|string} arg1
 * @param {(function(...)|string)=} arg2
 * @param {function(...)=} arg3
 */
net.createConnection = function(arg1, arg2, arg3) {};

/**
 * @constructor
 * @extends events.EventEmitter
 */
net.Server = function() {};

/**
 * 
 * @param {number|*} port
 * @param {(string|number|function(...))=} host
 * @param {(number|function(...))=} backlog
 * @param {function(...)=} callback
 */
net.Server.prototype.listen = function(port, host, backlog, callback) {};

/**
 * @param {function(...)=} callback
 */
net.Server.prototype.close = function(callback) {};

/**
 * @return {{port: number, family: string, address: string}}
 */
net.Server.prototype.address = function() {};

/**
 * @type {number}
 */
net.Server.prototype.maxConnectinos;

/**
 * @type {number}
 */
net.Server.prototype.connections;

/**
 * @constructor
 * @param {{fd: ?*, type: ?string, allowHalfOpen: ?boolean}=} options
 * @extends events.EventEmitter
 */
net.Socket = function(options) {};

/**
 * @param {number|string|function(...)} port
 * @param {(string|function(...))=} host
 * @param {function(...)=} connectListener
 */
net.Socket.prototype.connect = function(port, host, connectListener) {};

/**
 * @type {number}
 */
net.Socket.prototype.bufferSize;

/**
 * @param {?string=} encoding
 */
net.Socket.prototype.setEncoding = function(encoding) {};

/**
 * @param {string|nodeBuffer.Buffer} data
 * @param {(string|function(...))=}encoding
 * @param {function(...)=} callback
 */
net.Socket.prototype.write = function(data, encoding, callback) {};

/**
 * @param {(string|nodeBuffer.Buffer)=}data
 * @param {string=} encoding
 */
net.Socket.prototype.end = function(data, encoding) {};

/**
 */
net.Socket.prototype.destroy = function() {};

/**
 */
net.Socket.prototype.pause = function() {};

/**
 */
net.Socket.prototype.resume = function() {};

/**
 * @param {number} timeout
 * @param {function(...)=} callback
 */
net.Socket.prototype.setTimeout = function(timeout, callback) {};

/**
 * @param {boolean=} noDelay
 */
net.Socket.prototype.setNoDelay = function(noDelay) {};

/**
 * @param {(boolean|number)=} enable
 * @param {number=} initialDelay
 */
net.Socket.prototype.setKeepAlive = function(enable, initialDelay) {};

/**
 * @return {string}
 */
net.Socket.prototype.address = function() {};

/**
 * @type {?string}
 */
net.Socket.prototype.remoteAddress;

/**
 * @type {?number}
 */
net.Socket.prototype.remotePort;

/**
 * @type {number}
 */
net.Socket.prototype.bytesRead;

/**
 * @type {number}
 */
net.Socket.prototype.bytesWritten;

/**
 * @param {*} input
 * @return {number}
 */
net.isIP = function(input) {};

/**
 * @param {*} input
 * @return {boolean}
 */
net.isIPv4 = function(input) {};

/**
 * @param {*} input
 * @return {boolean}
 */
net.isIPv6 = function(input) {};
