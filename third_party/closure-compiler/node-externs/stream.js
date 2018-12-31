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
 * @fileoverview Definitions for node's stream module. Depends on the events module.
 * @see http://nodejs.org/api/stream.html
 * @see https://github.com/joyent/node/blob/master/lib/stream.js
 * @externs
 * @author Daniel Wirtz <dcode@dcode.io>
 */

/**
 BEGIN_NODE_INCLUDE
 var stream = require('stream');
 END_NODE_INCLUDE
 */

var stream = {};

/**
 * @constructor
 * @param {Object=} options
 * @extends events.EventEmitter
 */
stream.Stream = function(options) {};

/**
 * @param {stream.Writable} dest
 * @param {{end: boolean}=} pipeOpts
 * @return {stream.Writable}
 */
stream.Stream.prototype.pipe = function(dest, pipeOpts) {};

/**
 * @constructor
 * @extends stream.Readable
 */
stream.ReadableStream = function() {};

/**
 * @type {boolean}
 */
stream.ReadableStream.prototype.readable;

/**
 * @param {string=} encoding
 */
stream.ReadableStream.prototype.setEncoding = function(encoding) {};

/**
 */
stream.ReadableStream.prototype.destroy = function() {};

/**
 * @constructor
 * @extends stream.Writable
 */
stream.WritableStream = function() {};

/**
 */
stream.WritableStream.prototype.drain = function() {};

/**
 * @type {boolean}
 */
stream.WritableStream.prototype.writable;

/**
 * @param {string|buffer.Buffer} buffer
 * @param {string=} encoding
 */
stream.WritableStream.prototype.write = function(buffer, encoding) {};

/**
 * @param {string|buffer.Buffer=} buffer
 * @param {string=} encoding
 * @param {function(*=)=} cb
 */
stream.WritableStream.prototype.end = function(buffer, encoding, cb) {};

/**
 */
stream.WritableStream.prototype.destroy = function() {};

/**
 */
stream.WritableStream.prototype.destroySoon = function() {};

// Undocumented

/**
 * @constructor
 * @param {Object=} options
 * @extends stream.Stream
 */
stream.Readable = function(options) {};

/**
 * @type {boolean}
 * @deprecated
 */
stream.Readable.prototype.readable;

/**
 * @protected
 * @param {string|buffer.Buffer|null} chunk
 * @return {boolean}
 */
stream.Readable.prototype.push = function(chunk) {};

/**
 * @param {string|buffer.Buffer|null} chunk
 * @return {boolean}
 */
stream.Readable.prototype.unshift = function(chunk) {};

/**
 * @param {string} enc
 */
stream.Readable.prototype.setEncoding = function(enc) {};

/**
 * @param {number=} n
 * @return {buffer.Buffer|string|null}
 */
stream.Readable.prototype.read = function(n) {};

/**
 * @protected
 * @param {number} n
 */
stream.Readable.prototype._read = function(n) {};

/**
 * @param {stream.Writable=} dest
 * @return {stream.Readable}
 */
stream.Readable.prototype.unpipe = function(dest) {};

/**
 */
stream.Readable.prototype.resume = function() {};

/**
 */
stream.Readable.prototype.pause = function() {};

/**
 * @param {stream.Stream} stream
 * @return {stream.Readable}
 */
stream.Readable.prototype.wrap = function(stream) {};

/**
 * @constructor
 * @param {Object=} options
 * @extends stream.Stream
 */
stream.Writable = function(options) {};

/**
 * @deprecated
 * @type {boolean}
 */
stream.Writable.prototype.writable;

/**
 * @param {string|buffer.Buffer} chunk
 * @param {string=} encoding
 * @param {function(*=)=} cb
 * @return {boolean}
 */
stream.Writable.prototype.write = function(chunk, encoding, cb) {};

/**
 * @protected
 * @param {string|buffer.Buffer} chunk
 * @param {string} encoding
 * @param {function(*=)} cb
 */
stream.Writable.prototype._write = function(chunk, encoding, cb) {};

/**
 * @param {string|buffer.Buffer=} chunk
 * @param {string=} encoding
 * @param {function(*=)=} cb
 */
stream.Writable.prototype.end = function(chunk, encoding, cb) {};

/**
 * @constructor
 * @param {Object=} options
 * @extends stream.Readable
 * Xextends stream.Writable
 */
stream.Duplex = function(options) {};

/**
 * @type {boolean}
 */
stream.Duplex.prototype.allowHalfOpen;


/**
 * @param {Object=} options
 * @constructor
 * @extends stream.Duplex
 */
stream.Transform = function(options) {};

/**
 * @protected
 * @param {string|buffer.Buffer} chunk
 * @param {string} encoding
 * @param {function(*=)} cb
 */
stream.Transform._transform = function(chunk, encoding, cb) {};

/**
 * @protected
 * @param {function(*=)} cb
 */
stream.Transform._flush = function(cb) {};

/**
 * @param {Object=} options
 * @constructor
 * @extends stream.Transform
 */
stream.PassThrough = function(options) {};
