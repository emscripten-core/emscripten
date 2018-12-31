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
 * @fileoverview Definitions for node's buffer module.
 * @see http://nodejs.org/api/buffer.html
 * @see https://github.com/joyent/node/blob/master/lib/buffer.js
 * @externs
 * @author Daniel Wirtz <dcode@dcode.io>
 */

/**
 BEGIN_NODE_INCLUDE
 var buffer = require('buffer');
 END_NODE_INCLUDE
 */

/**
 * @type {Object.<string,*>}
 */
var buffer = {};

/**
 * @param {...*} var_args
 * @constructor
 * @nosideeffects
 */
buffer.Buffer = function(var_args) {};

/**
 * @param {string} encoding
 * @return {boolean}
 */
buffer.Buffer.isEncoding = function(encoding) {};

/**
 * @param {*} obj
 * @return {boolean}
 * @nosideeffects
 */
buffer.Buffer.isBuffer = function(obj) {};

/**
 * @param {string} string
 * @param {string=} encoding
 * @return {number}
 * @nosideeffects
 */
buffer.Buffer.byteLength = function(string, encoding) {};

/**
 * @param {Array.<buffer.Buffer>} list
 * @param {number=} totalLength
 * @return {buffer.Buffer}
 * @nosideeffects
 */
buffer.Buffer.concat = function(list, totalLength) {};

/**
 * @param {number} offset
 * @return {*}
 */
buffer.Buffer.prototype.get = function(offset) {};

/**
 * @param {number} offset
 * @param {*} v
 */
buffer.Buffer.prototype.set = function(offset, v) {};

/**
 * @param {string} string
 * @param {number|string=} offset
 * @param {number|string=} length
 * @param {number|string=} encoding
 * @return {*}
 */
buffer.Buffer.prototype.write = function(string, offset, length, encoding) {};

/**
 * @return {Array}
 */
buffer.Buffer.prototype.toJSON = function() {};

/**
 * @type {number}
 */
buffer.Buffer.prototype.length;

/**
 * @param {buffer.Buffer} targetBuffer
 * @param {number=} targetStart
 * @param {number=} sourceStart
 * @param {number=} sourceEnd
 * @return {buffer.Buffer}
 */
buffer.Buffer.prototype.copy = function(targetBuffer, targetStart, sourceStart, sourceEnd){};

/**
 * @param {number=} start
 * @param {number=} end
 * @return {buffer.Buffer}
 * @nosideeffects
 */
buffer.Buffer.prototype.slice = function(start, end) {};

/**
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.readUInt8 = function(offset, noAssert) {};

/**
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.readUInt16LE = function(offset, noAssert) {};

/**
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.readUInt16BE = function(offset, noAssert) {};

/**
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.readUInt32LE = function(offset, noAssert) {};

/**
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.readUInt32BE = function(offset, noAssert) {};

/**
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.readInt8 = function(offset, noAssert) {};

/**
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.readInt16LE = function(offset, noAssert) {};

/**
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.readInt16BE = function(offset, noAssert) {};

/**
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.readInt32LE = function(offset, noAssert) {};

/**
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.readInt32BE = function(offset, noAssert) {};

/**
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.readFloatLE = function(offset, noAssert) {};

/**
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.readFloatBE = function(offset, noAssert) {};

/**
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.readDoubleLE = function(offset, noAssert) {};

/**
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.readDoubleBE = function(offset, noAssert) {};

/**
 * @param {number} value
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.writeUInt8 = function(value, offset, noAssert) {};

/**
 * @param {number} value
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.writeUInt16LE = function(value, offset, noAssert) {};

/**
 * @param {number} value
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.writeUInt16BE = function(value, offset, noAssert) {};

/**
 * @param {number} value
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.writeUInt32LE = function(value, offset, noAssert) {};

/**
 * @param {number} value
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.writeUInt32BE = function(value, offset, noAssert) {};

/**
 * @param {number} value
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.writeInt8 = function(value, offset, noAssert) {};

/**
 * @param {number} value
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.writeInt16LE = function(value, offset, noAssert) {};

/**
 * @param {number} value
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.writeInt16BE = function(value, offset, noAssert) {};

/**
 * @param {number} value
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.writeInt32LE = function(value, offset, noAssert) {};

/**
 * @param {number} value
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.writeInt32BE = function(value, offset, noAssert) {};

/**
 * @param {number} value
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.writeFloatLE = function(value, offset, noAssert) {};

/**
 * @param {number} value
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.writeFloatBE = function(value, offset, noAssert) {};

/**
 * @param {number} value
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.writeDoubleLE = function(value, offset, noAssert) {};

/**
 * @param {number} value
 * @param {number} offset
 * @param {boolean=} noAssert
 * @return {number}
 */
buffer.Buffer.prototype.writeDoubleBE = function(value, offset, noAssert) {};

/**
 * @param {*} value
 * @param {number=} offset
 * @param {number=} end
 */
buffer.Buffer.prototype.fill = function(value, offset, end) {};

/**
 * @param {string=} encoding
 * @param {number=} start
 * @param {number=} end
 * @nosideeffects
 */
buffer.Buffer.prototype.toString = function(encoding, start, end) {};

/**
 * @type {number}
 */
buffer.Buffer.INSPECT_MAX_BYTES = 50;

/**
 * @param {number} size
 */
buffer.SlowBuffer = function(size) {};

/**
 * 
 * @param {string} string
 * @param {number|string} offset
 * @param {number|string=} length
 * @param {number|string=} encoding
 * @return {*}
 */
buffer.SlowBuffer.prototype.write = function(string, offset, length, encoding) {};

/**
 * @param {number} start
 * @param {number} end
 * @return {buffer.Buffer}
 */
buffer.SlowBuffer.prototype.slice = function(start, end) {};

/**
 * @return {string}
 */
buffer.SlowBuffer.prototype.toString = function() {};

//
// Legacy
//

/**
 * @param {number=} start
 * @param {number=} end
 * @return {buffer.Buffer}
 */
buffer.Buffer.prototype.utf8Slice = function(start, end) {};

/**
 * @param {number=} start
 * @param {number=} end
 * @return {buffer.Buffer}
 */
buffer.Buffer.prototype.binarySlice = function(start, end) {};

/**
 * @param {number=} start
 * @param {number=} end
 * @return {buffer.Buffer}
 */
buffer.Buffer.prototype.asciiSlice = function(start, end) {};

/**
 * @param {string} string
 * @param {number=} offset
 * @return {buffer.Buffer}
 */
buffer.Buffer.prototype.utf8Write = function(string, offset) {};

/**
 * @param {string} string
 * @param {number=} offset
 * @return {buffer.Buffer}
 */
buffer.Buffer.prototype.binaryWrite = function(string, offset) {};

/**
 * @param {string} string
 * @param {number=} offset
 * @return {buffer.Buffer}
 */
buffer.Buffer.prototype.asciiWrite = function(string, offset) {};
