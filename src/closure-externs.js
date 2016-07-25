/**
 * This file contains definitions for things that we'd really rather the closure compiler *didn't* minify.
 * See http://code.google.com/p/closure-compiler/wiki/FAQ#How_do_I_write_an_externs_file
 * See also the discussion here: https://github.com/kripken/emscripten/issues/1979
 *
 * The closure_compiler() method in tools/shared.py refers to this file when calling closure.
 */

// Closure externs used by library_uuid.js

/**
 * @param {Array} typedArray
 */
crypto.getRandomValues = function(typedArray) {};

/**
 BEGIN_NODE_INCLUDE
 var crypto = require('crypto');
 END_NODE_INCLUDE
 */

/**
 * @type {Object.<string,*>}
 */
var crypto = {};

/**
 * @param {number} size
 * @param {function(Error, buffer.Buffer)} callback
 */
crypto.randomBytes = function(size, callback) {};


// Closure externs used by library_sockfs.js

/**
 BEGIN_NODE_INCLUDE
 var ws = require('ws');
 END_NODE_INCLUDE
 */

/**
 * @type {Object.<string,*>}
 */
var ws = {};

/**
 * @param {string} event
 * @param {function()} callback
 */
ws.on = function(event, callback) {};

/**
 * @param {Object} data
 * @param {Object} flags
 * @param {function()=} callback
 */
ws.send = function(data, flags, callback) {};

/**
* @type {boolean}
*/
ws.binaryType;

/**
 * @type {Object.<string,*>}
 */
var wss = ws.Server;

/**
 * @param {string} event
 * @param {function()} callback
 */
wss.on = function(event, callback) {};

/**
 * @param {function()} callback
 */
wss.broadcast = function(callback) {};

/**
* @type {Object.<string,*>}
*/
wss._socket;

/**
* @type {string}
*/
wss.url;

/**
* @type {string}
*/
wss._socket.remoteAddress;

/**
* @type {number}
*/
wss._socket.remotePort;

/**
* @type {Object.<string,*>}
*/
var flags = {};
/**
* @type {boolean}
*/
flags.binary;


/**
 * @fileoverview Definitions for W3C's Gamepad specification.
 * @see http://www.w3.org/TR/gamepad/
 * @externs
 */

/**
 * @typedef {{id: string, index: number, timestamp: number, axes: Array.<number>, buttons: Array.<number>}}
 */
var Gamepad;

/**
* @type {Array.<number>}
*/
Gamepad.buttons;

/**
* @type {Array.<number>}
*/
Gamepad.axes;

/**
* @type {number}
*/
Gamepad.index;

/**
* @type {string}
*/
Gamepad.id;

/**
* @type {number}
*/
Gamepad.timestamp;

/**
 * @return {Array.<Gamepad>}
 */
navigator.getGamepads = function() {};

/**
 * @return {Array.<Gamepad>}
 */
navigator.webkitGetGamepads = function() {};

/**
 * @return {Array.<Gamepad>}
 */
navigator.webkitGamepads = function() {};

/**
 * @return {Array.<Gamepad>}
 */
navigator.mozGamepads = function() {};

/**
 * @return {Array.<Gamepad>}
 */
navigator.gamepads = function() {};

/**
 * Backported from latest closure...
 * @see https://developer.mozilla.org/en-US/docs/Web/API/Document/currentScript
 */
Document.prototype.currentScript;

//Atomics library (not yet in latest closure):
//See https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Atomics
var Atomics;
Atomics.prototype.NOTEQUAL = -1;
Atomics.prototype.OK = 0;
Atomics.prototype.TIMEDOUT = -2;
Atomics.prototype.add = function(typedArray, index, value) {};
Atomics.prototype.and = function(typedArray, index, value) {};
Atomics.prototype.compareExchange = function(typedArray, index, expectedValue, replacementValue) {};
Atomics.prototype.exchange = function(typedArray, index, value) {};
Atomics.prototype.load = function(typedArray, index) {};
Atomics.prototype.or = function(typedArray, index, value) {};
Atomics.prototype.store = function(typedArray, index, value) {};
Atomics.prototype.sub = function(typedArray, index, value) {};
Atomics.prototype.xor = function(typedArray, index, value) {};
Atomics.prototype.wait = function(typedArray, index, valuei, timeout) {};
Atomics.prototype.wake = function(typedArray, index, value) {};
Atomics.prototype.isLockFree = function(size) {};

/**
 * SIMD.js support (not in upstream closure yet).
 */
var SIMD;
//Base objects
SIMD.Float32x4;
SIMD.Float64x2;
SIMD.Int8x16;
SIMD.Int16x8;
SIMD.Int32x4;
SIMD.Uint8x16;
SIMD.Uint16x8;
SIMD.Uint32x4;
SIMD.Bool8x16;
SIMD.Bool16x8;
SIMD.Bool32x4;
SIMD.Bool64x2;

