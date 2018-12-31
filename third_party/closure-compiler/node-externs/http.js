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
 * @fileoverview Definitions for node's http module. Depends on the events module.
 * @see http://nodejs.org/api/http.html
 * @see https://github.com/joyent/node/blob/master/lib/http.js
 * @externs
 */

/**
BEGIN_NODE_INCLUDE
var http = require('http');
END_NODE_INCLUDE
 */

var http = {};

/**
 * @typedef {function(http.IncomingMessage, http.ServerResponse)}
 */
http.requestListener;

/**
 * @param {http.requestListener=} listener
 * @return {http.Server}
 */
http.createServer = function(listener) {};

/**
 * @param {http.requestListener=} listener  
 * @constructor
 * @extends events.EventEmitter
 */
http.Server = function(listener) {};

/**
 * @param {(number|string)} portOrPath
 * @param {(string|Function)=} hostnameOrCallback
 * @param {Function=} callback
 */
http.Server.prototype.listen = function(portOrPath, hostnameOrCallback, callback) {};

/**
 */
http.Server.prototype.close = function() {};

/**
 * @constructor
 * @extends stream.Readable
 */
http.IncomingMessage = function() {};

/**
 * @type {?string}
 * */
http.IncomingMessage.prototype.method;

/**
 * @type {?string}
 */
http.IncomingMessage.prototype.url;

/**
 * @type {Object}
 * */
http.IncomingMessage.prototype.headers;

/**
 * @type {Object}
 * */
http.IncomingMessage.prototype.trailers;

/**
 * @type {string}
 */
http.IncomingMessage.prototype.httpVersion;

/**
 * @type {string}
 */
http.IncomingMessage.prototype.httpVersionMajor;

/**
 * @type {string}
 */
http.IncomingMessage.prototype.httpVersionMinor;

/**
 * @type {*}
 */
http.IncomingMessage.prototype.connection;

/**
 * @type {?number}
 */
http.IncomingMessage.prototype.statusCode;

/**
 * @type {net.Socket}
 */
http.IncomingMessage.prototype.socket;

/**
 * @param {number} msecs
 * @param {function()} callback
 */
http.IncomingMessage.prototype.setTimeout = function(msecs, callback) {};

/**
 * @constructor
 * @extends events.EventEmitter
 * @private
 */
http.ServerResponse = function() {};

/**
 */
http.ServerResponse.prototype.writeContinue = function() {};

/**
 * @param {number} statusCode
 * @param {*=} reasonPhrase
 * @param {*=} headers
 */
http.ServerResponse.prototype.writeHead = function(statusCode, reasonPhrase, headers) {};

/**
 * @type {number}
 */
http.ServerResponse.prototype.statusCode;

/**
 * @param {string} name
 * @param {string} value
 */
http.ServerResponse.prototype.setHeader = function(name, value) {};

/**
 * @param {string} name
 * @return {string|undefined} value
 */
http.ServerResponse.prototype.getHeader = function(name) {};

/**
 * @param {string} name
 */
http.ServerResponse.prototype.removeHeader = function(name) {};

/**
 * @param {string|Array|buffer.Buffer} chunk
 * @param {string=} encoding
 */
http.ServerResponse.prototype.write = function(chunk, encoding) {};

/**
 * @param {Object} headers
 */
http.ServerResponse.prototype.addTrailers = function(headers) {};

/**
 * @param {(string|Array|buffer.Buffer)=} data
 * @param {string=} encoding
 */
http.ServerResponse.prototype.end = function(data, encoding) {};

/**
 * @constructor
 * @extends events.EventEmitter
 * @private
 */
http.ClientRequest = function() {};

/**
 * @param {string|Array|buffer.Buffer} chunk
 * @param {string=} encoding
 */
http.ClientRequest.prototype.write = function(chunk, encoding) {};

/**
 * @param {(string|Array|buffer.Buffer)=} data
 * @param {string=} encoding
 */
http.ClientRequest.prototype.end = function(data, encoding) {};

/**
 */
http.ClientRequest.prototype.abort = function() {};

/**
 * @param {Object} options
 * @param {function(http.IncomingMessage)} callback
 * @return {http.ClientRequest}
 */
http.request = function(options, callback) {};

/**
 * @param {Object} options
 * @param {function(http.IncomingMessage)} callback
 * @return {http.ClientRequest}
 */
http.get = function(options, callback) {};

/**
 * @constructor
 * @extends events.EventEmitter
 */
http.Agent = function() {};

/**
 * @type {number}
 */
http.Agent.prototype.maxSockets;

/**
 * @type {number}
 */
http.Agent.prototype.sockets;

/**
 * @type {Array.<http.ClientRequest>}
 */
http.Agent.prototype.requests;

/**
 * @type {http.Agent}
 */
http.globalAgent;
