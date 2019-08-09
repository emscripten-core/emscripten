/**
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * This file contains definitions for things that we'd really rather the closure compiler *didn't* minify.
 * See http://code.google.com/p/closure-compiler/wiki/FAQ#How_do_I_write_an_externs_file
 * See also the discussion here: https://github.com/emscripten-core/emscripten/issues/1979
 *
 * The closure_compiler() method in tools/shared.py refers to this file when calling closure.
 */

// Closure externs used by library_sockfs.js

/**
 * Backported from latest closure...
 * @see https://developer.mozilla.org/en-US/docs/Web/API/Document/currentScript
 */
Document.prototype.currentScript;

/**
 * Don't minify Math.*
 */
/**
 * @suppress {duplicate}
 */
var Math = {};
Math.abs = function() {};
Math.cos = function() {};
Math.sin = function() {};
Math.tan = function() {};
Math.acos = function() {};
Math.asin = function() {};
Math.atan = function() {};
Math.atan2 = function() {};
Math.exp = function() {};
Math.log = function() {};
Math.sqrt = function() {};
Math.ceil = function() {};
Math.floor = function() {};
Math.pow = function() {};
Math.imul = function() {};
Math.fround = function() {};
Math.round = function() {};
Math.min = function() {};
Math.max = function() {};
Math.clz32 = function() {};
Math.trunc = function() {};

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

SIMD.Float32x4.load1 = function() {};
SIMD.Float32x4.load2 = function() {};
SIMD.Float32x4.load3 = function() {};
SIMD.Float32x4.load4 = function() {};
SIMD.Float32x4.store1 = function() {};
SIMD.Float32x4.store2 = function() {};
SIMD.Float32x4.store3 = function() {};
SIMD.Float32x4.store4 = function() {};

SIMD.Int32x4.load1 = function() {};
SIMD.Int32x4.load2 = function() {};
SIMD.Int32x4.load3 = function() {};
SIMD.Int32x4.load4 = function() {};
SIMD.Int32x4.store1 = function() {};
SIMD.Int32x4.store2 = function() {};
SIMD.Int32x4.store3 = function() {};
SIMD.Int32x4.store4 = function() {};

SIMD.Uint32x4.load1 = function() {};
SIMD.Uint32x4.load2 = function() {};
SIMD.Uint32x4.load3 = function() {};
SIMD.Uint32x4.load4 = function() {};
SIMD.Uint32x4.store1 = function() {};
SIMD.Uint32x4.store2 = function() {};
SIMD.Uint32x4.store3 = function() {};
SIMD.Uint32x4.store4 = function() {};

SIMD.bool64x2.anyTrue = function() {};
SIMD.bool32x4.anyTrue = function() {};
SIMD.bool16x8.anyTrue = function() {};
SIMD.bool8x16.anyTrue = function() {};

SIMD.Float32x4.fromBool64x2Bits = function() {};
SIMD.Float64x2.fromBool64x2Bits = function() {};
SIMD.Int8x16.fromBool64x2Bits = function() {};
SIMD.Int16x8.fromBool64x2Bits = function() {};
SIMD.Int32x4.fromBool64x2Bits = function() {};
SIMD.Uint8x16.fromBool64x2Bits = function() {};
SIMD.Uint16x8.fromBool64x2Bits = function() {};
SIMD.Uint32x4.fromBool64x2Bits = function() {};
SIMD.Bool8x16.fromBool64x2Bits = function() {};
SIMD.Bool16x8.fromBool64x2Bits = function() {};
SIMD.Bool32x4.fromBool64x2Bits = function() {};
SIMD.Bool64x2.fromBool64x2Bits = function() {};

SIMD.Float32x4.fromFloat64x2 = function() {};
SIMD.Float64x2.fromFloat64x2 = function() {};
SIMD.Int8x16.fromFloat64x2 = function() {};
SIMD.Int16x8.fromFloat64x2 = function() {};
SIMD.Int32x4.fromFloat64x2 = function() {};
SIMD.Uint8x16.fromFloat64x2 = function() {};
SIMD.Uint16x8.fromFloat64x2 = function() {};
SIMD.Uint32x4.fromFloat64x2 = function() {};
SIMD.Bool8x16.fromFloat64x2 = function() {};
SIMD.Bool16x8.fromFloat64x2 = function() {};
SIMD.Bool32x4.fromFloat64x2 = function() {};
SIMD.Bool64x2.fromFloat64x2 = function() {};

/**
 * @const
 * @suppress {duplicate}
 */
var WebAssembly = {};
/**
 * @constructor
 * @param {!BufferSource} bytes
 */
WebAssembly.Module = function(bytes) {};
/**
 * @constructor
 * @param {!WebAssembly.Module} moduleObject
 * @param {Object=} importObject
 */