SIMD.Float32x4.abs = function() {};
SIMD.Float64x2.abs = function() {};
SIMD.Int8x16.abs = function() {};
SIMD.Int16x8.abs = function() {};
SIMD.Int32x4.abs = function() {};
SIMD.Uint8x16.abs = function() {};
SIMD.Uint16x8.abs = function() {};
SIMD.Uint32x4.abs = function() {};
SIMD.Bool8x16.abs = function() {};
SIMD.Bool16x8.abs = function() {};
SIMD.Bool32x4.abs = function() {};
SIMD.Bool64x2.abs = function() {};

SIMD.Float32x4.add = function() {};
SIMD.Float64x2.add = function() {};
SIMD.Int8x16.add = function() {};
SIMD.Int16x8.add = function() {};
SIMD.Int32x4.add = function() {};
SIMD.Uint8x16.add = function() {};
SIMD.Uint16x8.add = function() {};
SIMD.Uint32x4.add = function() {};
SIMD.Bool8x16.add = function() {};
SIMD.Bool16x8.add = function() {};
SIMD.Bool32x4.add = function() {};
SIMD.Bool64x2.add = function() {};

SIMD.Float32x4.addSaturate = function() {};
SIMD.Float64x2.addSaturate = function() {};
SIMD.Int8x16.addSaturate = function() {};
SIMD.Int16x8.addSaturate = function() {};
SIMD.Int32x4.addSaturate = function() {};
SIMD.Uint8x16.addSaturate = function() {};
SIMD.Uint16x8.addSaturate = function() {};
SIMD.Uint32x4.addSaturate = function() {};
SIMD.Bool8x16.addSaturate = function() {};
SIMD.Bool16x8.addSaturate = function() {};
SIMD.Bool32x4.addSaturate = function() {};
SIMD.Bool64x2.addSaturate = function() {};

SIMD.Float32x4.allTrue = function() {};
SIMD.Float64x2.allTrue = function() {};
SIMD.Int8x16.allTrue = function() {};
SIMD.Int16x8.allTrue = function() {};
SIMD.Int32x4.allTrue = function() {};
SIMD.Uint8x16.allTrue = function() {};
SIMD.Uint16x8.allTrue = function() {};
SIMD.Uint32x4.allTrue = function() {};
SIMD.Bool8x16.allTrue = function() {};
SIMD.Bool16x8.allTrue = function() {};
SIMD.Bool32x4.allTrue = function() {};
SIMD.Bool64x2.allTrue = function() {};

SIMD.Float32x4.and = function() {};
SIMD.Float64x2.and = function() {};
SIMD.Int8x16.and = function() {};
SIMD.Int16x8.and = function() {};
SIMD.Int32x4.and = function() {};
SIMD.Uint8x16.and = function() {};
SIMD.Uint16x8.and = function() {};
SIMD.Uint32x4.and = function() {};
SIMD.Bool8x16.and = function() {};
SIMD.Bool16x8.and = function() {};
SIMD.Bool32x4.and = function() {};
SIMD.Bool64x2.and = function() {};

SIMD.Float32x4.check = function() {};
SIMD.Float64x2.check = function() {};
SIMD.Int8x16.check = function() {};
SIMD.Int16x8.check = function() {};
SIMD.Int32x4.check = function() {};
SIMD.Uint8x16.check = function() {};
SIMD.Uint16x8.check = function() {};
SIMD.Uint32x4.check = function() {};
SIMD.Bool8x16.check = function() {};
SIMD.Bool16x8.check = function() {};
SIMD.Bool32x4.check = function() {};
SIMD.Bool64x2.check = function() {};

SIMD.Float32x4.div = function() {};
SIMD.Float64x2.div = function() {};
SIMD.Int8x16.div = function() {};
SIMD.Int16x8.div = function() {};
SIMD.Int32x4.div = function() {};
SIMD.Uint8x16.div = function() {};
SIMD.Uint16x8.div = function() {};
SIMD.Uint32x4.div = function() {};
SIMD.Bool8x16.div = function() {};
SIMD.Bool16x8.div = function() {};
SIMD.Bool32x4.div = function() {};
SIMD.Bool64x2.div = function() {};

SIMD.Float32x4.equal = function() {};
SIMD.Float64x2.equal = function() {};
SIMD.Int8x16.equal = function() {};
SIMD.Int16x8.equal = function() {};
SIMD.Int32x4.equal = function() {};
SIMD.Uint8x16.equal = function() {};
SIMD.Uint16x8.equal = function() {};
SIMD.Uint32x4.equal = function() {};
SIMD.Bool8x16.equal = function() {};
SIMD.Bool16x8.equal = function() {};
SIMD.Bool32x4.equal = function() {};
SIMD.Bool64x2.equal = function() {};

SIMD.Float32x4.extractLane = function() {};
SIMD.Float64x2.extractLane = function() {};
SIMD.Int8x16.extractLane = function() {};
SIMD.Int16x8.extractLane = function() {};
SIMD.Int32x4.extractLane = function() {};
SIMD.Uint8x16.extractLane = function() {};
SIMD.Uint16x8.extractLane = function() {};
SIMD.Uint32x4.extractLane = function() {};
SIMD.Bool8x16.extractLane = function() {};
SIMD.Bool16x8.extractLane = function() {};
SIMD.Bool32x4.extractLane = function() {};
SIMD.Bool64x2.extractLane = function() {};

SIMD.Float32x4.fromFloat32x4 = function() {};
SIMD.Float64x2.fromFloat32x4 = function() {};
SIMD.Int8x16.fromFloat32x4 = function() {};
SIMD.Int16x8.fromFloat32x4 = function() {};
SIMD.Int32x4.fromFloat32x4 = function() {};
SIMD.Uint8x16.fromFloat32x4 = function() {};
SIMD.Uint16x8.fromFloat32x4 = function() {};
SIMD.Uint32x4.fromFloat32x4 = function() {};
SIMD.Bool8x16.fromFloat32x4 = function() {};
SIMD.Bool16x8.fromFloat32x4 = function() {};
SIMD.Bool32x4.fromFloat32x4 = function() {};
SIMD.Bool64x2.fromFloat32x4 = function() {};

SIMD.Float32x4.fromFloat32x4Bits = function() {};
SIMD.Float64x2.fromFloat32x4Bits = function() {};
SIMD.Int8x16.fromFloat32x4Bits = function() {};
SIMD.Int16x8.fromFloat32x4Bits = function() {};
SIMD.Int32x4.fromFloat32x4Bits = function() {};
SIMD.Uint8x16.fromFloat32x4Bits = function() {};
SIMD.Uint16x8.fromFloat32x4Bits = function() {};
SIMD.Uint32x4.fromFloat32x4Bits = function() {};
SIMD.Bool8x16.fromFloat32x4Bits = function() {};
SIMD.Bool16x8.fromFloat32x4Bits = function() {};
SIMD.Bool32x4.fromFloat32x4Bits = function() {};
SIMD.Bool64x2.fromFloat32x4Bits = function() {};

SIMD.Float32x4.fromFloat64x2Bits = function() {};
SIMD.Float64x2.fromFloat64x2Bits = function() {};
SIMD.Int8x16.fromFloat64x2Bits = function() {};
SIMD.Int16x8.fromFloat64x2Bits = function() {};
SIMD.Int32x4.fromFloat64x2Bits = function() {};
SIMD.Uint8x16.fromFloat64x2Bits = function() {};
SIMD.Uint16x8.fromFloat64x2Bits = function() {};
SIMD.Uint32x4.fromFloat64x2Bits = function() {};
SIMD.Bool8x16.fromFloat64x2Bits = function() {};
SIMD.Bool16x8.fromFloat64x2Bits = function() {};
SIMD.Bool32x4.fromFloat64x2Bits = function() {};
SIMD.Bool64x2.fromFloat64x2Bits = function() {};

SIMD.Float32x4.fromInt16x8Bits = function() {};
SIMD.Float64x2.fromInt16x8Bits = function() {};
SIMD.Int8x16.fromInt16x8Bits = function() {};
SIMD.Int16x8.fromInt16x8Bits = function() {};
SIMD.Int32x4.fromInt16x8Bits = function() {};
SIMD.Uint8x16.fromInt16x8Bits = function() {};
SIMD.Uint16x8.fromInt16x8Bits = function() {};
SIMD.Uint32x4.fromInt16x8Bits = function() {};
SIMD.Bool8x16.fromInt16x8Bits = function() {};
SIMD.Bool16x8.fromInt16x8Bits = function() {};
SIMD.Bool32x4.fromInt16x8Bits = function() {};
SIMD.Bool64x2.fromInt16x8Bits = function() {};

SIMD.Float32x4.fromInt32x4 = function() {};
SIMD.Float64x2.fromInt32x4 = function() {};
SIMD.Int8x16.fromInt32x4 = function() {};
SIMD.Int16x8.fromInt32x4 = function() {};
SIMD.Int32x4.fromInt32x4 = function() {};
SIMD.Uint8x16.fromInt32x4 = function() {};
SIMD.Uint16x8.fromInt32x4 = function() {};
SIMD.Uint32x4.fromInt32x4 = function() {};
SIMD.Bool8x16.fromInt32x4 = function() {};
SIMD.Bool16x8.fromInt32x4 = function() {};
SIMD.Bool32x4.fromInt32x4 = function() {};
SIMD.Bool64x2.fromInt32x4 = function() {};