WebAssembly.Instance = function(moduleObject, importObject) {};
/** @typedef {{initial:number, maximum:(number|undefined)}} */
var MemoryDescriptor;
/**
 * @constructor
 * @param {MemoryDescriptor} memoryDescriptor
 */
WebAssembly.Memory = function(memoryDescriptor) {};
/** @typedef {{element:string, initial:number, maximum:(number|undefined)}} */
var TableDescriptor;
/**
 * @constructor
 * @param {TableDescriptor} tableDescriptor
 */
WebAssembly.Table = function(tableDescriptor) {};
/**
 * @constructor
 * @extends {Error}
 */
WebAssembly.CompileError = function() {};
/**
 * @constructor
 * @extends {Error}
 */
WebAssembly.LinkError = function() {};
/**
 * @constructor
 * @extends {Error}
 */
WebAssembly.RuntimeError = function() {};
/**
 * Note: Closure compiler does not support function overloading, omit this overload for now.
 * {function(!WebAssembly.Module, Object=):!Promise<!WebAssembly.Instance>}
 */
/**
 * @param {!BufferSource} moduleObject
 * @param {Object=} importObject
 * @return {!Promise<{module:WebAssembly.Module, instance:WebAssembly.Instance}>}
 */
WebAssembly.instantiate = function(moduleObject, importObject) {};
/**
 * @param {!Promise<!Response>} source
 * @param {Object=} importObject
 * @return {!Promise<{module:WebAssembly.Module, instance:WebAssembly.Instance}>}
 */
WebAssembly.instantiateStreaming = function(source, importObject) {};
/**
 * @param {!BufferSource} bytes
 * @return {!Promise<!WebAssembly.Module>}
 */
WebAssembly.compile = function(bytes) {};
/**
 * @param {!BufferSource} bytes
 * @return {boolean}
 */
WebAssembly.validate = function(bytes) {};
/**
 * @param {!WebAssembly.Module} moduleObject
 * @return {!Array<{name:string, kind:string}>}
 */
WebAssembly.Module.exports = function(moduleObject) {};
/**
 * @param {!WebAssembly.Module} moduleObject
 * @return {!Array<{module:string, name:string, kind:string}>}
 */
WebAssembly.Module.imports = function(moduleObject) {};
/**
 * @param {!WebAssembly.Module} moduleObject
 * @param {string} sectionName
 * @return {!Array<!ArrayBuffer>}
 */
WebAssembly.Module.customSections = function(moduleObject, sectionName) {};
/** @dict */
WebAssembly.Instance.prototype.exports;
/**
 * @param {number} delta
 * @return {number}
 */
WebAssembly.Memory.prototype.grow = function(delta) {};
/**
 * @type {!ArrayBuffer}
 */
WebAssembly.Memory.prototype.buffer;
/**
 * @param {number} delta
 * @return {number}
 */
WebAssembly.Table.prototype.grow = function(delta) {};
/**
 * @type {number}
 */
WebAssembly.Table.prototype.length;
/**
 * @param {number} index
 * @return {function(...)}
 */
WebAssembly.Table.prototype.get = function(index) {};
/**
 * @param {number} index
 * @param {?function(...)} value
 */
WebAssembly.Table.prototype.set = function(index, value) {};

/**
 * @suppress {undefinedVars}
 */
var wakaUnknownAfter;
/**
 * @suppress {undefinedVars}
 */
var wakaUnknownBefore;
/**
 * @suppress {undefinedVars}
 */
var MozBlobBuilder;

// Module loaders externs, for AMD etc.

/**
 * @param {Function} wrapper
 */
var define = function (wrapper) {};

/**
 * @type {Worker}
 */
var worker;

/**
 * @param {Object} message
 */
var onmessage = function(message) {};

/**
 * @param {string} type
 * @param {!Function} listener
 */
var addEventListener = function (type, listener) {};

// Fetch.js/Fetch Worker

/**
 * @suppress {undefinedVars}
 */
var ENVIRONMENT_IS_FETCH_WORKER;

// Due to the way MODULARIZE works, Closure is run on generated code that does not define _scriptDir,
// but only after MODULARIZE has finished, _scriptDir is injected to the generated code.
// Therefore it cannot be minified.
/**
 * @suppress {duplicate, undefinedVars}
 */
var _scriptDir;

// Closure run on asm.js uses a hack to execute only on shell code, declare externs needed for it.
/**
 * @suppress {undefinedVars}
 */
var wakaGlobal;
/**
 * @suppress {undefinedVars}
 */
var wakaEnv;
/**
 * @suppress {undefinedVars}
 */
var wakaBuffer;


// Browser externs on global window object.
var pageXOffset;
var pageYOffset;
var innerWidth;
var innerHeight;
var outerWidth;
var outerHeight;
var event;
var devicePixelRatio;