SIMD.Float32x4.fromInt32x4Bits = function() {};
SIMD.Float64x2.fromInt32x4Bits = function() {};
SIMD.Int8x16.fromInt32x4Bits = function() {};
SIMD.Int16x8.fromInt32x4Bits = function() {};
SIMD.Int32x4.fromInt32x4Bits = function() {};
SIMD.Uint8x16.fromInt32x4Bits = function() {};
SIMD.Uint16x8.fromInt32x4Bits = function() {};
SIMD.Uint32x4.fromInt32x4Bits = function() {};
SIMD.Bool8x16.fromInt32x4Bits = function() {};
SIMD.Bool16x8.fromInt32x4Bits = function() {};
SIMD.Bool32x4.fromInt32x4Bits = function() {};
SIMD.Bool64x2.fromInt32x4Bits = function() {};

SIMD.Float32x4.fromInt8x16Bits = function() {};
SIMD.Float64x2.fromInt8x16Bits = function() {};
SIMD.Int8x16.fromInt8x16Bits = function() {};
SIMD.Int16x8.fromInt8x16Bits = function() {};
SIMD.Int32x4.fromInt8x16Bits = function() {};
SIMD.Uint8x16.fromInt8x16Bits = function() {};
SIMD.Uint16x8.fromInt8x16Bits = function() {};
SIMD.Uint32x4.fromInt8x16Bits = function() {};
SIMD.Bool8x16.fromInt8x16Bits = function() {};
SIMD.Bool16x8.fromInt8x16Bits = function() {};
SIMD.Bool32x4.fromInt8x16Bits = function() {};
SIMD.Bool64x2.fromInt8x16Bits = function() {};

SIMD.Float32x4.fromUint16x8Bits = function() {};
SIMD.Float64x2.fromUint16x8Bits = function() {};
SIMD.Int8x16.fromUint16x8Bits = function() {};
SIMD.Int16x8.fromUint16x8Bits = function() {};
SIMD.Int32x4.fromUint16x8Bits = function() {};
SIMD.Uint8x16.fromUint16x8Bits = function() {};
SIMD.Uint16x8.fromUint16x8Bits = function() {};
SIMD.Uint32x4.fromUint16x8Bits = function() {};
SIMD.Bool8x16.fromUint16x8Bits = function() {};
SIMD.Bool16x8.fromUint16x8Bits = function() {};
SIMD.Bool32x4.fromUint16x8Bits = function() {};
SIMD.Bool64x2.fromUint16x8Bits = function() {};

SIMD.Float32x4.fromUint32x4 = function() {};
SIMD.Float64x2.fromUint32x4 = function() {};
SIMD.Int8x16.fromUint32x4 = function() {};
SIMD.Int16x8.fromUint32x4 = function() {};
SIMD.Int32x4.fromUint32x4 = function() {};
SIMD.Uint8x16.fromUint32x4 = function() {};
SIMD.Uint16x8.fromUint32x4 = function() {};
SIMD.Uint32x4.fromUint32x4 = function() {};
SIMD.Bool8x16.fromUint32x4 = function() {};
SIMD.Bool16x8.fromUint32x4 = function() {};
SIMD.Bool32x4.fromUint32x4 = function() {};
SIMD.Bool64x2.fromUint32x4 = function() {};

SIMD.Float32x4.fromUint32x4Bits = function() {};
SIMD.Float64x2.fromUint32x4Bits = function() {};
SIMD.Int8x16.fromUint32x4Bits = function() {};
SIMD.Int16x8.fromUint32x4Bits = function() {};
SIMD.Int32x4.fromUint32x4Bits = function() {};
SIMD.Uint8x16.fromUint32x4Bits = function() {};
SIMD.Uint16x8.fromUint32x4Bits = function() {};
SIMD.Uint32x4.fromUint32x4Bits = function() {};
SIMD.Bool8x16.fromUint32x4Bits = function() {};
SIMD.Bool16x8.fromUint32x4Bits = function() {};
SIMD.Bool32x4.fromUint32x4Bits = function() {};
SIMD.Bool64x2.fromUint32x4Bits = function() {};

SIMD.Float32x4.fromUint8x16Bits = function() {};
SIMD.Float64x2.fromUint8x16Bits = function() {};
SIMD.Int8x16.fromUint8x16Bits = function() {};
SIMD.Int16x8.fromUint8x16Bits = function() {};
SIMD.Int32x4.fromUint8x16Bits = function() {};
SIMD.Uint8x16.fromUint8x16Bits = function() {};
SIMD.Uint16x8.fromUint8x16Bits = function() {};
SIMD.Uint32x4.fromUint8x16Bits = function() {};
SIMD.Bool8x16.fromUint8x16Bits = function() {};
SIMD.Bool16x8.fromUint8x16Bits = function() {};
SIMD.Bool32x4.fromUint8x16Bits = function() {};
SIMD.Bool64x2.fromUint8x16Bits = function() {};

SIMD.Float32x4.greaterThan = function() {};
SIMD.Float64x2.greaterThan = function() {};
SIMD.Int8x16.greaterThan = function() {};
SIMD.Int16x8.greaterThan = function() {};
SIMD.Int32x4.greaterThan = function() {};
SIMD.Uint8x16.greaterThan = function() {};
SIMD.Uint16x8.greaterThan = function() {};
SIMD.Uint32x4.greaterThan = function() {};
SIMD.Bool8x16.greaterThan = function() {};
SIMD.Bool16x8.greaterThan = function() {};
SIMD.Bool32x4.greaterThan = function() {};
SIMD.Bool64x2.greaterThan = function() {};

SIMD.Float32x4.greaterThanOrEqual = function() {};
SIMD.Float64x2.greaterThanOrEqual = function() {};
SIMD.Int8x16.greaterThanOrEqual = function() {};
SIMD.Int16x8.greaterThanOrEqual = function() {};
SIMD.Int32x4.greaterThanOrEqual = function() {};
SIMD.Uint8x16.greaterThanOrEqual = function() {};
SIMD.Uint16x8.greaterThanOrEqual = function() {};
SIMD.Uint32x4.greaterThanOrEqual = function() {};
SIMD.Bool8x16.greaterThanOrEqual = function() {};
SIMD.Bool16x8.greaterThanOrEqual = function() {};
SIMD.Bool32x4.greaterThanOrEqual = function() {};
SIMD.Bool64x2.greaterThanOrEqual = function() {};

SIMD.Float32x4.lessThan = function() {};
SIMD.Float64x2.lessThan = function() {};
SIMD.Int8x16.lessThan = function() {};
SIMD.Int16x8.lessThan = function() {};
SIMD.Int32x4.lessThan = function() {};
SIMD.Uint8x16.lessThan = function() {};
SIMD.Uint16x8.lessThan = function() {};
SIMD.Uint32x4.lessThan = function() {};
SIMD.Bool8x16.lessThan = function() {};
SIMD.Bool16x8.lessThan = function() {};
SIMD.Bool32x4.lessThan = function() {};
SIMD.Bool64x2.lessThan = function() {};

SIMD.Float32x4.lessThanOrEqual = function() {};
SIMD.Float64x2.lessThanOrEqual = function() {};
SIMD.Int8x16.lessThanOrEqual = function() {};
SIMD.Int16x8.lessThanOrEqual = function() {};
SIMD.Int32x4.lessThanOrEqual = function() {};
SIMD.Uint8x16.lessThanOrEqual = function() {};
SIMD.Uint16x8.lessThanOrEqual = function() {};
SIMD.Uint32x4.lessThanOrEqual = function() {};
SIMD.Bool8x16.lessThanOrEqual = function() {};
SIMD.Bool16x8.lessThanOrEqual = function() {};
SIMD.Bool32x4.lessThanOrEqual = function() {};
SIMD.Bool64x2.lessThanOrEqual = function() {};

SIMD.Float32x4.load = function() {};
SIMD.Float64x2.load = function() {};
SIMD.Int8x16.load = function() {};
SIMD.Int16x8.load = function() {};
SIMD.Int32x4.load = function() {};
SIMD.Uint8x16.load = function() {};
SIMD.Uint16x8.load = function() {};
SIMD.Uint32x4.load = function() {};
SIMD.Bool8x16.load = function() {};
SIMD.Bool16x8.load = function() {};
SIMD.Bool32x4.load = function() {};
SIMD.Bool64x2.load = function() {};

SIMD.Float32x4.max = function() {};
SIMD.Float64x2.max = function() {};
SIMD.Int8x16.max = function() {};
SIMD.Int16x8.max = function() {};
SIMD.Int32x4.max = function() {};
SIMD.Uint8x16.max = function() {};
SIMD.Uint16x8.max = function() {};
SIMD.Uint32x4.max = function() {};
SIMD.Bool8x16.max = function() {};
SIMD.Bool16x8.max = function() {};
SIMD.Bool32x4.max = function() {};
SIMD.Bool64x2.max = function() {};

SIMD.Float32x4.maxNum = function() {};
SIMD.Float64x2.maxNum = function() {};
SIMD.Int8x16.maxNum = function() {};
SIMD.Int16x8.maxNum = function() {};
SIMD.Int32x4.maxNum = function() {};
SIMD.Uint8x16.maxNum = function() {};
SIMD.Uint16x8.maxNum = function() {};
SIMD.Uint32x4.maxNum = function() {};
SIMD.Bool8x16.maxNum = function() {};
SIMD.Bool16x8.maxNum = function() {};
SIMD.Bool32x4.maxNum = function() {};
SIMD.Bool64x2.maxNum = function() {};

SIMD.Float32x4.min = function() {};
SIMD.Float64x2.min = function() {};
SIMD.Int8x16.min = function() {};
SIMD.Int16x8.min = function() {};
SIMD.Int32x4.min = function() {};
SIMD.Uint8x16.min = function() {};
SIMD.Uint16x8.min = function() {};
SIMD.Uint32x4.min = function() {};
SIMD.Bool8x16.min = function() {};
SIMD.Bool16x8.min = function() {};
SIMD.Bool32x4.min = function() {};
SIMD.Bool64x2.min = function() {};

SIMD.Float32x4.minNum = function() {};
SIMD.Float64x2.minNum = function() {};
SIMD.Int8x16.minNum = function() {};
SIMD.Int16x8.minNum = function() {};
SIMD.Int32x4.minNum = function() {};
SIMD.Uint8x16.minNum = function() {};
SIMD.Uint16x8.minNum = function() {};
SIMD.Uint32x4.minNum = function() {};
SIMD.Bool8x16.minNum = function() {};
SIMD.Bool16x8.minNum = function() {};
SIMD.Bool32x4.minNum = function() {};
SIMD.Bool64x2.minNum = function() {};

SIMD.Float32x4.mul = function() {};
SIMD.Float64x2.mul = function() {};
SIMD.Int8x16.mul = function() {};
SIMD.Int16x8.mul = function() {};
SIMD.Int32x4.mul = function() {};
SIMD.Uint8x16.mul = function() {};
SIMD.Uint16x8.mul = function() {};
SIMD.Uint32x4.mul = function() {};
SIMD.Bool8x16.mul = function() {};
SIMD.Bool16x8.mul = function() {};
SIMD.Bool32x4.mul = function() {};
SIMD.Bool64x2.mul = function() {};

SIMD.Float32x4.neg = function() {};
SIMD.Float64x2.neg = function() {};
SIMD.Int8x16.neg = function() {};
SIMD.Int16x8.neg = function() {};
SIMD.Int32x4.neg = function() {};
SIMD.Uint8x16.neg = function() {};
SIMD.Uint16x8.neg = function() {};
SIMD.Uint32x4.neg = function() {};
SIMD.Bool8x16.neg = function() {};
SIMD.Bool16x8.neg = function() {};
SIMD.Bool32x4.neg = function() {};
SIMD.Bool64x2.neg = function() {};

SIMD.Float32x4.not = function() {};
SIMD.Float64x2.not = function() {};
SIMD.Int8x16.not = function() {};
SIMD.Int16x8.not = function() {};
SIMD.Int32x4.not = function() {};
SIMD.Uint8x16.not = function() {};
SIMD.Uint16x8.not = function() {};
SIMD.Uint32x4.not = function() {};
SIMD.Bool8x16.not = function() {};
SIMD.Bool16x8.not = function() {};
SIMD.Bool32x4.not = function() {};
SIMD.Bool64x2.not = function() {};

SIMD.Float32x4.notEqual = function() {};
SIMD.Float64x2.notEqual = function() {};
SIMD.Int8x16.notEqual = function() {};
SIMD.Int16x8.notEqual = function() {};
SIMD.Int32x4.notEqual = function() {};
SIMD.Uint8x16.notEqual = function() {};
SIMD.Uint16x8.notEqual = function() {};
SIMD.Uint32x4.notEqual = function() {};
SIMD.Bool8x16.notEqual = function() {};
SIMD.Bool16x8.notEqual = function() {};
SIMD.Bool32x4.notEqual = function() {};
SIMD.Bool64x2.notEqual = function() {};

SIMD.Float32x4.or = function() {};
SIMD.Float64x2.or = function() {};
SIMD.Int8x16.or = function() {};
SIMD.Int16x8.or = function() {};
SIMD.Int32x4.or = function() {};
SIMD.Uint8x16.or = function() {};
SIMD.Uint16x8.or = function() {};
SIMD.Uint32x4.or = function() {};
SIMD.Bool8x16.or = function() {};
SIMD.Bool16x8.or = function() {};
SIMD.Bool32x4.or = function() {};
SIMD.Bool64x2.or = function() {};

SIMD.Float32x4.prototype.toSource = function() {};
SIMD.Float64x2.prototype.toSource = function() {};
SIMD.Int8x16.prototype.toSource = function() {};
SIMD.Int16x8.prototype.toSource = function() {};
SIMD.Int32x4.prototype.toSource = function() {};
SIMD.Uint8x16.prototype.toSource = function() {};
SIMD.Uint16x8.prototype.toSource = function() {};
SIMD.Uint32x4.prototype.toSource = function() {};
SIMD.Bool8x16.prototype.toSource = function() {};
SIMD.Bool16x8.prototype.toSource = function() {};
SIMD.Bool32x4.prototype.toSource = function() {};
SIMD.Bool64x2.prototype.toSource = function() {};

SIMD.Float32x4.reciprocalApproximation = function() {};
SIMD.Float64x2.reciprocalApproximation = function() {};
SIMD.Int8x16.reciprocalApproximation = function() {};
SIMD.Int16x8.reciprocalApproximation = function() {};
SIMD.Int32x4.reciprocalApproximation = function() {};
SIMD.Uint8x16.reciprocalApproximation = function() {};
SIMD.Uint16x8.reciprocalApproximation = function() {};
SIMD.Uint32x4.reciprocalApproximation = function() {};
SIMD.Bool8x16.reciprocalApproximation = function() {};
SIMD.Bool16x8.reciprocalApproximation = function() {};
SIMD.Bool32x4.reciprocalApproximation = function() {};
SIMD.Bool64x2.reciprocalApproximation = function() {};

SIMD.Float32x4.reciprocalSqrtApproximation = function() {};
SIMD.Float64x2.reciprocalSqrtApproximation = function() {};
SIMD.Int8x16.reciprocalSqrtApproximation = function() {};
SIMD.Int16x8.reciprocalSqrtApproximation = function() {};
SIMD.Int32x4.reciprocalSqrtApproximation = function() {};
SIMD.Uint8x16.reciprocalSqrtApproximation = function() {};
SIMD.Uint16x8.reciprocalSqrtApproximation = function() {};
SIMD.Uint32x4.reciprocalSqrtApproximation = function() {};
SIMD.Bool8x16.reciprocalSqrtApproximation = function() {};
SIMD.Bool16x8.reciprocalSqrtApproximation = function() {};
SIMD.Bool32x4.reciprocalSqrtApproximation = function() {};
SIMD.Bool64x2.reciprocalSqrtApproximation = function() {};

SIMD.Float32x4.replaceLane = function() {};
SIMD.Float64x2.replaceLane = function() {};
SIMD.Int8x16.replaceLane = function() {};
SIMD.Int16x8.replaceLane = function() {};
SIMD.Int32x4.replaceLane = function() {};
SIMD.Uint8x16.replaceLane = function() {};
SIMD.Uint16x8.replaceLane = function() {};
SIMD.Uint32x4.replaceLane = function() {};
SIMD.Bool8x16.replaceLane = function() {};
SIMD.Bool16x8.replaceLane = function() {};
SIMD.Bool32x4.replaceLane = function() {};
SIMD.Bool64x2.replaceLane = function() {};

SIMD.Float32x4.select = function() {};
SIMD.Float64x2.select = function() {};
SIMD.Int8x16.select = function() {};
SIMD.Int16x8.select = function() {};
SIMD.Int32x4.select = function() {};
SIMD.Uint8x16.select = function() {};
SIMD.Uint16x8.select = function() {};
SIMD.Uint32x4.select = function() {};
SIMD.Bool8x16.select = function() {};
SIMD.Bool16x8.select = function() {};
SIMD.Bool32x4.select = function() {};
SIMD.Bool64x2.select = function() {};

SIMD.Float32x4.shiftLeftByScalar = function() {};
SIMD.Float64x2.shiftLeftByScalar = function() {};
SIMD.Int8x16.shiftLeftByScalar = function() {};
SIMD.Int16x8.shiftLeftByScalar = function() {};
SIMD.Int32x4.shiftLeftByScalar = function() {};
SIMD.Uint8x16.shiftLeftByScalar = function() {};
SIMD.Uint16x8.shiftLeftByScalar = function() {};
SIMD.Uint32x4.shiftLeftByScalar = function() {};
SIMD.Bool8x16.shiftLeftByScalar = function() {};
SIMD.Bool16x8.shiftLeftByScalar = function() {};
SIMD.Bool32x4.shiftLeftByScalar = function() {};
SIMD.Bool64x2.shiftLeftByScalar = function() {};

SIMD.Float32x4.shiftRightByScalar = function() {};
SIMD.Float64x2.shiftRightByScalar = function() {};
SIMD.Int8x16.shiftRightByScalar = function() {};
SIMD.Int16x8.shiftRightByScalar = function() {};
SIMD.Int32x4.shiftRightByScalar = function() {};
SIMD.Uint8x16.shiftRightByScalar = function() {};
SIMD.Uint16x8.shiftRightByScalar = function() {};
SIMD.Uint32x4.shiftRightByScalar = function() {};
SIMD.Bool8x16.shiftRightByScalar = function() {};
SIMD.Bool16x8.shiftRightByScalar = function() {};
SIMD.Bool32x4.shiftRightByScalar = function() {};
SIMD.Bool64x2.shiftRightByScalar = function() {};

SIMD.Float32x4.shuffle = function() {};
SIMD.Float64x2.shuffle = function() {};
SIMD.Int8x16.shuffle = function() {};
SIMD.Int16x8.shuffle = function() {};
SIMD.Int32x4.shuffle = function() {};
SIMD.Uint8x16.shuffle = function() {};
SIMD.Uint16x8.shuffle = function() {};
SIMD.Uint32x4.shuffle = function() {};
SIMD.Bool8x16.shuffle = function() {};
SIMD.Bool16x8.shuffle = function() {};
SIMD.Bool32x4.shuffle = function() {};
SIMD.Bool64x2.shuffle = function() {};

SIMD.Float32x4.splat = function() {};
SIMD.Float64x2.splat = function() {};
SIMD.Int8x16.splat = function() {};
SIMD.Int16x8.splat = function() {};
SIMD.Int32x4.splat = function() {};
SIMD.Uint8x16.splat = function() {};
SIMD.Uint16x8.splat = function() {};
SIMD.Uint32x4.splat = function() {};
SIMD.Bool8x16.splat = function() {};
SIMD.Bool16x8.splat = function() {};
SIMD.Bool32x4.splat = function() {};
SIMD.Bool64x2.splat = function() {};

SIMD.Float32x4.sqrt = function() {};
SIMD.Float64x2.sqrt = function() {};
SIMD.Int8x16.sqrt = function() {};
SIMD.Int16x8.sqrt = function() {};
SIMD.Int32x4.sqrt = function() {};
SIMD.Uint8x16.sqrt = function() {};
SIMD.Uint16x8.sqrt = function() {};
SIMD.Uint32x4.sqrt = function() {};
SIMD.Bool8x16.sqrt = function() {};
SIMD.Bool16x8.sqrt = function() {};
SIMD.Bool32x4.sqrt = function() {};
SIMD.Bool64x2.sqrt = function() {};

SIMD.Float32x4.store = function() {};
SIMD.Float64x2.store = function() {};
SIMD.Int8x16.store = function() {};
SIMD.Int16x8.store = function() {};
SIMD.Int32x4.store = function() {};
SIMD.Uint8x16.store = function() {};
SIMD.Uint16x8.store = function() {};
SIMD.Uint32x4.store = function() {};
SIMD.Bool8x16.store = function() {};
SIMD.Bool16x8.store = function() {};
SIMD.Bool32x4.store = function() {};
SIMD.Bool64x2.store = function() {};

SIMD.Float32x4.sub = function() {};
SIMD.Float64x2.sub = function() {};
SIMD.Int8x16.sub = function() {};
SIMD.Int16x8.sub = function() {};
SIMD.Int32x4.sub = function() {};
SIMD.Uint8x16.sub = function() {};
SIMD.Uint16x8.sub = function() {};
SIMD.Uint32x4.sub = function() {};
SIMD.Bool8x16.sub = function() {};
SIMD.Bool16x8.sub = function() {};
SIMD.Bool32x4.sub = function() {};
SIMD.Bool64x2.sub = function() {};

SIMD.Float32x4.subSaturate = function() {};
SIMD.Float64x2.subSaturate = function() {};
SIMD.Int8x16.subSaturate = function() {};
SIMD.Int16x8.subSaturate = function() {};
SIMD.Int32x4.subSaturate = function() {};
SIMD.Uint8x16.subSaturate = function() {};
SIMD.Uint16x8.subSaturate = function() {};
SIMD.Uint32x4.subSaturate = function() {};
SIMD.Bool8x16.subSaturate = function() {};
SIMD.Bool16x8.subSaturate = function() {};
SIMD.Bool32x4.subSaturate = function() {};
SIMD.Bool64x2.subSaturate = function() {};

SIMD.Float32x4.swizzle = function() {};
SIMD.Float64x2.swizzle = function() {};
SIMD.Int8x16.swizzle = function() {};
SIMD.Int16x8.swizzle = function() {};
SIMD.Int32x4.swizzle = function() {};
SIMD.Uint8x16.swizzle = function() {};
SIMD.Uint16x8.swizzle = function() {};
SIMD.Uint32x4.swizzle = function() {};
SIMD.Bool8x16.swizzle = function() {};
SIMD.Bool16x8.swizzle = function() {};
SIMD.Bool32x4.swizzle = function() {};
SIMD.Bool64x2.swizzle = function() {};

SIMD.Float32x4.xor = function() {};
SIMD.Float64x2.xor = function() {};
SIMD.Int8x16.xor = function() {};
SIMD.Int16x8.xor = function() {};
SIMD.Int32x4.xor = function() {};
SIMD.Uint8x16.xor = function() {};
SIMD.Uint16x8.xor = function() {};
SIMD.Uint32x4.xor = function() {};
SIMD.Bool8x16.xor = function() {};
SIMD.Bool16x8.xor = function() {};
SIMD.Bool32x4.xor = function() {};
SIMD.Bool64x2.xor = function() {};
