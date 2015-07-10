/*
  vim: set ts=8 sts=2 et sw=2 tw=79:
  Copyright (C) 2013

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

// A conforming SIMD.js implementation may contain the following deviations to
// normal JS numeric behavior:
//  - Subnormal numbers may or may not be flushed to zero on input or output of
//    any SIMD operation.

// Many of the operations in SIMD.js have semantics which correspond to scalar
// operations in JS, however there are a few differences:
//  - Vector shifts don't mask the shift count.
//  - Conversions from float to int32 throw on error.
//  - Load and store operations throw when out of bounds.

(function(global) {

if (typeof global.SIMD === "undefined") {
  // SIMD module.
  global.SIMD = {};
}

if (typeof module !== "undefined") {
  // For CommonJS modules
  module.exports = global.SIMD;
}

var SIMD = global.SIMD;

// private stuff.
// Temporary buffers for swizzles and bitcasts.
var _f32x4 = new Float32Array(4);
var _f64x2 = new Float64Array(_f32x4.buffer);
var _i32x4 = new Int32Array(_f32x4.buffer);
var _i16x8 = new Int16Array(_f32x4.buffer);
var _i8x16 = new Int8Array(_f32x4.buffer);

var _f32;
var truncatef32;
if (typeof Math.fround !== 'undefined') {
  truncatef32 = Math.fround;
} else {
  _f32 = new Float32Array(1);

  truncatef32 = function(x) {
    _f32[0] = x;
    return _f32[0];
  }
}

// Type checking functions.

function isInt32(o) {
  return (o | 0) === o;
}

function isTypedArray(o) {
  return (o instanceof Int8Array) ||
         (o instanceof Uint8Array) ||
         (o instanceof Uint8ClampedArray) ||
         (o instanceof Int16Array) ||
         (o instanceof Uint16Array) ||
         (o instanceof Int32Array) ||
         (o instanceof Uint32Array) ||
         (o instanceof Float32Array) ||
         (o instanceof Float64Array);
}

function minNum(x, y) {
  return x != x ? y :
         y != y ? x :
         Math.min(x, y);
}

function maxNum(x, y) {
  return x != x ? y :
         y != y ? x :
         Math.max(x, y);
}

function int32FromFloat(x) {
  if (x > -2147483649.0 && x < 2147483648.0)
    return x|0;
  throw new RangeError("Conversion from floating-point to integer failed");
}

function checkLaneIndex(numLanes) {
  return function(lane) {
    if (!isInt32(lane))
      throw new TypeError('lane index must be an int32');
    if (lane < 0 || lane >= numLanes)
      throw new RangeError('lane index must be in bounds');
  }
}

var check2 = checkLaneIndex(2);
var check4 = checkLaneIndex(4);
var check8 = checkLaneIndex(8);
var check16 = checkLaneIndex(16);
var check32 = checkLaneIndex(32);

// Save/Restore utilities for implementing bitwise conversions.

function saveBool32x4(x) {
  x = SIMD.Bool32x4.check(x);
  _i32x4[0] = SIMD.Bool32x4.extractLane(x, 0);
  _i32x4[1] = SIMD.Bool32x4.extractLane(x, 1);
  _i32x4[2] = SIMD.Bool32x4.extractLane(x, 2);
  _i32x4[3] = SIMD.Bool32x4.extractLane(x, 3);
}

function saveBool16x8(x) {
  x = SIMD.Bool16x8.check(x);
  _i16x8[0] = SIMD.Bool16x8.extractLane(x, 0);
  _i16x8[1] = SIMD.Bool16x8.extractLane(x, 1);
  _i16x8[2] = SIMD.Bool16x8.extractLane(x, 2);
  _i16x8[3] = SIMD.Bool16x8.extractLane(x, 3);
  _i16x8[4] = SIMD.Bool16x8.extractLane(x, 4);
  _i16x8[5] = SIMD.Bool16x8.extractLane(x, 5);
  _i16x8[6] = SIMD.Bool16x8.extractLane(x, 6);
  _i16x8[7] = SIMD.Bool16x8.extractLane(x, 7);
}

function saveBool8x16(x) {
  x = SIMD.Bool8x16.check(x);
  _i8x16[0] = SIMD.Bool8x16.extractLane(x, 0);
  _i8x16[1] = SIMD.Bool8x16.extractLane(x, 1);
  _i8x16[2] = SIMD.Bool8x16.extractLane(x, 2);
  _i8x16[3] = SIMD.Bool8x16.extractLane(x, 3);
  _i8x16[4] = SIMD.Bool8x16.extractLane(x, 4);
  _i8x16[5] = SIMD.Bool8x16.extractLane(x, 5);
  _i8x16[6] = SIMD.Bool8x16.extractLane(x, 6);
  _i8x16[7] = SIMD.Bool8x16.extractLane(x, 7);
  _i8x16[8] = SIMD.Bool8x16.extractLane(x, 8);
  _i8x16[9] = SIMD.Bool8x16.extractLane(x, 9);
  _i8x16[10] = SIMD.Bool8x16.extractLane(x, 10);
  _i8x16[11] = SIMD.Bool8x16.extractLane(x, 11);
  _i8x16[12] = SIMD.Bool8x16.extractLane(x, 12);
  _i8x16[13] = SIMD.Bool8x16.extractLane(x, 13);
  _i8x16[14] = SIMD.Bool8x16.extractLane(x, 14);
  _i8x16[15] = SIMD.Bool8x16.extractLane(x, 15);
}

function saveFloat32x4(x) {
  x = SIMD.Float32x4.check(x);
  _f32x4[0] = SIMD.Float32x4.extractLane(x, 0);
  _f32x4[1] = SIMD.Float32x4.extractLane(x, 1);
  _f32x4[2] = SIMD.Float32x4.extractLane(x, 2);
  _f32x4[3] = SIMD.Float32x4.extractLane(x, 3);
}

function saveInt32x4(x) {
  x = SIMD.Int32x4.check(x);
  _i32x4[0] = SIMD.Int32x4.extractLane(x, 0);
  _i32x4[1] = SIMD.Int32x4.extractLane(x, 1);
  _i32x4[2] = SIMD.Int32x4.extractLane(x, 2);
  _i32x4[3] = SIMD.Int32x4.extractLane(x, 3);
}

function saveInt16x8(x) {
  x = SIMD.Int16x8.check(x);
  _i16x8[0] = SIMD.Int16x8.extractLane(x, 0);
  _i16x8[1] = SIMD.Int16x8.extractLane(x, 1);
  _i16x8[2] = SIMD.Int16x8.extractLane(x, 2);
  _i16x8[3] = SIMD.Int16x8.extractLane(x, 3);
  _i16x8[4] = SIMD.Int16x8.extractLane(x, 4);
  _i16x8[5] = SIMD.Int16x8.extractLane(x, 5);
  _i16x8[6] = SIMD.Int16x8.extractLane(x, 6);
  _i16x8[7] = SIMD.Int16x8.extractLane(x, 7);
}

function saveInt8x16(x) {
  x = SIMD.Int8x16.check(x);
  _i8x16[0] = SIMD.Int8x16.extractLane(x, 0);
  _i8x16[1] = SIMD.Int8x16.extractLane(x, 1);
  _i8x16[2] = SIMD.Int8x16.extractLane(x, 2);
  _i8x16[3] = SIMD.Int8x16.extractLane(x, 3);
  _i8x16[4] = SIMD.Int8x16.extractLane(x, 4);
  _i8x16[5] = SIMD.Int8x16.extractLane(x, 5);
  _i8x16[6] = SIMD.Int8x16.extractLane(x, 6);
  _i8x16[7] = SIMD.Int8x16.extractLane(x, 7);
  _i8x16[8] = SIMD.Int8x16.extractLane(x, 8);
  _i8x16[9] = SIMD.Int8x16.extractLane(x, 9);
  _i8x16[10] = SIMD.Int8x16.extractLane(x, 10);
  _i8x16[11] = SIMD.Int8x16.extractLane(x, 11);
  _i8x16[12] = SIMD.Int8x16.extractLane(x, 12);
  _i8x16[13] = SIMD.Int8x16.extractLane(x, 13);
  _i8x16[14] = SIMD.Int8x16.extractLane(x, 14);
  _i8x16[15] = SIMD.Int8x16.extractLane(x, 15);
}

function restoreBool32x4() {
  var alias = _i32x4;
  return SIMD.Bool32x4(alias[0], alias[1], alias[2], alias[3]);
}

function restoreBool16x8() {
  var alias = _i16x8;
  return SIMD.Bool16x8(alias[0], alias[1], alias[2], alias[3],
                       alias[4], alias[5], alias[6], alias[7]);
}

function restoreBool8x16() {
  var alias = _i8x16;
  return SIMD.Bool8x16(alias[0], alias[1], alias[2], alias[3],
                       alias[4], alias[5], alias[6], alias[7],
                       alias[8], alias[9], alias[10], alias[11],
                       alias[12], alias[13], alias[14], alias[15]);
}

function restoreFloat32x4() {
  var alias = _f32x4;
  return SIMD.Float32x4(alias[0], alias[1], alias[2], alias[3]);
}

function restoreInt32x4() {
  var alias = _i32x4;
  return SIMD.Int32x4(alias[0], alias[1], alias[2], alias[3]);
}

function restoreInt16x8() {
  var alias = _i16x8;
  return SIMD.Int16x8(alias[0], alias[1], alias[2], alias[3],
                      alias[4], alias[5], alias[6], alias[7]);
}

function restoreInt8x16() {
  var alias = _i8x16;
  return SIMD.Int8x16(alias[0], alias[1], alias[2], alias[3],
                      alias[4], alias[5], alias[6], alias[7],
                      alias[8], alias[9], alias[10], alias[11],
                      alias[12], alias[13], alias[14], alias[15]);
}


if (typeof SIMD.Bool32x4 === "undefined") {
  /**
    * Construct a new instance of Bool32x4 number.
    * @constructor
    */
  SIMD.Bool32x4 = function(x, y, z, w) {
    if (!(this instanceof SIMD.Bool32x4)) {
      return new SIMD.Bool32x4(x, y, z, w);
    }

    this.x_ = !!x;
    this.y_ = !!y;
    this.z_ = !!z;
    this.w_ = !!w;
  }
}

if (typeof SIMD.Bool32x4.check === "undefined") {
  /**
    * Check whether the argument is a Bool32x4.
    * @param {Bool32x4} v An instance of Bool32x4.
    * @return {Bool32x4} The Bool32x4 instance.
    */
  SIMD.Bool32x4.check = function(v) {
    if (!(v instanceof SIMD.Bool32x4)) {
      throw new TypeError("argument is not a Bool32x4.");
    }
    return v;
  }
}

if (typeof SIMD.Bool32x4.splat === "undefined") {
  /**
    * Construct a new instance of Bool32x4 with the same value
    * in all lanes.
    * @param {double} value used for all lanes.
    * @constructor
    */
  SIMD.Bool32x4.splat = function(s) {
    return SIMD.Bool32x4(s, s, s, s);
  }
}

if (typeof SIMD.Bool32x4.extractLane === "undefined") {
  /**
    * @param {Bool32x4} v An instance of Bool32x4.
    * @param {integer} i Index in concatenation of v for lane i
    * @return {Boolean} The value in lane i of v.
    */
  SIMD.Bool32x4.extractLane = function(v, i) {
    v = SIMD.Bool32x4.check(v);
    check4(i);
    switch(i) {
      case 0: return v.x_;
      case 1: return v.y_;
      case 2: return v.z_;
      case 3: return v.w_;
    }
  }
}

if (typeof SIMD.Bool32x4.replaceLane === "undefined") {
  /**
    * @param {Bool32x4} v An instance of Bool32x4.
    * @param {integer} i Index in concatenation of v for lane i
    * @param {double} value used for lane i.
    * @return {Bool32x4} New instance of Bool32x4 with the values in v and
    * lane i replaced with {s}.
    */
  SIMD.Bool32x4.replaceLane = function(v, i, s) {
    v = SIMD.Bool32x4.check(v);
    check4(i);
    saveBool32x4(v);
    _i32x4[i] = s;
    return restoreBool32x4();
  }
}

if (typeof SIMD.Bool32x4.allTrue === "undefined") {
  /**
    * Check if all 4 lanes hold a true value
    * @param {Bool32x4} v An instance of Bool32x4.
    * @return {Boolean} All 4 lanes holds a true value
    */
  SIMD.Bool32x4.allTrue = function(v) {
    v = SIMD.Bool32x4.check(v);
    return SIMD.Bool32x4.extractLane(v, 0) &&
        SIMD.Bool32x4.extractLane(v, 1) &&
        SIMD.Bool32x4.extractLane(v, 2) &&
        SIMD.Bool32x4.extractLane(v, 3);
  }
}

if (typeof SIMD.Bool32x4.anyTrue === "undefined") {
  /**
    * Check if any of the 4 lanes hold a true value
    * @param {Bool32x4} v An instance of Bool32x4.
    * @return {Boolean} Any of the 4 lanes holds a true value
    */
  SIMD.Bool32x4.anyTrue = function(v) {
    v = SIMD.Bool32x4.check(v);
    return SIMD.Bool32x4.extractLane(v, 0) ||
        SIMD.Bool32x4.extractLane(v, 1) ||
        SIMD.Bool32x4.extractLane(v, 2) ||
        SIMD.Bool32x4.extractLane(v, 3);
  }
}

if (typeof SIMD.Bool32x4.and === "undefined") {
  /**
    * @param {Bool32x4} a An instance of Bool32x4.
    * @param {Bool32x4} b An instance of Bool32x4.
    * @return {Bool32x4} New instance of Bool32x4 with values of a & b.
    */
  SIMD.Bool32x4.and = function(a, b) {
    a = SIMD.Bool32x4.check(a);
    b = SIMD.Bool32x4.check(b);
    return SIMD.Bool32x4(SIMD.Bool32x4.extractLane(a, 0) & SIMD.Bool32x4.extractLane(b, 0),
                         SIMD.Bool32x4.extractLane(a, 1) & SIMD.Bool32x4.extractLane(b, 1),
                         SIMD.Bool32x4.extractLane(a, 2) & SIMD.Bool32x4.extractLane(b, 2),
                         SIMD.Bool32x4.extractLane(a, 3) & SIMD.Bool32x4.extractLane(b, 3));
  }
}

if (typeof SIMD.Bool32x4.or === "undefined") {
  /**
    * @param {Bool32x4} a An instance of Bool32x4.
    * @param {Bool32x4} b An instance of Bool32x4.
    * @return {Bool32x4} New instance of Bool32x4 with values of a | b.
    */
  SIMD.Bool32x4.or = function(a, b) {
    a = SIMD.Bool32x4.check(a);
    b = SIMD.Bool32x4.check(b);
    return SIMD.Bool32x4(SIMD.Bool32x4.extractLane(a, 0) | SIMD.Bool32x4.extractLane(b, 0),
                         SIMD.Bool32x4.extractLane(a, 1) | SIMD.Bool32x4.extractLane(b, 1),
                         SIMD.Bool32x4.extractLane(a, 2) | SIMD.Bool32x4.extractLane(b, 2),
                         SIMD.Bool32x4.extractLane(a, 3) | SIMD.Bool32x4.extractLane(b, 3));
  }
}

if (typeof SIMD.Bool32x4.xor === "undefined") {
  /**
    * @param {Bool32x4} a An instance of Bool32x4.
    * @param {Bool32x4} b An instance of Bool32x4.
    * @return {Bool32x4} New instance of Bool32x4 with values of a ^ b.
    */
  SIMD.Bool32x4.xor = function(a, b) {
    a = SIMD.Bool32x4.check(a);
    b = SIMD.Bool32x4.check(b);
    return SIMD.Bool32x4(SIMD.Bool32x4.extractLane(a, 0) ^ SIMD.Bool32x4.extractLane(b, 0),
                         SIMD.Bool32x4.extractLane(a, 1) ^ SIMD.Bool32x4.extractLane(b, 1),
                         SIMD.Bool32x4.extractLane(a, 2) ^ SIMD.Bool32x4.extractLane(b, 2),
                         SIMD.Bool32x4.extractLane(a, 3) ^ SIMD.Bool32x4.extractLane(b, 3));
  }
}

if (typeof SIMD.Bool32x4.not === "undefined") {
  /**
    * @param {Bool32x4} a An instance of Bool32x4.
    * @return {Bool32x4} New instance of Bool32x4 with values of !a
    */
  SIMD.Bool32x4.not = function(a) {
    a = SIMD.Bool32x4.check(a);
    return SIMD.Bool32x4(!SIMD.Bool32x4.extractLane(a, 0),
                         !SIMD.Bool32x4.extractLane(a, 1),
                         !SIMD.Bool32x4.extractLane(a, 2),
                         !SIMD.Bool32x4.extractLane(a, 3));
  }
}

if (typeof SIMD.Bool32x4.equal === "undefined") {
  /**
    * @param {Bool32x4} a An instance of Bool32x4.
    * @param {Bool32x4} b An instance of Bool32x4.
    * @return {Bool32x4} true or false in each lane depending on
    * the result of a == b.
    */
  SIMD.Bool32x4.equal = function(a, b) {
    a = SIMD.Bool32x4.check(a);
    b = SIMD.Bool32x4.check(b);
    return SIMD.Bool32x4(SIMD.Bool32x4.extractLane(a, 0) == SIMD.Bool32x4.extractLane(b, 0),
                         SIMD.Bool32x4.extractLane(a, 1) == SIMD.Bool32x4.extractLane(b, 1),
                         SIMD.Bool32x4.extractLane(a, 2) == SIMD.Bool32x4.extractLane(b, 2),
                         SIMD.Bool32x4.extractLane(a, 3) == SIMD.Bool32x4.extractLane(b, 3));
  }
}

if (typeof SIMD.Bool32x4.notEqual === "undefined") {
  /**
    * @param {Bool32x4} a An instance of Bool32x4.
    * @param {Bool32x4} b An instance of Bool32x4.
    * @return {Bool32x4} true or false in each lane depending on
    * the result of a != b.
    */
  SIMD.Bool32x4.notEqual = function(a, b) {
    a = SIMD.Bool32x4.check(a);
    b = SIMD.Bool32x4.check(b);
    return SIMD.Bool32x4(SIMD.Bool32x4.extractLane(a, 0) != SIMD.Bool32x4.extractLane(b, 0),
                         SIMD.Bool32x4.extractLane(a, 1) != SIMD.Bool32x4.extractLane(b, 1),
                         SIMD.Bool32x4.extractLane(a, 2) != SIMD.Bool32x4.extractLane(b, 2),
                         SIMD.Bool32x4.extractLane(a, 3) != SIMD.Bool32x4.extractLane(b, 3));
  }
}

if (typeof SIMD.Bool32x4.select === "undefined") {
  /**
    * @param {Bool32x4} mask Selector mask. An instance of Bool32x4
    * @param {Bool32x4} trueValue Pick lane from here if corresponding
    * selector lane is 1
    * @param {Bool32x4} falseValue Pick lane from here if corresponding
    * selector lane is 0
    * @return {Bool32x4} Mix of lanes from trueValue or falseValue as
    * indicated
    */
  SIMD.Bool32x4.select = function(mask, trueValue, falseValue) {
    mask = SIMD.Bool32x4.check(mask);
    trueValue = SIMD.Bool32x4.check(trueValue);
    falseValue = SIMD.Bool32x4.check(falseValue);
    var tr = SIMD.Bool32x4.and(mask, trueValue);
    var fr = SIMD.Bool32x4.and(SIMD.Bool32x4.not(mask), falseValue);
    return SIMD.Bool32x4.or(tr, fr);
  }
}

if (!Object.hasOwnProperty(SIMD.Bool32x4.prototype, 'valueOf')) {
  SIMD.Bool32x4.prototype.valueOf = function() {
    throw new TypeError("Bool32x4 cannot be converted to a number");
  }
}

if (!Object.hasOwnProperty(SIMD.Bool32x4.prototype, 'toString')) {
  /**
   * @return {String} a string representing the Bool32x4.
   */
  SIMD.Bool32x4.prototype.toString = function() {
    return "SIMD.Bool32x4(" +
      this.x_ + ", " +
      this.y_ + ", " +
      this.z_ + ", " +
      this.w_ + ")"
  }
}

if (!Object.hasOwnProperty(SIMD.Bool32x4.prototype, 'toLocaleString')) {
  /**
   * @return {String} a locale-sensitive string representing the Bool32x4.
   */
  SIMD.Bool32x4.prototype.toLocaleString = function() {
    return "SIMD.Bool32x4(" +
      this.x_.toLocaleString() + ", " +
      this.y_.toLocaleString() + ", " +
      this.z_.toLocaleString() + ", " +
      this.w_.toLocaleString() + ")"
  }
}

if (typeof SIMD.Bool16x8 === "undefined") {
  /**
    * Construct a new instance of Bool16x8 number.
    * @constructor
    */
  SIMD.Bool16x8 = function(s0, s1, s2, s3, s4, s5, s6, s7) {
    if (!(this instanceof SIMD.Bool16x8)) {
      return new SIMD.Bool16x8(s0, s1, s2, s3, s4, s5, s6, s7);
    }

    this.s0_ = !!s0;
    this.s1_ = !!s1;
    this.s2_ = !!s2;
    this.s3_ = !!s3;
    this.s4_ = !!s4;
    this.s5_ = !!s5;
    this.s6_ = !!s6;
    this.s7_ = !!s7;
  }
}

if (typeof SIMD.Bool16x8.check === "undefined") {
  /**
    * Check whether the argument is a Bool16x8.
    * @param {Bool16x8} v An instance of Bool16x8.
    * @return {Bool16x8} The Bool16x8 instance.
    */
  SIMD.Bool16x8.check = function(v) {
    if (!(v instanceof SIMD.Bool16x8)) {
      throw new TypeError("argument is not a Bool16x8.");
    }
    return v;
  }
}

if (typeof SIMD.Bool16x8.splat === "undefined") {
  /**
    * Construct a new instance of Bool16x8 with the same value
    * in all lanes.
    * @param {double} value used for all lanes.
    * @constructor
    */
  SIMD.Bool16x8.splat = function(s) {
    return SIMD.Bool16x8(s, s, s, s, s, s, s, s);
  }
}

if (typeof SIMD.Bool16x8.extractLane === "undefined") {
  /**
    * @param {Bool16x8} v An instance of Bool16x8.
    * @param {integer} i Index in concatenation of v for lane i
    * @return {Boolean} The value in lane i of v.
    */
  SIMD.Bool16x8.extractLane = function(v, i) {
    v = SIMD.Bool16x8.check(v);
    check8(i);
    switch(i) {
      case 0: return v.s0_;
      case 1: return v.s1_;
      case 2: return v.s2_;
      case 3: return v.s3_;
      case 4: return v.s4_;
      case 5: return v.s5_;
      case 6: return v.s6_;
      case 7: return v.s7_;
    }
  }
}

if (typeof SIMD.Bool16x8.replaceLane === "undefined") {
  /**
    * @param {Bool16x8} v An instance of Bool16x8.
    * @param {integer} i Index in concatenation of v for lane i
    * @param {double} value used for lane i.
    * @return {Bool16x8} New instance of Bool16x8 with the values in v and
    * lane i replaced with {s}.
    */
  SIMD.Bool16x8.replaceLane = function(v, i, s) {
    v = SIMD.Bool16x8.check(v);
    check8(i);
    saveBool16x8(v);
    _i16x8[i] = s;
    return restoreBool16x8();
  }
}

if (typeof SIMD.Bool16x8.allTrue === "undefined") {
  /**
    * Check if all 8 lanes hold a true value
    * @param {Bool16x8} v An instance of Bool16x8.
    * @return {Boolean} All 8 lanes holds a true value
    */
  SIMD.Bool16x8.allTrue = function(v) {
    v = SIMD.Bool16x8.check(v);
    return SIMD.Bool16x8.extractLane(v, 0) &&
           SIMD.Bool16x8.extractLane(v, 1) &&
           SIMD.Bool16x8.extractLane(v, 2) &&
           SIMD.Bool16x8.extractLane(v, 3) &&
           SIMD.Bool16x8.extractLane(v, 4) &&
           SIMD.Bool16x8.extractLane(v, 5) &&
           SIMD.Bool16x8.extractLane(v, 6) &&
           SIMD.Bool16x8.extractLane(v, 7);
  }
}

if (typeof SIMD.Bool16x8.anyTrue === "undefined") {
  /**
    * Check if any of the 8 lanes hold a true value
    * @param {Bool16x8} v An instance of Int16x8.
    * @return {Boolean} Any of the 8 lanes holds a true value
    */
  SIMD.Bool16x8.anyTrue = function(v) {
    v = SIMD.Bool16x8.check(v);
    return SIMD.Bool16x8.extractLane(v, 0) ||
           SIMD.Bool16x8.extractLane(v, 1) ||
           SIMD.Bool16x8.extractLane(v, 2) ||
           SIMD.Bool16x8.extractLane(v, 3) ||
           SIMD.Bool16x8.extractLane(v, 4) ||
           SIMD.Bool16x8.extractLane(v, 5) ||
           SIMD.Bool16x8.extractLane(v, 6) ||
           SIMD.Bool16x8.extractLane(v, 7);
  }
}

if (typeof SIMD.Bool16x8.and === "undefined") {
  /**
    * @param {Bool16x8} a An instance of Bool16x8.
    * @param {Bool16x8} b An instance of Bool16x8.
    * @return {Bool16x8} New instance of Bool16x8 with values of a & b.
    */
  SIMD.Bool16x8.and = function(a, b) {
    a = SIMD.Bool16x8.check(a);
    b = SIMD.Bool16x8.check(b);
    return SIMD.Bool16x8(SIMD.Bool16x8.extractLane(a, 0) & SIMD.Bool16x8.extractLane(b, 0),
                         SIMD.Bool16x8.extractLane(a, 1) & SIMD.Bool16x8.extractLane(b, 1),
                         SIMD.Bool16x8.extractLane(a, 2) & SIMD.Bool16x8.extractLane(b, 2),
                         SIMD.Bool16x8.extractLane(a, 3) & SIMD.Bool16x8.extractLane(b, 3),
                         SIMD.Bool16x8.extractLane(a, 4) & SIMD.Bool16x8.extractLane(b, 4),
                         SIMD.Bool16x8.extractLane(a, 5) & SIMD.Bool16x8.extractLane(b, 5),
                         SIMD.Bool16x8.extractLane(a, 6) & SIMD.Bool16x8.extractLane(b, 6),
                         SIMD.Bool16x8.extractLane(a, 7) & SIMD.Bool16x8.extractLane(b, 7));
  }
}

if (typeof SIMD.Bool16x8.or === "undefined") {
  /**
    * @param {Bool16x8} a An instance of Bool16x8.
    * @param {Bool16x8} b An instance of Bool16x8.
    * @return {Bool16x8} New instance of Bool16x8 with values of a | b.
    */
  SIMD.Bool16x8.or = function(a, b) {
    a = SIMD.Bool16x8.check(a);
    b = SIMD.Bool16x8.check(b);
    return SIMD.Bool16x8(SIMD.Bool16x8.extractLane(a, 0) | SIMD.Bool16x8.extractLane(b, 0),
                         SIMD.Bool16x8.extractLane(a, 1) | SIMD.Bool16x8.extractLane(b, 1),
                         SIMD.Bool16x8.extractLane(a, 2) | SIMD.Bool16x8.extractLane(b, 2),
                         SIMD.Bool16x8.extractLane(a, 3) | SIMD.Bool16x8.extractLane(b, 3),
                         SIMD.Bool16x8.extractLane(a, 4) | SIMD.Bool16x8.extractLane(b, 4),
                         SIMD.Bool16x8.extractLane(a, 5) | SIMD.Bool16x8.extractLane(b, 5),
                         SIMD.Bool16x8.extractLane(a, 6) | SIMD.Bool16x8.extractLane(b, 6),
                         SIMD.Bool16x8.extractLane(a, 7) | SIMD.Bool16x8.extractLane(b, 7));
  }
}

if (typeof SIMD.Bool16x8.xor === "undefined") {
  /**
    * @param {Bool16x8} a An instance of Bool16x8.
    * @param {Bool16x8} b An instance of Bool16x8.
    * @return {Bool16x8} New instance of Bool16x8 with values of a ^ b.
    */
  SIMD.Bool16x8.xor = function(a, b) {
    a = SIMD.Bool16x8.check(a);
    b = SIMD.Bool16x8.check(b);
    return SIMD.Bool16x8(SIMD.Bool16x8.extractLane(a, 0) ^ SIMD.Bool16x8.extractLane(b, 0),
                         SIMD.Bool16x8.extractLane(a, 1) ^ SIMD.Bool16x8.extractLane(b, 1),
                         SIMD.Bool16x8.extractLane(a, 2) ^ SIMD.Bool16x8.extractLane(b, 2),
                         SIMD.Bool16x8.extractLane(a, 3) ^ SIMD.Bool16x8.extractLane(b, 3),
                         SIMD.Bool16x8.extractLane(a, 4) ^ SIMD.Bool16x8.extractLane(b, 4),
                         SIMD.Bool16x8.extractLane(a, 5) ^ SIMD.Bool16x8.extractLane(b, 5),
                         SIMD.Bool16x8.extractLane(a, 6) ^ SIMD.Bool16x8.extractLane(b, 6),
                         SIMD.Bool16x8.extractLane(a, 7) ^ SIMD.Bool16x8.extractLane(b, 7));
  }
}

if (typeof SIMD.Bool16x8.not === "undefined") {
  /**
    * @param {Bool16x8} a An instance of Bool16x8.
    * @return {Bool16x8} New instance of Bool16x8 with values of !a
    */
  SIMD.Bool16x8.not = function(a) {
    a = SIMD.Bool16x8.check(a);
    return SIMD.Bool16x8(!SIMD.Bool16x8.extractLane(a, 0),
                         !SIMD.Bool16x8.extractLane(a, 1),
                         !SIMD.Bool16x8.extractLane(a, 2),
                         !SIMD.Bool16x8.extractLane(a, 3),
                         !SIMD.Bool16x8.extractLane(a, 4),
                         !SIMD.Bool16x8.extractLane(a, 5),
                         !SIMD.Bool16x8.extractLane(a, 6),
                         !SIMD.Bool16x8.extractLane(a, 7));
  }
}

if (typeof SIMD.Bool16x8.equal === "undefined") {
  /**
    * @param {Bool16x8} a An instance of Bool16x8.
    * @param {Bool16x8} b An instance of Bool16x8.
    * @return {Bool16x8} true or false in each lane depending on
    * the result of a == b.
    */
  SIMD.Bool16x8.equal = function(a, b) {
    a = SIMD.Bool16x8.check(a);
    b = SIMD.Bool16x8.check(b);
    return SIMD.Bool16x8(SIMD.Bool16x8.extractLane(a, 0) == SIMD.Bool16x8.extractLane(b, 0),
                         SIMD.Bool16x8.extractLane(a, 1) == SIMD.Bool16x8.extractLane(b, 1),
                         SIMD.Bool16x8.extractLane(a, 2) == SIMD.Bool16x8.extractLane(b, 2),
                         SIMD.Bool16x8.extractLane(a, 3) == SIMD.Bool16x8.extractLane(b, 3),
                         SIMD.Bool16x8.extractLane(a, 4) == SIMD.Bool16x8.extractLane(b, 4),
                         SIMD.Bool16x8.extractLane(a, 5) == SIMD.Bool16x8.extractLane(b, 5),
                         SIMD.Bool16x8.extractLane(a, 6) == SIMD.Bool16x8.extractLane(b, 6),
                         SIMD.Bool16x8.extractLane(a, 7) == SIMD.Bool16x8.extractLane(b, 7));
  }
}

if (typeof SIMD.Bool16x8.notEqual === "undefined") {
  /**
    * @param {Bool16x8} a An instance of Bool16x8.
    * @param {Bool16x8} b An instance of Bool16x8.
    * @return {Bool16x8} true or false in each lane depending on
    * the result of a != b.
    */
  SIMD.Bool16x8.notEqual = function(a, b) {
    a = SIMD.Bool16x8.check(a);
    b = SIMD.Bool16x8.check(b);
    return SIMD.Bool16x8(SIMD.Bool16x8.extractLane(a, 0) != SIMD.Bool16x8.extractLane(b, 0),
                         SIMD.Bool16x8.extractLane(a, 1) != SIMD.Bool16x8.extractLane(b, 1),
                         SIMD.Bool16x8.extractLane(a, 2) != SIMD.Bool16x8.extractLane(b, 2),
                         SIMD.Bool16x8.extractLane(a, 3) != SIMD.Bool16x8.extractLane(b, 3),
                         SIMD.Bool16x8.extractLane(a, 4) != SIMD.Bool16x8.extractLane(b, 4),
                         SIMD.Bool16x8.extractLane(a, 5) != SIMD.Bool16x8.extractLane(b, 5),
                         SIMD.Bool16x8.extractLane(a, 6) != SIMD.Bool16x8.extractLane(b, 6),
                         SIMD.Bool16x8.extractLane(a, 7) != SIMD.Bool16x8.extractLane(b, 7));
  }
}

if (typeof SIMD.Bool16x8.select === "undefined") {
  /**
    * @param {Bool16x8} mask Selector mask. An instance of Bool16x8
    * @param {Bool16x8} trueValue Pick lane from here if corresponding
    * selector lane is 1
    * @param {Bool16x8} falseValue Pick lane from here if corresponding
    * selector lane is 0
    * @return {Bool16x8} Mix of lanes from trueValue or falseValue as
    * indicated
    */
  SIMD.Bool16x8.select = function(mask, trueValue, falseValue) {
    mask = SIMD.Bool16x8.check(mask);
    trueValue = SIMD.Bool16x8.check(trueValue);
    falseValue = SIMD.Bool16x8.check(falseValue);
    var tr = SIMD.Bool16x8.and(mask, trueValue);
    var fr = SIMD.Bool16x8.and(SIMD.Bool16x8.not(mask), falseValue);
    return SIMD.Bool16x8.or(tr, fr);
  }
}

if (!Object.hasOwnProperty(SIMD.Bool16x8.prototype, 'valueOf')) {
  SIMD.Bool16x8.prototype.valueOf = function() {
    throw new TypeError("Bool16x8 cannot be converted to a number");
  }
}

if (!Object.hasOwnProperty(SIMD.Bool16x8.prototype, 'toString')) {
  /**
   * @return {String} a string representing the Bool32x4.
   */
  SIMD.Bool16x8.prototype.toString = function() {
    return "SIMD.Bool16x8(" +
      this.s0_ + ", " +
      this.s1_ + ", " +
      this.s2_ + ", " +
      this.s3_ + ", " +
      this.s4_ + ", " +
      this.s5_ + ", " +
      this.s6_ + ", " +
      this.s7_ + ")";
  }
}

if (!Object.hasOwnProperty(SIMD.Bool16x8.prototype, 'toLocaleString')) {
  /**
   * @return {String} a locale-sensitive string representing the Bool16x8.
   */
  SIMD.Bool16x8.prototype.toLocaleString = function() {
    return "SIMD.Bool16x8(" +
      this.s0_.toLocaleString() + ", " +
      this.s1_.toLocaleString() + ", " +
      this.s2_.toLocaleString() + ", " +
      this.s3_.toLocaleString() + ", " +
      this.s4_.toLocaleString() + ", " +
      this.s5_.toLocaleString() + ", " +
      this.s6_.toLocaleString() + ", " +
      this.s7_.toLocaleString() + ")";
  }
}

if (typeof SIMD.Bool8x16 === "undefined") {
  /**
    * Construct a new instance of Bool8x16 number.
    * @constructor
    */
  SIMD.Bool8x16 = function(s0, s1, s2, s3, s4, s5, s6, s7,
                           s8, s9, s10, s11, s12, s13, s14, s15) {
    if (!(this instanceof SIMD.Bool8x16)) {
      return new SIMD.Bool8x16(s0, s1, s2, s3, s4, s5, s6, s7,
                               s8, s9, s10, s11, s12, s13, s14, s15);
    }

    this.s0_ = !!s0;
    this.s1_ = !!s1;
    this.s2_ = !!s2;
    this.s3_ = !!s3;
    this.s4_ = !!s4;
    this.s5_ = !!s5;
    this.s6_ = !!s6;
    this.s7_ = !!s7;
    this.s8_ = !!s8;
    this.s9_ = !!s9;
    this.s10_ = !!s10;
    this.s11_ = !!s11;
    this.s12_ = !!s12;
    this.s13_ = !!s13;
    this.s14_ = !!s14;
    this.s15_ = !!s15;
  }
}

if (typeof SIMD.Bool8x16.check === "undefined") {
  /**
    * Check whether the argument is a Bool8x16.
    * @param {Bool8x16} v An instance of Bool8x16.
    * @return {Bool8x16} The Bool8x16 instance.
    */
  SIMD.Bool8x16.check = function(v) {
    if (!(v instanceof SIMD.Bool8x16)) {
      throw new TypeError("argument is not a Bool8x16.");
    }
    return v;
  }
}

if (typeof SIMD.Bool8x16.splat === "undefined") {
  /**
    * Construct a new instance of Bool8x16 with the same value
    * in all lanes.
    * @param {double} value used for all lanes.
    * @constructor
    */
  SIMD.Bool8x16.splat = function(s) {
    return SIMD.Bool8x16(s, s, s, s, s, s, s, s,
                         s, s, s, s, s, s, s, s);
  }
}

if (typeof SIMD.Bool8x16.extractLane === "undefined") {
  /**
    * @param {Bool8x16} v An instance of Bool8x16.
    * @param {integer} i Index in concatenation of v for lane i
    * @return {Boolean} The value in lane i of v.
    */
  SIMD.Bool8x16.extractLane = function(v, i) {
    v = SIMD.Bool8x16.check(v);
    check16(i);
    switch(i) {
      case 0: return v.s0_;
      case 1: return v.s1_;
      case 2: return v.s2_;
      case 3: return v.s3_;
      case 4: return v.s4_;
      case 5: return v.s5_;
      case 6: return v.s6_;
      case 7: return v.s7_;
      case 8: return v.s8_;
      case 9: return v.s9_;
      case 10: return v.s10_;
      case 11: return v.s11_;
      case 12: return v.s12_;
      case 13: return v.s13_;
      case 14: return v.s14_;
      case 15: return v.s15_;
    }
  }
}

if (typeof SIMD.Bool8x16.replaceLane === "undefined") {
  /**
    * @param {Bool8x16} v An instance of Bool8x16.
    * @param {integer} i Index in concatenation of v for lane i
    * @param {double} value used for lane i.
    * @return {Bool8x16} New instance of Bool8x16 with the values in v and
    * lane i replaced with {s}.
    */
  SIMD.Bool8x16.replaceLane = function(v, i, s) {
    v = SIMD.Bool8x16.check(v);
    check16(i);
    saveBool8x16(v);
    _i8x16[i] = s;
    return restoreBool8x16();
  }
}

if (typeof SIMD.Bool8x16.allTrue === "undefined") {
  /**
    * Check if all 16 lanes hold a true value
    * @param {Bool8x16} v An instance of Bool8x16.
    * @return {Boolean} All 16 lanes holds a true value
    */
  SIMD.Bool8x16.allTrue = function(v) {
    v = SIMD.Bool8x16.check(v);
    return SIMD.Bool8x16.extractLane(v, 0) &&
           SIMD.Bool8x16.extractLane(v, 1) &&
           SIMD.Bool8x16.extractLane(v, 2) &&
           SIMD.Bool8x16.extractLane(v, 3) &&
           SIMD.Bool8x16.extractLane(v, 4) &&
           SIMD.Bool8x16.extractLane(v, 5) &&
           SIMD.Bool8x16.extractLane(v, 6) &&
           SIMD.Bool8x16.extractLane(v, 7) &&
           SIMD.Bool8x16.extractLane(v, 8) &&
           SIMD.Bool8x16.extractLane(v, 9) &&
           SIMD.Bool8x16.extractLane(v, 10) &&
           SIMD.Bool8x16.extractLane(v, 11) &&
           SIMD.Bool8x16.extractLane(v, 12) &&
           SIMD.Bool8x16.extractLane(v, 13) &&
           SIMD.Bool8x16.extractLane(v, 14) &&
           SIMD.Bool8x16.extractLane(v, 15);
  }
}

if (typeof SIMD.Bool8x16.anyTrue === "undefined") {
  /**
    * Check if any of the 16 lanes hold a true value
    * @param {Bool8x16} v An instance of Bool16x8.
    * @return {Boolean} Any of the 16 lanes holds a true value
    */
  SIMD.Bool8x16.anyTrue = function(v) {
    v = SIMD.Bool8x16.check(v);
    return SIMD.Bool8x16.extractLane(v, 0) ||
           SIMD.Bool8x16.extractLane(v, 1) ||
           SIMD.Bool8x16.extractLane(v, 2) ||
           SIMD.Bool8x16.extractLane(v, 3) ||
           SIMD.Bool8x16.extractLane(v, 4) ||
           SIMD.Bool8x16.extractLane(v, 5) ||
           SIMD.Bool8x16.extractLane(v, 6) ||
           SIMD.Bool8x16.extractLane(v, 7) ||
           SIMD.Bool8x16.extractLane(v, 8) ||
           SIMD.Bool8x16.extractLane(v, 9) ||
           SIMD.Bool8x16.extractLane(v, 10) ||
           SIMD.Bool8x16.extractLane(v, 11) ||
           SIMD.Bool8x16.extractLane(v, 12) ||
           SIMD.Bool8x16.extractLane(v, 13) ||
           SIMD.Bool8x16.extractLane(v, 14) ||
           SIMD.Bool8x16.extractLane(v, 15);
  }
}

if (typeof SIMD.Bool8x16.and === "undefined") {
  /**
    * @param {Bool8x16} a An instance of Bool8x16.
    * @param {Bool8x16} b An instance of Bool8x16.
    * @return {Bool8x16} New instance of Bool8x16 with values of a & b.
    */
  SIMD.Bool8x16.and = function(a, b) {
    a = SIMD.Bool8x16.check(a);
    b = SIMD.Bool8x16.check(b);
    return SIMD.Bool8x16(SIMD.Bool8x16.extractLane(a, 0) & SIMD.Bool8x16.extractLane(b, 0),
                         SIMD.Bool8x16.extractLane(a, 1) & SIMD.Bool8x16.extractLane(b, 1),
                         SIMD.Bool8x16.extractLane(a, 2) & SIMD.Bool8x16.extractLane(b, 2),
                         SIMD.Bool8x16.extractLane(a, 3) & SIMD.Bool8x16.extractLane(b, 3),
                         SIMD.Bool8x16.extractLane(a, 4) & SIMD.Bool8x16.extractLane(b, 4),
                         SIMD.Bool8x16.extractLane(a, 5) & SIMD.Bool8x16.extractLane(b, 5),
                         SIMD.Bool8x16.extractLane(a, 6) & SIMD.Bool8x16.extractLane(b, 6),
                         SIMD.Bool8x16.extractLane(a, 7) & SIMD.Bool8x16.extractLane(b, 7),
                         SIMD.Bool8x16.extractLane(a, 8) & SIMD.Bool8x16.extractLane(b, 8),
                         SIMD.Bool8x16.extractLane(a, 9) & SIMD.Bool8x16.extractLane(b, 9),
                         SIMD.Bool8x16.extractLane(a, 10) & SIMD.Bool8x16.extractLane(b, 10),
                         SIMD.Bool8x16.extractLane(a, 11) & SIMD.Bool8x16.extractLane(b, 11),
                         SIMD.Bool8x16.extractLane(a, 12) & SIMD.Bool8x16.extractLane(b, 12),
                         SIMD.Bool8x16.extractLane(a, 13) & SIMD.Bool8x16.extractLane(b, 13),
                         SIMD.Bool8x16.extractLane(a, 14) & SIMD.Bool8x16.extractLane(b, 14),
                         SIMD.Bool8x16.extractLane(a, 15) & SIMD.Bool8x16.extractLane(b, 15));
  }
}

if (typeof SIMD.Bool8x16.or === "undefined") {
  /**
    * @param {Bool8x16} a An instance of Bool8x16.
    * @param {Bool8x16} b An instance of Bool8x16.
    * @return {Bool8x16} New instance of Bool8x16 with values of a | b.
    */
  SIMD.Bool8x16.or = function(a, b) {
    a = SIMD.Bool8x16.check(a);
    b = SIMD.Bool8x16.check(b);
    return SIMD.Bool8x16(SIMD.Bool8x16.extractLane(a, 0) | SIMD.Bool8x16.extractLane(b, 0),
                         SIMD.Bool8x16.extractLane(a, 1) | SIMD.Bool8x16.extractLane(b, 1),
                         SIMD.Bool8x16.extractLane(a, 2) | SIMD.Bool8x16.extractLane(b, 2),
                         SIMD.Bool8x16.extractLane(a, 3) | SIMD.Bool8x16.extractLane(b, 3),
                         SIMD.Bool8x16.extractLane(a, 4) | SIMD.Bool8x16.extractLane(b, 4),
                         SIMD.Bool8x16.extractLane(a, 5) | SIMD.Bool8x16.extractLane(b, 5),
                         SIMD.Bool8x16.extractLane(a, 6) | SIMD.Bool8x16.extractLane(b, 6),
                         SIMD.Bool8x16.extractLane(a, 7) | SIMD.Bool8x16.extractLane(b, 7),
                         SIMD.Bool8x16.extractLane(a, 8) | SIMD.Bool8x16.extractLane(b, 8),
                         SIMD.Bool8x16.extractLane(a, 9) | SIMD.Bool8x16.extractLane(b, 9),
                         SIMD.Bool8x16.extractLane(a, 10) | SIMD.Bool8x16.extractLane(b, 10),
                         SIMD.Bool8x16.extractLane(a, 11) | SIMD.Bool8x16.extractLane(b, 11),
                         SIMD.Bool8x16.extractLane(a, 12) | SIMD.Bool8x16.extractLane(b, 12),
                         SIMD.Bool8x16.extractLane(a, 13) | SIMD.Bool8x16.extractLane(b, 13),
                         SIMD.Bool8x16.extractLane(a, 14) | SIMD.Bool8x16.extractLane(b, 14),
                         SIMD.Bool8x16.extractLane(a, 15) | SIMD.Bool8x16.extractLane(b, 15));
  }
}

if (typeof SIMD.Bool8x16.xor === "undefined") {
  /**
    * @param {Bool8x16} a An instance of Bool8x16.
    * @param {Bool8x16} b An instance of Bool8x16.
    * @return {Bool8x16} New instance of Bool8x16 with values of a ^ b.
    */
  SIMD.Bool8x16.xor = function(a, b) {
    a = SIMD.Bool8x16.check(a);
    b = SIMD.Bool8x16.check(b);
    return SIMD.Bool8x16(SIMD.Bool8x16.extractLane(a, 0) ^ SIMD.Bool8x16.extractLane(b, 0),
                         SIMD.Bool8x16.extractLane(a, 1) ^ SIMD.Bool8x16.extractLane(b, 1),
                         SIMD.Bool8x16.extractLane(a, 2) ^ SIMD.Bool8x16.extractLane(b, 2),
                         SIMD.Bool8x16.extractLane(a, 3) ^ SIMD.Bool8x16.extractLane(b, 3),
                         SIMD.Bool8x16.extractLane(a, 4) ^ SIMD.Bool8x16.extractLane(b, 4),
                         SIMD.Bool8x16.extractLane(a, 5) ^ SIMD.Bool8x16.extractLane(b, 5),
                         SIMD.Bool8x16.extractLane(a, 6) ^ SIMD.Bool8x16.extractLane(b, 6),
                         SIMD.Bool8x16.extractLane(a, 7) ^ SIMD.Bool8x16.extractLane(b, 7),
                         SIMD.Bool8x16.extractLane(a, 8) ^ SIMD.Bool8x16.extractLane(b, 8),
                         SIMD.Bool8x16.extractLane(a, 9) ^ SIMD.Bool8x16.extractLane(b, 9),
                         SIMD.Bool8x16.extractLane(a, 10) ^ SIMD.Bool8x16.extractLane(b, 10),
                         SIMD.Bool8x16.extractLane(a, 11) ^ SIMD.Bool8x16.extractLane(b, 11),
                         SIMD.Bool8x16.extractLane(a, 12) ^ SIMD.Bool8x16.extractLane(b, 12),
                         SIMD.Bool8x16.extractLane(a, 13) ^ SIMD.Bool8x16.extractLane(b, 13),
                         SIMD.Bool8x16.extractLane(a, 14) ^ SIMD.Bool8x16.extractLane(b, 14),
                         SIMD.Bool8x16.extractLane(a, 15) ^ SIMD.Bool8x16.extractLane(b, 15));
  }
}

if (typeof SIMD.Bool8x16.not === "undefined") {
  /**
    * @param {Bool8x16} a An instance of Bool8x16.
    * @return {Bool8x16} New instance of Bool8x16 with values of !a
    */
  SIMD.Bool8x16.not = function(a) {
    a = SIMD.Bool8x16.check(a);
    return SIMD.Bool8x16(!SIMD.Bool8x16.extractLane(a, 0),
                         !SIMD.Bool8x16.extractLane(a, 1),
                         !SIMD.Bool8x16.extractLane(a, 2),
                         !SIMD.Bool8x16.extractLane(a, 3),
                         !SIMD.Bool8x16.extractLane(a, 4),
                         !SIMD.Bool8x16.extractLane(a, 5),
                         !SIMD.Bool8x16.extractLane(a, 6),
                         !SIMD.Bool8x16.extractLane(a, 7),
                         !SIMD.Bool8x16.extractLane(a, 8),
                         !SIMD.Bool8x16.extractLane(a, 9),
                         !SIMD.Bool8x16.extractLane(a, 10),
                         !SIMD.Bool8x16.extractLane(a, 11),
                         !SIMD.Bool8x16.extractLane(a, 12),
                         !SIMD.Bool8x16.extractLane(a, 13),
                         !SIMD.Bool8x16.extractLane(a, 14),
                         !SIMD.Bool8x16.extractLane(a, 15));
  }
}

if (typeof SIMD.Bool8x16.equal === "undefined") {
  /**
    * @param {Bool8x16} a An instance of Bool8x16.
    * @param {Bool8x16} b An instance of Bool8x16.
    * @return {Bool8x16} true or false in each lane depending on
    * the result of a == b.
    */
  SIMD.Bool8x16.equal = function(a, b) {
    a = SIMD.Bool8x16.check(a);
    b = SIMD.Bool8x16.check(b);
    return SIMD.Bool8x16(SIMD.Bool8x16.extractLane(a, 0) == SIMD.Bool8x16.extractLane(b, 0),
                         SIMD.Bool8x16.extractLane(a, 1) == SIMD.Bool8x16.extractLane(b, 1),
                         SIMD.Bool8x16.extractLane(a, 2) == SIMD.Bool8x16.extractLane(b, 2),
                         SIMD.Bool8x16.extractLane(a, 3) == SIMD.Bool8x16.extractLane(b, 3),
                         SIMD.Bool8x16.extractLane(a, 4) == SIMD.Bool8x16.extractLane(b, 4),
                         SIMD.Bool8x16.extractLane(a, 5) == SIMD.Bool8x16.extractLane(b, 5),
                         SIMD.Bool8x16.extractLane(a, 6) == SIMD.Bool8x16.extractLane(b, 6),
                         SIMD.Bool8x16.extractLane(a, 7) == SIMD.Bool8x16.extractLane(b, 7),
                         SIMD.Bool8x16.extractLane(a, 8) == SIMD.Bool8x16.extractLane(b, 8),
                         SIMD.Bool8x16.extractLane(a, 9) == SIMD.Bool8x16.extractLane(b, 9),
                         SIMD.Bool8x16.extractLane(a, 10) == SIMD.Bool8x16.extractLane(b, 10),
                         SIMD.Bool8x16.extractLane(a, 11) == SIMD.Bool8x16.extractLane(b, 11),
                         SIMD.Bool8x16.extractLane(a, 12) == SIMD.Bool8x16.extractLane(b, 12),
                         SIMD.Bool8x16.extractLane(a, 13) == SIMD.Bool8x16.extractLane(b, 13),
                         SIMD.Bool8x16.extractLane(a, 14) == SIMD.Bool8x16.extractLane(b, 14),
                         SIMD.Bool8x16.extractLane(a, 15) == SIMD.Bool8x16.extractLane(b, 15));
  }
}

if (typeof SIMD.Bool8x16.notEqual === "undefined") {
  /**
    * @param {Bool8x16} a An instance of Bool8x16.
    * @param {Bool8x16} b An instance of Bool8x16.
    * @return {Bool8x16} true or false in each lane depending on
    * the result of a != b.
    */
  SIMD.Bool8x16.notEqual = function(a, b) {
    a = SIMD.Bool8x16.check(a);
    b = SIMD.Bool8x16.check(b);
    return SIMD.Bool8x16(SIMD.Bool8x16.extractLane(a, 0) != SIMD.Bool8x16.extractLane(b, 0),
                         SIMD.Bool8x16.extractLane(a, 1) != SIMD.Bool8x16.extractLane(b, 1),
                         SIMD.Bool8x16.extractLane(a, 2) != SIMD.Bool8x16.extractLane(b, 2),
                         SIMD.Bool8x16.extractLane(a, 3) != SIMD.Bool8x16.extractLane(b, 3),
                         SIMD.Bool8x16.extractLane(a, 4) != SIMD.Bool8x16.extractLane(b, 4),
                         SIMD.Bool8x16.extractLane(a, 5) != SIMD.Bool8x16.extractLane(b, 5),
                         SIMD.Bool8x16.extractLane(a, 6) != SIMD.Bool8x16.extractLane(b, 6),
                         SIMD.Bool8x16.extractLane(a, 7) != SIMD.Bool8x16.extractLane(b, 7),
                         SIMD.Bool8x16.extractLane(a, 8) != SIMD.Bool8x16.extractLane(b, 8),
                         SIMD.Bool8x16.extractLane(a, 9) != SIMD.Bool8x16.extractLane(b, 9),
                         SIMD.Bool8x16.extractLane(a, 10) != SIMD.Bool8x16.extractLane(b, 10),
                         SIMD.Bool8x16.extractLane(a, 11) != SIMD.Bool8x16.extractLane(b, 11),
                         SIMD.Bool8x16.extractLane(a, 12) != SIMD.Bool8x16.extractLane(b, 12),
                         SIMD.Bool8x16.extractLane(a, 13) != SIMD.Bool8x16.extractLane(b, 13),
                         SIMD.Bool8x16.extractLane(a, 14) != SIMD.Bool8x16.extractLane(b, 14),
                         SIMD.Bool8x16.extractLane(a, 15) != SIMD.Bool8x16.extractLane(b, 15));
  }
}

if (typeof SIMD.Bool8x16.select === "undefined") {
  /**
    * @param {Bool8x16} mask Selector mask. An instance of Bool8x16
    * @param {Bool8x16} trueValue Pick lane from here if corresponding
    * selector lane is 1
    * @param {Bool8x16} falseValue Pick lane from here if corresponding
    * selector lane is 0
    * @return {Bool8x16} Mix of lanes from trueValue or falseValue as
    * indicated
    */
  SIMD.Bool8x16.select = function(mask, trueValue, falseValue) {
    mask = SIMD.Bool8x16.check(mask);
    trueValue = SIMD.Bool8x16.check(trueValue);
    falseValue = SIMD.Bool8x16.check(falseValue);
    var tr = SIMD.Bool8x16.and(mask, trueValue);
    var fr = SIMD.Bool8x16.and(SIMD.Bool8x16.not(mask), falseValue);
    return SIMD.Bool8x16.or(tr, fr);
  }
}

if (!Object.hasOwnProperty(SIMD.Bool8x16.prototype, 'valueOf')) {
  SIMD.Bool8x16.prototype.valueOf = function() {
    throw new TypeError("Bool8x16 cannot be converted to a number");
  }
}

if (!Object.hasOwnProperty(SIMD.Bool8x16.prototype, 'toString')) {
  /**
   * @return {String} a string representing the Bool32x4.
   */
  SIMD.Bool8x16.prototype.toString = function() {
    return "SIMD.Bool8x16(" +
      this.s0_ + ", " +
      this.s1_ + ", " +
      this.s2_ + ", " +
      this.s3_ + ", " +
      this.s4_ + ", " +
      this.s5_ + ", " +
      this.s6_ + ", " +
      this.s7_ + ", " +
      this.s8_ + ", " +
      this.s9_ + ", " +
      this.s10_ + ", " +
      this.s11_ + ", " +
      this.s12_ + ", " +
      this.s13_ + ", " +
      this.s14_ + ", " +
      this.s15_ + ")";
  }
}

if (!Object.hasOwnProperty(SIMD.Bool8x16.prototype, 'toLocaleString')) {
  /**
   * @return {String} a locale-sensitive string representing the Bool8x16.
   */
  SIMD.Bool8x16.prototype.toLocaleString = function() {
    return "SIMD.Bool8x16(" +
      this.s0_.toLocaleString() + ", " +
      this.s1_.toLocaleString() + ", " +
      this.s2_.toLocaleString() + ", " +
      this.s3_.toLocaleString() + ", " +
      this.s4_.toLocaleString() + ", " +
      this.s5_.toLocaleString() + ", " +
      this.s6_.toLocaleString() + ", " +
      this.s7_.toLocaleString() + ", " +
      this.s8_.toLocaleString() + ", " +
      this.s9_.toLocaleString() + ", " +
      this.s10_.toLocaleString() + ", " +
      this.s11_.toLocaleString() + ", " +
      this.s12_.toLocaleString() + ", " +
      this.s13_.toLocaleString() + ", " +
      this.s14_.toLocaleString() + ", " +
      this.s15_.toLocaleString() + ")";
  }
}


if (typeof SIMD.Float32x4 === "undefined") {
  /**
    * Construct a new instance of Float32x4 number.
    * @param {double} value used for x lane.
    * @param {double} value used for y lane.
    * @param {double} value used for z lane.
    * @param {double} value used for w lane.
    * @constructor
    */
  SIMD.Float32x4 = function(x, y, z, w) {
    if (!(this instanceof SIMD.Float32x4)) {
      return new SIMD.Float32x4(x, y, z, w);
    }

    this.x_ = truncatef32(x);
    this.y_ = truncatef32(y);
    this.z_ = truncatef32(z);
    this.w_ = truncatef32(w);
  }
}

if (typeof SIMD.Float32x4.extractLane === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @param {integer} i Index in concatenation of t for lane i
    * @return {double} The value in lane i of t.
    */
  SIMD.Float32x4.extractLane = function(t, i) {
    t = SIMD.Float32x4.check(t);
    check4(i);
    switch(i) {
      case 0: return t.x_;
      case 1: return t.y_;
      case 2: return t.z_;
      case 3: return t.w_;
    }
  }
}

if (typeof SIMD.Float32x4.replaceLane === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @param {integer} i Index in concatenation of t for lane i
    * @param {double} value used for lane i.
    * @return {Float32x4} New instance of Float32x4 with the values in t and
    * lane i replaced with {v}.
    */
  SIMD.Float32x4.replaceLane = function(t, i, v) {
    t = SIMD.Float32x4.check(t);
    check4(i);
    saveFloat32x4(t);
    _f32x4[i] = v;
    return restoreFloat32x4();
  }
}

if (typeof SIMD.Float32x4.check === "undefined") {
  /**
    * Check whether the argument is a Float32x4.
    * @param {Float32x4} v An instance of Float32x4.
    * @return {Float32x4} The Float32x4 instance.
    */
  SIMD.Float32x4.check = function(v) {
    if (!(v instanceof SIMD.Float32x4)) {
      throw new TypeError("argument is not a Float32x4.");
    }
    return v;
  }
}

if (typeof SIMD.Float32x4.splat === "undefined") {
  /**
    * Construct a new instance of Float32x4 with the same value
    * in all lanes.
    * @param {double} value used for all lanes.
    * @constructor
    */
  SIMD.Float32x4.splat = function(s) {
    return SIMD.Float32x4(s, s, s, s);
  }
}

if (typeof SIMD.Float32x4.fromInt32x4 === "undefined") {
  /**
    * @param {Int32x4} t An instance of Int32x4.
    * @return {Float32x4} An integer to float conversion copy of t.
    */
  SIMD.Float32x4.fromInt32x4 = function(t) {
    t = SIMD.Int32x4.check(t);
    return SIMD.Float32x4(SIMD.Int32x4.extractLane(t, 0),
                          SIMD.Int32x4.extractLane(t, 1),
                          SIMD.Int32x4.extractLane(t, 2),
                          SIMD.Int32x4.extractLane(t, 3));
  }
}

if (typeof SIMD.Float32x4.fromInt32x4Bits === "undefined") {
  /**
   * @param {Int32x4} t An instance of Int32x4.
   * @return {Float32x4} a bit-wise copy of t as a Float32x4.
   */
  SIMD.Float32x4.fromInt32x4Bits = function(t) {
    saveInt32x4(t);
    return restoreFloat32x4();
  }
}

if (typeof SIMD.Float32x4.fromInt16x8Bits === "undefined") {
  /**
   * @param {Int16x8} t An instance of Int16x8.
   * @return {Float32x4} a bit-wise copy of t as a Float32x4.
   */
  SIMD.Float32x4.fromInt16x8Bits = function(t) {
    saveInt16x8(t);
    return restoreFloat32x4();
  }
}

if (typeof SIMD.Float32x4.fromInt8x16Bits === "undefined") {
  /**
   * @param {Int8x16} t An instance of Int8x16.
   * @return {Float32x4} a bit-wise copy of t as a Float32x4.
   */
  SIMD.Float32x4.fromInt8x16Bits = function(t) {
    saveInt8x16(t);
    return restoreFloat32x4();
  }
}

if (!Object.hasOwnProperty(SIMD.Float32x4.prototype, 'toString')) {
  /**
   * @return {String} a string representing the Float32x4.
   */
  SIMD.Float32x4.prototype.toString = function() {
    return "SIMD.Float32x4(" +
      this.x_ + ", " +
      this.y_ + ", " +
      this.z_ + ", " +
      this.w_ + ")"
  }
}

if (!Object.hasOwnProperty(SIMD.Float32x4.prototype, 'toLocaleString')) {
  /**
   * @return {String} a locale-sensitive string representing the Float32x4.
   */
  SIMD.Float32x4.prototype.toLocaleString = function() {
    return "SIMD.Float32x4(" +
      this.x_.toLocaleString() + ", " +
      this.y_.toLocaleString() + ", " +
      this.z_.toLocaleString() + ", " +
      this.w_.toLocaleString() + ")"
  }
}

if (!Object.hasOwnProperty(SIMD.Float32x4.prototype, 'valueOf')) {
  SIMD.Float32x4.prototype.valueOf = function() {
    throw new TypeError("Float32x4 cannot be converted to a number");
  }
}

if (typeof SIMD.Int32x4 === "undefined") {
  /**
    * Construct a new instance of Int32x4 number.
    * @param {integer} 32-bit value used for x lane.
    * @param {integer} 32-bit value used for y lane.
    * @param {integer} 32-bit value used for z lane.
    * @param {integer} 32-bit value used for w lane.
    * @constructor
    */
  SIMD.Int32x4 = function(x, y, z, w) {
    if (!(this instanceof SIMD.Int32x4)) {
      return new SIMD.Int32x4(x, y, z, w);
    }

    this.x_ = x|0;
    this.y_ = y|0;
    this.z_ = z|0;
    this.w_ = w|0;
  }
}

if (typeof SIMD.Int32x4.extractLane === "undefined") {
  /**
    * @param {Int32x4} t An instance of Int32x4.
    * @param {integer} i Index in concatenation of t for lane i
    * @return {integer} The value in lane i of t.
    */
  SIMD.Int32x4.extractLane = function(t, i) {
    t = SIMD.Int32x4.check(t);
    check4(i);
    switch(i) {
      case 0: return t.x_;
      case 1: return t.y_;
      case 2: return t.z_;
      case 3: return t.w_;
    }
  }
}

if (typeof SIMD.Int32x4.replaceLane === "undefined") {
  /**
    * @param {Int32x4} t An instance of Int32x4.
    * @param {integer} i Index in concatenation of t for lane i
    * @param {integer} value used for lane i.
    * @return {Int32x4} New instance of Int32x4 with the values in t and
    * lane i replaced with {v}.
    */
  SIMD.Int32x4.replaceLane = function(t, i, v) {
    t = SIMD.Int32x4.check(t);
    check4(i);
    saveInt32x4(t);
    _i32x4[i] = v;
    return restoreInt32x4();
  }
}

if (typeof SIMD.Int32x4.check === "undefined") {
  /**
    * Check whether the argument is a Int32x4.
    * @param {Int32x4} v An instance of Int32x4.
    * @return {Int32x4} The Int32x4 instance.
    */
  SIMD.Int32x4.check = function(v) {
    if (!(v instanceof SIMD.Int32x4)) {
      throw new TypeError("argument is not a Int32x4.");
    }
    return v;
  }
}

if (typeof SIMD.Int32x4.splat === "undefined") {
  /**
    * Construct a new instance of Int32x4 with the same value
    * in all lanes.
    * @param {integer} value used for all lanes.
    * @constructor
    */
  SIMD.Int32x4.splat = function(s) {
    return SIMD.Int32x4(s, s, s, s);
  }
}

if (typeof SIMD.Int32x4.fromFloat32x4 === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @return {Int32x4} with a integer to float conversion of t.
    */
  SIMD.Int32x4.fromFloat32x4 = function(t) {
    t = SIMD.Float32x4.check(t);
    return SIMD.Int32x4(int32FromFloat(SIMD.Float32x4.extractLane(t, 0)),
                        int32FromFloat(SIMD.Float32x4.extractLane(t, 1)),
                        int32FromFloat(SIMD.Float32x4.extractLane(t, 2)),
                        int32FromFloat(SIMD.Float32x4.extractLane(t, 3)));
  }
}

if (typeof SIMD.Int32x4.fromFloat32x4Bits === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @return {Int32x4} a bit-wise copy of t as a Int32x4.
    */
  SIMD.Int32x4.fromFloat32x4Bits = function(t) {
    saveFloat32x4(t);
    return restoreInt32x4();
  }
}

if (typeof SIMD.Int32x4.fromInt16x8Bits === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @return {Int32x4} a bit-wise copy of t as a Int32x4.
    */
  SIMD.Int32x4.fromInt16x8Bits = function(t) {
    saveInt16x8(t);
    return restoreInt32x4();
  }
}

if (typeof SIMD.Int32x4.fromInt8x16Bits === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @return {Int32x4} a bit-wise copy of t as a Int32x4.
    */
  SIMD.Int32x4.fromInt8x16Bits = function(t) {
    saveInt8x16(t);
    return restoreInt32x4();
  }
}

if (!Object.hasOwnProperty(SIMD.Int32x4.prototype, 'toString')) {
  /**
   * @return {String} a string representing the Int32x4.
   */
  SIMD.Int32x4.prototype.toString = function() {
    return "SIMD.Int32x4(" +
      this.x_ + ", " +
      this.y_ + ", " +
      this.z_ + ", " +
      this.w_ + ")";
  }
}

if (!Object.hasOwnProperty(SIMD.Int32x4.prototype, 'toLocaleString')) {
  /**
   * @return {String} a locale-sensitive string representing the Int32x4.
   */
  SIMD.Int32x4.prototype.toLocaleString = function() {
    return "SIMD.Int32x4(" +
      this.x_.toLocaleString() + ", " +
      this.y_.toLocaleString() + ", " +
      this.z_.toLocaleString() + ", " +
      this.w_.toLocaleString() + ")";
  }
}

if (!Object.hasOwnProperty(SIMD.Int32x4.prototype, 'valueOf')) {
  SIMD.Int32x4.prototype.valueOf = function() {
    throw new TypeError("Int32x4 cannot be converted to a number");
  }
}

if (typeof SIMD.Int16x8 === "undefined") {
  /**
    * Construct a new instance of Int16x8 number.
    * @param {integer} 16-bit value used for s0 lane.
    * @param {integer} 16-bit value used for s1 lane.
    * @param {integer} 16-bit value used for s2 lane.
    * @param {integer} 16-bit value used for s3 lane.
    * @param {integer} 16-bit value used for s4 lane.
    * @param {integer} 16-bit value used for s5 lane.
    * @param {integer} 16-bit value used for s6 lane.
    * @param {integer} 16-bit value used for s7 lane.
    * @constructor
    */
  SIMD.Int16x8 = function(s0, s1, s2, s3, s4, s5, s6, s7) {
    if (!(this instanceof SIMD.Int16x8)) {
      return new SIMD.Int16x8(s0, s1, s2, s3, s4, s5, s6, s7);
    }

    this.s0_ = s0 << 16 >> 16;
    this.s1_ = s1 << 16 >> 16;
    this.s2_ = s2 << 16 >> 16;
    this.s3_ = s3 << 16 >> 16;
    this.s4_ = s4 << 16 >> 16;
    this.s5_ = s5 << 16 >> 16;
    this.s6_ = s6 << 16 >> 16;
    this.s7_ = s7 << 16 >> 16;
  }
}

if (typeof SIMD.Int16x8.extractLane === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @param {integer} i Index in concatenation of t for lane i
    * @return {integer} The value in lane i of t.
    */
  SIMD.Int16x8.extractLane = function(t, i) {
    t = SIMD.Int16x8.check(t);
    check8(i);
    switch(i) {
      case 0: return t.s0_;
      case 1: return t.s1_;
      case 2: return t.s2_;
      case 3: return t.s3_;
      case 4: return t.s4_;
      case 5: return t.s5_;
      case 6: return t.s6_;
      case 7: return t.s7_;
    }
  }
}

if (typeof SIMD.Int16x8.unsignedExtractLane === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @param {integer} i Index in concatenation of t for lane i
    * @return {integer} The value in lane i of t extracted as an unsigned value.
    */
  SIMD.Int16x8.unsignedExtractLane = function(t, i) {
    t = SIMD.Int16x8.check(t);
    check8(i);
    switch(i) {
      case 0: return t.s0_ & 0xffff;
      case 1: return t.s1_ & 0xffff;
      case 2: return t.s2_ & 0xffff;
      case 3: return t.s3_ & 0xffff;
      case 4: return t.s4_ & 0xffff;
      case 5: return t.s5_ & 0xffff;
      case 6: return t.s6_ & 0xffff;
      case 7: return t.s7_ & 0xffff;
    }
  }
}

if (typeof SIMD.Int16x8.replaceLane === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @param {integer} i Index in concatenation of t for lane i
    * @param {integer} value used for lane i.
    * @return {Int16x8} New instance of Int16x8 with the values in t and
    * lane i replaced with {v}.
    */
  SIMD.Int16x8.replaceLane = function(t, i, v) {
    t = SIMD.Int16x8.check(t);
    check8(i);
    saveInt16x8(t);
    _i16x8[i] = v;
    return restoreInt16x8();
  }
}

if (typeof SIMD.Int16x8.check === "undefined") {
  /**
    * Check whether the argument is a Int16x8.
    * @param {Int16x8} v An instance of Int16x8.
    * @return {Int16x8} The Int16x8 instance.
    */
  SIMD.Int16x8.check = function(v) {
    if (!(v instanceof SIMD.Int16x8)) {
      throw new TypeError("argument is not a Int16x8.");
    }
    return v;
  }
}

if (typeof SIMD.Int16x8.splat === "undefined") {
  /**
    * Construct a new instance of Int16x8 with the same value
    * in all lanes.
    * @param {integer} value used for all lanes.
    * @constructor
    */
  SIMD.Int16x8.splat = function(s) {
    return SIMD.Int16x8(s, s, s, s, s, s, s, s);
  }
}

if (typeof SIMD.Int16x8.fromFloat32x4Bits === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @return {Int16x8} a bit-wise copy of t as a Int16x8.
    */
  SIMD.Int16x8.fromFloat32x4Bits = function(t) {
    saveFloat32x4(t);
    return restoreInt16x8();
  }
}

if (typeof SIMD.Int16x8.fromInt32x4Bits === "undefined") {
  /**
    * @param {Int32x4} t An instance of Int32x4.
    * @return {Int16x8} a bit-wise copy of t as a Int16x8.
    */
  SIMD.Int16x8.fromInt32x4Bits = function(t) {
    saveInt32x4(t);
    return restoreInt16x8();
  }
}

if (typeof SIMD.Int16x8.fromInt8x16Bits === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @return {Int16x8} a bit-wise copy of t as a Int16x8.
    */
  SIMD.Int16x8.fromInt8x16Bits = function(t) {
    saveInt8x16(t);
    return restoreInt16x8();
  }
}

if (!Object.hasOwnProperty(SIMD.Int16x8.prototype, 'toString')) {
  /**
   * @return {String} a string representing the Int16x8.
   */
  SIMD.Int16x8.prototype.toString = function() {
    return "SIMD.Int16x8(" +
      this.s0_ + ", " +
      this.s1_ + ", " +
      this.s2_ + ", " +
      this.s3_ + ", " +
      this.s4_ + ", " +
      this.s5_ + ", " +
      this.s6_ + ", " +
      this.s7_ + ")";
  }
}

if (!Object.hasOwnProperty(SIMD.Int16x8.prototype, 'toLocaleString')) {
  /**
   * @return {String} a locale-sensitive string representing the Int16x8.
   */
  SIMD.Int16x8.prototype.toLocaleString = function() {
    return "SIMD.Int16x8(" +
      this.s0_.toLocaleString() + ", " +
      this.s1_.toLocaleString() + ", " +
      this.s2_.toLocaleString() + ", " +
      this.s3_.toLocaleString() + ", " +
      this.s4_.toLocaleString() + ", " +
      this.s5_.toLocaleString() + ", " +
      this.s6_.toLocaleString() + ", " +
      this.s7_.toLocaleString() + ")";
  }
}

if (!Object.hasOwnProperty(SIMD.Int16x8.prototype, 'valueOf')) {
  SIMD.Int16x8.prototype.valueOf = function() {
    throw new TypeError("Int16x8 cannot be converted to a number");
  }
}

if (typeof SIMD.Int8x16 === "undefined") {
  /**
    * Construct a new instance of Int8x16 number.
    * @param {integer} 8-bit value used for s0 lane.
    * @param {integer} 8-bit value used for s1 lane.
    * @param {integer} 8-bit value used for s2 lane.
    * @param {integer} 8-bit value used for s3 lane.
    * @param {integer} 8-bit value used for s4 lane.
    * @param {integer} 8-bit value used for s5 lane.
    * @param {integer} 8-bit value used for s6 lane.
    * @param {integer} 8-bit value used for s7 lane.
    * @param {integer} 8-bit value used for s8 lane.
    * @param {integer} 8-bit value used for s9 lane.
    * @param {integer} 8-bit value used for s10 lane.
    * @param {integer} 8-bit value used for s11 lane.
    * @param {integer} 8-bit value used for s12 lane.
    * @param {integer} 8-bit value used for s13 lane.
    * @param {integer} 8-bit value used for s14 lane.
    * @param {integer} 8-bit value used for s15 lane.
    * @constructor
    */
  SIMD.Int8x16 = function(s0, s1, s2, s3, s4, s5, s6, s7,
                          s8, s9, s10, s11, s12, s13, s14, s15) {
    if (!(this instanceof SIMD.Int8x16)) {
      return new SIMD.Int8x16(s0, s1, s2, s3, s4, s5, s6, s7,
                              s8, s9, s10, s11, s12, s13, s14, s15);
    }

    this.s0_ = s0 << 24 >> 24;
    this.s1_ = s1 << 24 >> 24;
    this.s2_ = s2 << 24 >> 24;
    this.s3_ = s3 << 24 >> 24;
    this.s4_ = s4 << 24 >> 24;
    this.s5_ = s5 << 24 >> 24;
    this.s6_ = s6 << 24 >> 24;
    this.s7_ = s7 << 24 >> 24;
    this.s8_ = s8 << 24 >> 24;
    this.s9_ = s9 << 24 >> 24;
    this.s10_ = s10 << 24 >> 24;
    this.s11_ = s11 << 24 >> 24;
    this.s12_ = s12 << 24 >> 24;
    this.s13_ = s13 << 24 >> 24;
    this.s14_ = s14 << 24 >> 24;
    this.s15_ = s15 << 24 >> 24;
  }
}

if (typeof SIMD.Int8x16.extractLane === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @param {integer} i Index in concatenation of t for lane i
    * @return {integer} The value in lane i of t.
    */
  SIMD.Int8x16.extractLane = function(t, i) {
    t = SIMD.Int8x16.check(t);
    check16(i);
    switch(i) {
      case 0: return t.s0_;
      case 1: return t.s1_;
      case 2: return t.s2_;
      case 3: return t.s3_;
      case 4: return t.s4_;
      case 5: return t.s5_;
      case 6: return t.s6_;
      case 7: return t.s7_;
      case 8: return t.s8_;
      case 9: return t.s9_;
      case 10: return t.s10_;
      case 11: return t.s11_;
      case 12: return t.s12_;
      case 13: return t.s13_;
      case 14: return t.s14_;
      case 15: return t.s15_;
    }
  }
}

if (typeof SIMD.Int8x16.unsignedExtractLane === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @param {integer} i Index in concatenation of t for lane i
    * @return {integer} The value in lane i of t extracted as an unsigned value.
    */
  SIMD.Int8x16.unsignedExtractLane = function(t, i) {
    t = SIMD.Int8x16.check(t);
    check16(i);
    switch(i) {
      case 0: return t.s0_ & 0xff;
      case 1: return t.s1_ & 0xff;
      case 2: return t.s2_ & 0xff;
      case 3: return t.s3_ & 0xff;
      case 4: return t.s4_ & 0xff;
      case 5: return t.s5_ & 0xff;
      case 6: return t.s6_ & 0xff;
      case 7: return t.s7_ & 0xff;
      case 8: return t.s8_ & 0xff;
      case 9: return t.s9_ & 0xff;
      case 10: return t.s10_ & 0xff;
      case 11: return t.s11_ & 0xff;
      case 12: return t.s12_ & 0xff;
      case 13: return t.s13_ & 0xff;
      case 14: return t.s14_ & 0xff;
      case 15: return t.s15_ & 0xff;
    }
  }
}

if (typeof SIMD.Int8x16.replaceLane === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @param {integer} i Index in concatenation of t for lane i
    * @param {integer} value used for lane i.
    * @return {Int8x16} New instance of Int8x16 with the values in t and
    * lane i replaced with {v}.
    */
  SIMD.Int8x16.replaceLane = function(t, i, v) {
    t = SIMD.Int8x16.check(t);
    check16(i);
    saveInt8x16(t);
    _i8x16[i] = v;
    return restoreInt8x16();
  }
}

if (typeof SIMD.Int8x16.check === "undefined") {
  /**
    * Check whether the argument is a Int8x16.
    * @param {Int8x16} v An instance of Int8x16.
    * @return {Int8x16} The Int8x16 instance.
    */
  SIMD.Int8x16.check = function(v) {
    if (!(v instanceof SIMD.Int8x16)) {
      throw new TypeError("argument is not a Int8x16.");
    }
    return v;
  }
}

if (typeof SIMD.Int8x16.splat === "undefined") {
  /**
    * Construct a new instance of Int8x16 with the same value
    * in all lanes.
    * @param {integer} value used for all lanes.
    * @constructor
    */
  SIMD.Int8x16.splat = function(s) {
    return SIMD.Int8x16(s, s, s, s, s, s, s, s,
                        s, s, s, s, s, s, s, s);
  }
}

if (typeof SIMD.Int8x16.fromFloat32x4Bits === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @return {Int8x16} a bit-wise copy of t as a Int8x16.
    */
  SIMD.Int8x16.fromFloat32x4Bits = function(t) {
    saveFloat32x4(t);
    return restoreInt8x16();
  }
}

if (typeof SIMD.Int8x16.fromInt32x4Bits === "undefined") {
  /**
    * @param {Int32x4} t An instance of Int32x4.
    * @return {Int8x16} a bit-wise copy of t as a Int8x16.
    */
  SIMD.Int8x16.fromInt32x4Bits = function(t) {
    saveInt32x4(t);
    return restoreInt8x16();
  }
}

if (typeof SIMD.Int8x16.fromInt16x8Bits === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @return {Int8x16} a bit-wise copy of t as a Int8x16.
    */
  SIMD.Int8x16.fromInt16x8Bits = function(t) {
    saveInt16x8(t);
    return restoreInt8x16();
  }
}

if (!Object.hasOwnProperty(SIMD.Int8x16.prototype, 'toString')) {
  /**
   * @return {String} a string representing the Int8x16.
   */
  SIMD.Int8x16.prototype.toString = function() {
    return "SIMD.Int8x16(" +
      this.s0_ + ", " +
      this.s1_ + ", " +
      this.s2_ + ", " +
      this.s3_ + ", " +
      this.s4_ + ", " +
      this.s5_ + ", " +
      this.s6_ + ", " +
      this.s7_ + ", " +
      this.s8_ + ", " +
      this.s9_ + ", " +
      this.s10_ + ", " +
      this.s11_ + ", " +
      this.s12_ + ", " +
      this.s13_ + ", " +
      this.s14_ + ", " +
      this.s15_ + ")";
  }
}

if (!Object.hasOwnProperty(SIMD.Int8x16.prototype, 'toLocaleString')) {
  /**
   * @return {String} a locale-sensitive string representing the Int8x16.
   */
  SIMD.Int8x16.prototype.toLocaleString = function() {
    return "SIMD.Int8x16(" +
      this.s0_.toLocaleString() + ", " +
      this.s1_.toLocaleString() + ", " +
      this.s2_.toLocaleString() + ", " +
      this.s3_.toLocaleString() + ", " +
      this.s4_.toLocaleString() + ", " +
      this.s5_.toLocaleString() + ", " +
      this.s6_.toLocaleString() + ", " +
      this.s7_.toLocaleString() + ", " +
      this.s8_.toLocaleString() + ", " +
      this.s9_.toLocaleString() + ", " +
      this.s10_.toLocaleString() + ", " +
      this.s11_.toLocaleString() + ", " +
      this.s12_.toLocaleString() + ", " +
      this.s13_.toLocaleString() + ", " +
      this.s14_.toLocaleString() + ", " +
      this.s15_.toLocaleString() + ")";
  }
}

if (!Object.hasOwnProperty(SIMD.Int8x16.prototype, 'valueOf')) {
  SIMD.Int8x16.prototype.valueOf = function() {
    throw new TypeError("Int8x16 cannot be converted to a number");
  }
}

if (typeof SIMD.Float32x4.abs === "undefined") {
  /**
   * @param {Float32x4} t An instance of Float32x4.
   * @return {Float32x4} New instance of Float32x4 with absolute values of
   * t.
   */
  SIMD.Float32x4.abs = function(t) {
    t = SIMD.Float32x4.check(t);
    return SIMD.Float32x4(Math.abs(SIMD.Float32x4.extractLane(t, 0)),
                          Math.abs(SIMD.Float32x4.extractLane(t, 1)),
                          Math.abs(SIMD.Float32x4.extractLane(t, 2)),
                          Math.abs(SIMD.Float32x4.extractLane(t, 3)));
  }
}

if (typeof SIMD.Float32x4.neg === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @return {Float32x4} New instance of Float32x4 with negated values of
    * t.
    */
  SIMD.Float32x4.neg = function(t) {
    t = SIMD.Float32x4.check(t);
    return SIMD.Float32x4(-SIMD.Float32x4.extractLane(t, 0),
                          -SIMD.Float32x4.extractLane(t, 1),
                          -SIMD.Float32x4.extractLane(t, 2),
                          -SIMD.Float32x4.extractLane(t, 3));
  }
}

if (typeof SIMD.Float32x4.add === "undefined") {
  /**
    * @param {Float32x4} a An instance of Float32x4.
    * @param {Float32x4} b An instance of Float32x4.
    * @return {Float32x4} New instance of Float32x4 with a + b.
    */
  SIMD.Float32x4.add = function(a, b) {
    a = SIMD.Float32x4.check(a);
    b = SIMD.Float32x4.check(b);
    return SIMD.Float32x4(
        SIMD.Float32x4.extractLane(a, 0) + SIMD.Float32x4.extractLane(b, 0),
        SIMD.Float32x4.extractLane(a, 1) + SIMD.Float32x4.extractLane(b, 1),
        SIMD.Float32x4.extractLane(a, 2) + SIMD.Float32x4.extractLane(b, 2),
        SIMD.Float32x4.extractLane(a, 3) + SIMD.Float32x4.extractLane(b, 3));
  }
}

if (typeof SIMD.Float32x4.sub === "undefined") {
  /**
    * @param {Float32x4} a An instance of Float32x4.
    * @param {Float32x4} b An instance of Float32x4.
    * @return {Float32x4} New instance of Float32x4 with a - b.
    */
  SIMD.Float32x4.sub = function(a, b) {
    a = SIMD.Float32x4.check(a);
    b = SIMD.Float32x4.check(b);
    return SIMD.Float32x4(
        SIMD.Float32x4.extractLane(a, 0) - SIMD.Float32x4.extractLane(b, 0),
        SIMD.Float32x4.extractLane(a, 1) - SIMD.Float32x4.extractLane(b, 1),
        SIMD.Float32x4.extractLane(a, 2) - SIMD.Float32x4.extractLane(b, 2),
        SIMD.Float32x4.extractLane(a, 3) - SIMD.Float32x4.extractLane(b, 3));
  }
}

if (typeof SIMD.Float32x4.mul === "undefined") {
  /**
    * @param {Float32x4} a An instance of Float32x4.
    * @param {Float32x4} b An instance of Float32x4.
    * @return {Float32x4} New instance of Float32x4 with a * b.
    */
  SIMD.Float32x4.mul = function(a, b) {
    a = SIMD.Float32x4.check(a);
    b = SIMD.Float32x4.check(b);
    return SIMD.Float32x4(
        SIMD.Float32x4.extractLane(a, 0) * SIMD.Float32x4.extractLane(b, 0),
        SIMD.Float32x4.extractLane(a, 1) * SIMD.Float32x4.extractLane(b, 1),
        SIMD.Float32x4.extractLane(a, 2) * SIMD.Float32x4.extractLane(b, 2),
        SIMD.Float32x4.extractLane(a, 3) * SIMD.Float32x4.extractLane(b, 3));
  }
}

if (typeof SIMD.Float32x4.div === "undefined") {
  /**
    * @param {Float32x4} a An instance of Float32x4.
    * @param {Float32x4} b An instance of Float32x4.
    * @return {Float32x4} New instance of Float32x4 with a / b.
    */
  SIMD.Float32x4.div = function(a, b) {
    a = SIMD.Float32x4.check(a);
    b = SIMD.Float32x4.check(b);
    return SIMD.Float32x4(
        SIMD.Float32x4.extractLane(a, 0) / SIMD.Float32x4.extractLane(b, 0),
        SIMD.Float32x4.extractLane(a, 1) / SIMD.Float32x4.extractLane(b, 1),
        SIMD.Float32x4.extractLane(a, 2) / SIMD.Float32x4.extractLane(b, 2),
        SIMD.Float32x4.extractLane(a, 3) / SIMD.Float32x4.extractLane(b, 3));
  }
}

if (typeof SIMD.Float32x4.min === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @param {Float32x4} other An instance of Float32x4.
    * @return {Float32x4} New instance of Float32x4 with the minimum value of
    * t and other.
    */
  SIMD.Float32x4.min = function(t, other) {
    t = SIMD.Float32x4.check(t);
    other = SIMD.Float32x4.check(other);
    var cx = Math.min(SIMD.Float32x4.extractLane(t, 0),
                      SIMD.Float32x4.extractLane(other, 0));
    var cy = Math.min(SIMD.Float32x4.extractLane(t, 1),
                      SIMD.Float32x4.extractLane(other, 1));
    var cz = Math.min(SIMD.Float32x4.extractLane(t, 2),
                      SIMD.Float32x4.extractLane(other, 2));
    var cw = Math.min(SIMD.Float32x4.extractLane(t, 3),
                      SIMD.Float32x4.extractLane(other, 3));
    return SIMD.Float32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Float32x4.max === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @param {Float32x4} other An instance of Float32x4.
    * @return {Float32x4} New instance of Float32x4 with the maximum value of
    * t and other.
    */
  SIMD.Float32x4.max = function(t, other) {
    t = SIMD.Float32x4.check(t);
    other = SIMD.Float32x4.check(other);
    var cx = Math.max(SIMD.Float32x4.extractLane(t, 0),
                      SIMD.Float32x4.extractLane(other, 0));
    var cy = Math.max(SIMD.Float32x4.extractLane(t, 1),
                      SIMD.Float32x4.extractLane(other, 1));
    var cz = Math.max(SIMD.Float32x4.extractLane(t, 2),
                      SIMD.Float32x4.extractLane(other, 2));
    var cw = Math.max(SIMD.Float32x4.extractLane(t, 3),
                      SIMD.Float32x4.extractLane(other, 3));
    return SIMD.Float32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Float32x4.minNum === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @param {Float32x4} other An instance of Float32x4.
    * @return {Float32x4} New instance of Float32x4 with the minimum value of
    * t and other, preferring numbers over NaNs.
    */
  SIMD.Float32x4.minNum = function(t, other) {
    t = SIMD.Float32x4.check(t);
    other = SIMD.Float32x4.check(other);
    var cx = minNum(SIMD.Float32x4.extractLane(t, 0),
                    SIMD.Float32x4.extractLane(other, 0));
    var cy = minNum(SIMD.Float32x4.extractLane(t, 1),
                    SIMD.Float32x4.extractLane(other, 1));
    var cz = minNum(SIMD.Float32x4.extractLane(t, 2),
                    SIMD.Float32x4.extractLane(other, 2));
    var cw = minNum(SIMD.Float32x4.extractLane(t, 3),
                    SIMD.Float32x4.extractLane(other, 3));
    return SIMD.Float32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Float32x4.maxNum === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @param {Float32x4} other An instance of Float32x4.
    * @return {Float32x4} New instance of Float32x4 with the maximum value of
    * t and other, preferring numbers over NaNs.
    */
  SIMD.Float32x4.maxNum = function(t, other) {
    t = SIMD.Float32x4.check(t);
    other = SIMD.Float32x4.check(other);
    var cx = maxNum(SIMD.Float32x4.extractLane(t, 0),
                    SIMD.Float32x4.extractLane(other, 0));
    var cy = maxNum(SIMD.Float32x4.extractLane(t, 1),
                    SIMD.Float32x4.extractLane(other, 1));
    var cz = maxNum(SIMD.Float32x4.extractLane(t, 2),
                    SIMD.Float32x4.extractLane(other, 2));
    var cw = maxNum(SIMD.Float32x4.extractLane(t, 3),
                    SIMD.Float32x4.extractLane(other, 3));
    return SIMD.Float32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Float32x4.reciprocalApproximation === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @return {Float32x4} New instance of Float32x4 with an approximation of the
    * reciprocal value of t.
    */
  SIMD.Float32x4.reciprocalApproximation = function(t) {
    t = SIMD.Float32x4.check(t);
    return SIMD.Float32x4.div(SIMD.Float32x4.splat(1.0), t);
  }
}

if (typeof SIMD.Float32x4.reciprocalSqrtApproximation === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @return {Float32x4} New instance of Float32x4 with an approximation of the
    * reciprocal value of the square root of t.
    */
  SIMD.Float32x4.reciprocalSqrtApproximation = function(t) {
    t = SIMD.Float32x4.check(t);
    return SIMD.Float32x4.reciprocalApproximation(SIMD.Float32x4.sqrt(t));
  }
}

if (typeof SIMD.Float32x4.sqrt === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @return {Float32x4} New instance of Float32x4 with square root of
    * values of t.
    */
  SIMD.Float32x4.sqrt = function(t) {
    t = SIMD.Float32x4.check(t);
    return SIMD.Float32x4(Math.sqrt(SIMD.Float32x4.extractLane(t, 0)),
                          Math.sqrt(SIMD.Float32x4.extractLane(t, 1)),
                          Math.sqrt(SIMD.Float32x4.extractLane(t, 2)),
                          Math.sqrt(SIMD.Float32x4.extractLane(t, 3)));
  }
}

if (typeof SIMD.Float32x4.swizzle === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4 to be swizzled.
    * @param {integer} x - Index in t for lane x
    * @param {integer} y - Index in t for lane y
    * @param {integer} z - Index in t for lane z
    * @param {integer} w - Index in t for lane w
    * @return {Float32x4} New instance of Float32x4 with lanes swizzled.
    */
  SIMD.Float32x4.swizzle = function(t, x, y, z, w) {
    t = SIMD.Float32x4.check(t);
    check4(x);
    check4(y);
    check4(z);
    check4(w);
    _f32x4[0] = SIMD.Float32x4.extractLane(t, 0);
    _f32x4[1] = SIMD.Float32x4.extractLane(t, 1);
    _f32x4[2] = SIMD.Float32x4.extractLane(t, 2);
    _f32x4[3] = SIMD.Float32x4.extractLane(t, 3);
    var storage = _f32x4;
    return SIMD.Float32x4(storage[x], storage[y], storage[z], storage[w]);
  }
}

if (typeof SIMD.Float32x4.shuffle === "undefined") {

  var _f32x8 = new Float32Array(8);

  /**
    * @param {Float32x4} t1 An instance of Float32x4 to be shuffled.
    * @param {Float32x4} t2 An instance of Float32x4 to be shuffled.
    * @param {integer} x - Index in concatenation of t1 and t2 for lane x
    * @param {integer} y - Index in concatenation of t1 and t2 for lane y
    * @param {integer} z - Index in concatenation of t1 and t2 for lane z
    * @param {integer} w - Index in concatenation of t1 and t2 for lane w
    * @return {Float32x4} New instance of Float32x4 with lanes shuffled.
    */
  SIMD.Float32x4.shuffle = function(t1, t2, x, y, z, w) {
    t1 = SIMD.Float32x4.check(t1);
    t2 = SIMD.Float32x4.check(t2);
    check8(x);
    check8(y);
    check8(z);
    check8(w);
    var storage = _f32x8;
    storage[0] = SIMD.Float32x4.extractLane(t1, 0);
    storage[1] = SIMD.Float32x4.extractLane(t1, 1);
    storage[2] = SIMD.Float32x4.extractLane(t1, 2);
    storage[3] = SIMD.Float32x4.extractLane(t1, 3);
    storage[4] = SIMD.Float32x4.extractLane(t2, 0);
    storage[5] = SIMD.Float32x4.extractLane(t2, 1);
    storage[6] = SIMD.Float32x4.extractLane(t2, 2);
    storage[7] = SIMD.Float32x4.extractLane(t2, 3);
    return SIMD.Float32x4(storage[x], storage[y], storage[z], storage[w]);
  }
}

if (typeof SIMD.Float32x4.lessThan === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @param {Float32x4} other An instance of Float32x4.
    * @return {Bool32x4} true or false in each lane depending on
    * the result of t < other.
    */
  SIMD.Float32x4.lessThan = function(t, other) {
    t = SIMD.Float32x4.check(t);
    other = SIMD.Float32x4.check(other);
    var cx =
        SIMD.Float32x4.extractLane(t, 0) < SIMD.Float32x4.extractLane(other, 0);
    var cy =
        SIMD.Float32x4.extractLane(t, 1) < SIMD.Float32x4.extractLane(other, 1);
    var cz =
        SIMD.Float32x4.extractLane(t, 2) < SIMD.Float32x4.extractLane(other, 2);
    var cw =
        SIMD.Float32x4.extractLane(t, 3) < SIMD.Float32x4.extractLane(other, 3);
    return SIMD.Bool32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Float32x4.lessThanOrEqual === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @param {Float32x4} other An instance of Float32x4.
    * @return {Bool32x4} true or false in each lane depending on
    * the result of t <= other.
    */
  SIMD.Float32x4.lessThanOrEqual = function(t, other) {
    t = SIMD.Float32x4.check(t);
    other = SIMD.Float32x4.check(other);
    var cx = SIMD.Float32x4.extractLane(t, 0) <=
        SIMD.Float32x4.extractLane(other, 0);
    var cy = SIMD.Float32x4.extractLane(t, 1) <=
        SIMD.Float32x4.extractLane(other, 1);
    var cz = SIMD.Float32x4.extractLane(t, 2) <=
        SIMD.Float32x4.extractLane(other, 2);
    var cw = SIMD.Float32x4.extractLane(t, 3) <=
        SIMD.Float32x4.extractLane(other, 3);
    return SIMD.Bool32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Float32x4.equal === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @param {Float32x4} other An instance of Float32x4.
    * @return {Bool32x4} true or false in each lane depending on
    * the result of t == other.
    */
  SIMD.Float32x4.equal = function(t, other) {
    t = SIMD.Float32x4.check(t);
    other = SIMD.Float32x4.check(other);
    var cx = SIMD.Float32x4.extractLane(t, 0) ==
        SIMD.Float32x4.extractLane(other, 0);
    var cy = SIMD.Float32x4.extractLane(t, 1) ==
        SIMD.Float32x4.extractLane(other, 1);
    var cz = SIMD.Float32x4.extractLane(t, 2) ==
        SIMD.Float32x4.extractLane(other, 2);
    var cw = SIMD.Float32x4.extractLane(t, 3) ==
        SIMD.Float32x4.extractLane(other, 3);
    return SIMD.Bool32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Float32x4.notEqual === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @param {Float32x4} other An instance of Float32x4.
    * @return {Bool32x4} true or false in each lane depending on
    * the result of t != other.
    */
  SIMD.Float32x4.notEqual = function(t, other) {
    t = SIMD.Float32x4.check(t);
    other = SIMD.Float32x4.check(other);
    var cx = SIMD.Float32x4.extractLane(t, 0) !=
        SIMD.Float32x4.extractLane(other, 0);
    var cy = SIMD.Float32x4.extractLane(t, 1) !=
        SIMD.Float32x4.extractLane(other, 1);
    var cz = SIMD.Float32x4.extractLane(t, 2) !=
        SIMD.Float32x4.extractLane(other, 2);
    var cw = SIMD.Float32x4.extractLane(t, 3) !=
        SIMD.Float32x4.extractLane(other, 3);
    return SIMD.Bool32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Float32x4.greaterThanOrEqual === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @param {Float32x4} other An instance of Float32x4.
    * @return {Bool32x4} true or false in each lane depending on
    * the result of t >= other.
    */
  SIMD.Float32x4.greaterThanOrEqual = function(t, other) {
    t = SIMD.Float32x4.check(t);
    other = SIMD.Float32x4.check(other);
    var cx = SIMD.Float32x4.extractLane(t, 0) >=
        SIMD.Float32x4.extractLane(other, 0);
    var cy = SIMD.Float32x4.extractLane(t, 1) >=
        SIMD.Float32x4.extractLane(other, 1);
    var cz = SIMD.Float32x4.extractLane(t, 2) >=
        SIMD.Float32x4.extractLane(other, 2);
    var cw = SIMD.Float32x4.extractLane(t, 3) >=
        SIMD.Float32x4.extractLane(other, 3);
    return SIMD.Bool32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Float32x4.greaterThan === "undefined") {
  /**
    * @param {Float32x4} t An instance of Float32x4.
    * @param {Float32x4} other An instance of Float32x4.
    * @return {Bool32x4} true or false in each lane depending on
    * the result of t > other.
    */
  SIMD.Float32x4.greaterThan = function(t, other) {
    t = SIMD.Float32x4.check(t);
    other = SIMD.Float32x4.check(other);
    var cx =
        SIMD.Float32x4.extractLane(t, 0) > SIMD.Float32x4.extractLane(other, 0);
    var cy =
        SIMD.Float32x4.extractLane(t, 1) > SIMD.Float32x4.extractLane(other, 1);
    var cz =
        SIMD.Float32x4.extractLane(t, 2) > SIMD.Float32x4.extractLane(other, 2);
    var cw =
        SIMD.Float32x4.extractLane(t, 3) > SIMD.Float32x4.extractLane(other, 3);
    return SIMD.Bool32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Float32x4.select === "undefined") {
  /**
    * @param {Bool32x4} t Selector mask. An instance of Bool32x4
    * @param {Float32x4} trueValue Pick lane from here if corresponding
    * selector lane is true
    * @param {Float32x4} falseValue Pick lane from here if corresponding
    * selector lane is false
    * @return {Float32x4} Mix of lanes from trueValue or falseValue as
    * indicated
    */
  SIMD.Float32x4.select = function(t, trueValue, falseValue) {
    t = SIMD.Bool32x4.check(t);
    trueValue = SIMD.Float32x4.check(trueValue);
    falseValue = SIMD.Float32x4.check(falseValue);
    return SIMD.Float32x4(
        SIMD.Bool32x4.extractLane(t, 0) ?
            SIMD.Float32x4.extractLane(trueValue, 0) :
                SIMD.Float32x4.extractLane(falseValue, 0),
        SIMD.Bool32x4.extractLane(t, 1) ?
            SIMD.Float32x4.extractLane(trueValue, 1) :
                SIMD.Float32x4.extractLane(falseValue, 1),
        SIMD.Bool32x4.extractLane(t, 2) ?
            SIMD.Float32x4.extractLane(trueValue, 2) :
                SIMD.Float32x4.extractLane(falseValue, 2),
        SIMD.Bool32x4.extractLane(t, 3) ?
            SIMD.Float32x4.extractLane(trueValue, 3) :
                SIMD.Float32x4.extractLane(falseValue, 3));
  }
}

if (typeof SIMD.Float32x4.load === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {Float32x4} New instance of Float32x4.
    */
  SIMD.Float32x4.load = function(tarray, index) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var f32temp = _f32x4;
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? f32temp : _i32x4) :
                _f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.Float32x4(f32temp[0], f32temp[1], f32temp[2], f32temp[3]);
  }
}

if (typeof SIMD.Float32x4.load1 === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {Float32x4} New instance of Float32x4.
    */
  SIMD.Float32x4.load1 = function(tarray, index) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 4) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var f32temp = _f32x4;
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? f32temp : _i32x4) :
                _f64x2;
    var n = 4 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.Float32x4(f32temp[0], 0.0, 0.0, 0.0);
  }
}

if (typeof SIMD.Float32x4.load2 === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {Float32x4} New instance of Float32x4.
    */
  SIMD.Float32x4.load2 = function(tarray, index) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 8) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var f32temp = _f32x4;
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? f32temp : _i32x4) :
                _f64x2;
    var n = 8 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.Float32x4(f32temp[0], f32temp[1], 0.0, 0.0);
  }
}

if (typeof SIMD.Float32x4.load3 === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {Float32x4} New instance of Float32x4.
    */
  SIMD.Float32x4.load3 = function(tarray, index) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 12) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var f32temp = _f32x4;
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? f32temp : _i32x4) :
                _f64x2;
    var n = 12 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.Float32x4(f32temp[0], f32temp[1], f32temp[2], 0.0);
  }
}

if (typeof SIMD.Float32x4.store === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {Float32x4} value An instance of Float32x4.
    * @return {Float32x4} value
    */
  SIMD.Float32x4.store = function(tarray, index, value) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.Float32x4.check(value);
    _f32x4[0] = SIMD.Float32x4.extractLane(value, 0);
    _f32x4[1] = SIMD.Float32x4.extractLane(value, 1);
    _f32x4[2] = SIMD.Float32x4.extractLane(value, 2);
    _f32x4[3] = SIMD.Float32x4.extractLane(value, 3);
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : _i32x4) :
                _f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      tarray[index + i] = array[i];
    return value;
  }
}

if (typeof SIMD.Float32x4.store1 === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {Float32x4} value An instance of Float32x4.
    * @return {Float32x4} value
    */
  SIMD.Float32x4.store1 = function(tarray, index, value) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 4) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.Float32x4.check(value);
    if (bpe == 8) {
      // tarray's elements are too wide. Just create a new view; this is rare.
      var view = new Float32Array(tarray.buffer,
                                  tarray.byteOffset + index * 8, 1);
      view[0] = SIMD.Float32x4.extractLane(value, 0);
    } else {
      _f32x4[0] = SIMD.Float32x4.extractLane(value, 0);
      var array = bpe == 1 ? _i8x16 :
                  bpe == 2 ? _i16x8 :
                  (tarray instanceof Float32Array ? _f32x4 : _i32x4);
      var n = 4 / bpe;
      for (var i = 0; i < n; ++i)
        tarray[index + i] = array[i];
      return value;
    }
  }
}

if (typeof SIMD.Float32x4.store2 === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {Float32x4} value An instance of Float32x4.
    * @return {Float32x4} value
    */
  SIMD.Float32x4.store2 = function(tarray, index, value) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 8) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.Float32x4.check(value);
    _f32x4[0] = SIMD.Float32x4.extractLane(value, 0);
    _f32x4[1] = SIMD.Float32x4.extractLane(value, 1);
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : _i32x4) :
                _f64x2;
    var n = 8 / bpe;
    for (var i = 0; i < n; ++i)
      tarray[index + i] = array[i];
    return value;
  }
}

if (typeof SIMD.Float32x4.store3 === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {Float32x4} value An instance of Float32x4.
    * @return {Float32x4} value
    */
  SIMD.Float32x4.store3 = function(tarray, index, value) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 12) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.Float32x4.check(value);
    if (bpe == 8) {
      // tarray's elements are too wide. Just create a new view; this is rare.
      var view = new Float32Array(tarray.buffer,
                                  tarray.byteOffset + index * 8, 3);
      view[0] = SIMD.Float32x4.extractLane(value, 0);
      view[1] = SIMD.Float32x4.extractLane(value, 1);
      view[2] = SIMD.Float32x4.extractLane(value, 2);
    } else {
      _f32x4[0] = SIMD.Float32x4.extractLane(value, 0);
      _f32x4[1] = SIMD.Float32x4.extractLane(value, 1);
      _f32x4[2] = SIMD.Float32x4.extractLane(value, 2);
      var array = bpe == 1 ? _i8x16 :
                  bpe == 2 ? _i16x8 :
                  (tarray instanceof Float32Array ? _f32x4 : _i32x4);
      var n = 12 / bpe;
      for (var i = 0; i < n; ++i)
        tarray[index + i] = array[i];
      return value;
    }
  }
}


if (typeof SIMD.Int32x4.and === "undefined") {
  /**
    * @param {Int32x4} a An instance of Int32x4.
    * @param {Int32x4} b An instance of Int32x4.
    * @return {Int32x4} New instance of Int32x4 with values of a & b.
    */
  SIMD.Int32x4.and = function(a, b) {
    a = SIMD.Int32x4.check(a);
    b = SIMD.Int32x4.check(b);
    return SIMD.Int32x4(
        SIMD.Int32x4.extractLane(a, 0) & SIMD.Int32x4.extractLane(b, 0),
        SIMD.Int32x4.extractLane(a, 1) & SIMD.Int32x4.extractLane(b, 1),
        SIMD.Int32x4.extractLane(a, 2) & SIMD.Int32x4.extractLane(b, 2),
        SIMD.Int32x4.extractLane(a, 3) & SIMD.Int32x4.extractLane(b, 3));
  }
}

if (typeof SIMD.Int32x4.or === "undefined") {
  /**
    * @param {Int32x4} a An instance of Int32x4.
    * @param {Int32x4} b An instance of Int32x4.
    * @return {Int32x4} New instance of Int32x4 with values of a | b.
    */
  SIMD.Int32x4.or = function(a, b) {
    a = SIMD.Int32x4.check(a);
    b = SIMD.Int32x4.check(b);
    return SIMD.Int32x4(
        SIMD.Int32x4.extractLane(a, 0) | SIMD.Int32x4.extractLane(b, 0),
        SIMD.Int32x4.extractLane(a, 1) | SIMD.Int32x4.extractLane(b, 1),
        SIMD.Int32x4.extractLane(a, 2) | SIMD.Int32x4.extractLane(b, 2),
        SIMD.Int32x4.extractLane(a, 3) | SIMD.Int32x4.extractLane(b, 3));
  }
}

if (typeof SIMD.Int32x4.xor === "undefined") {
  /**
    * @param {Int32x4} a An instance of Int32x4.
    * @param {Int32x4} b An instance of Int32x4.
    * @return {Int32x4} New instance of Int32x4 with values of a ^ b.
    */
  SIMD.Int32x4.xor = function(a, b) {
    a = SIMD.Int32x4.check(a);
    b = SIMD.Int32x4.check(b);
    return SIMD.Int32x4(
        SIMD.Int32x4.extractLane(a, 0) ^ SIMD.Int32x4.extractLane(b, 0),
        SIMD.Int32x4.extractLane(a, 1) ^ SIMD.Int32x4.extractLane(b, 1),
        SIMD.Int32x4.extractLane(a, 2) ^ SIMD.Int32x4.extractLane(b, 2),
        SIMD.Int32x4.extractLane(a, 3) ^ SIMD.Int32x4.extractLane(b, 3));
  }
}

if (typeof SIMD.Int32x4.not === "undefined") {
  /**
    * @param {Int32x4} t An instance of Int32x4.
    * @return {Int32x4} New instance of Int32x4 with values of ~t
    */
  SIMD.Int32x4.not = function(t) {
    t = SIMD.Int32x4.check(t);
    return SIMD.Int32x4(~SIMD.Int32x4.extractLane(t, 0),
                        ~SIMD.Int32x4.extractLane(t, 1),
                        ~SIMD.Int32x4.extractLane(t, 2),
                        ~SIMD.Int32x4.extractLane(t, 3));
  }
}

if (typeof SIMD.Int32x4.neg === "undefined") {
  /**
    * @param {Int32x4} t An instance of Int32x4.
    * @return {Int32x4} New instance of Int32x4 with values of -t
    */
  SIMD.Int32x4.neg = function(t) {
    t = SIMD.Int32x4.check(t);
    return SIMD.Int32x4(-SIMD.Int32x4.extractLane(t, 0),
                        -SIMD.Int32x4.extractLane(t, 1),
                        -SIMD.Int32x4.extractLane(t, 2),
                        -SIMD.Int32x4.extractLane(t, 3));
  }
}

if (typeof SIMD.Int32x4.add === "undefined") {
  /**
    * @param {Int32x4} a An instance of Int32x4.
    * @param {Int32x4} b An instance of Int32x4.
    * @return {Int32x4} New instance of Int32x4 with values of a + b.
    */
  SIMD.Int32x4.add = function(a, b) {
    a = SIMD.Int32x4.check(a);
    b = SIMD.Int32x4.check(b);
    return SIMD.Int32x4(
        SIMD.Int32x4.extractLane(a, 0) + SIMD.Int32x4.extractLane(b, 0),
        SIMD.Int32x4.extractLane(a, 1) + SIMD.Int32x4.extractLane(b, 1),
        SIMD.Int32x4.extractLane(a, 2) + SIMD.Int32x4.extractLane(b, 2),
        SIMD.Int32x4.extractLane(a, 3) + SIMD.Int32x4.extractLane(b, 3));
  }
}

if (typeof SIMD.Int32x4.sub === "undefined") {
  /**
    * @param {Int32x4} a An instance of Int32x4.
    * @param {Int32x4} b An instance of Int32x4.
    * @return {Int32x4} New instance of Int32x4 with values of a - b.
    */
  SIMD.Int32x4.sub = function(a, b) {
    a = SIMD.Int32x4.check(a);
    b = SIMD.Int32x4.check(b);
    return SIMD.Int32x4(
        SIMD.Int32x4.extractLane(a, 0) - SIMD.Int32x4.extractLane(b, 0),
        SIMD.Int32x4.extractLane(a, 1) - SIMD.Int32x4.extractLane(b, 1),
        SIMD.Int32x4.extractLane(a, 2) - SIMD.Int32x4.extractLane(b, 2),
        SIMD.Int32x4.extractLane(a, 3) - SIMD.Int32x4.extractLane(b, 3));
  }
}

if (typeof SIMD.Int32x4.mul === "undefined") {
  /**
    * @param {Int32x4} a An instance of Int32x4.
    * @param {Int32x4} b An instance of Int32x4.
    * @return {Int32x4} New instance of Int32x4 with values of a * b.
    */
  SIMD.Int32x4.mul = function(a, b) {
    a = SIMD.Int32x4.check(a);
    b = SIMD.Int32x4.check(b);
    return SIMD.Int32x4(
        Math.imul(SIMD.Int32x4.extractLane(a, 0),
                  SIMD.Int32x4.extractLane(b, 0)),
        Math.imul(SIMD.Int32x4.extractLane(a, 1),
                  SIMD.Int32x4.extractLane(b, 1)),
        Math.imul(SIMD.Int32x4.extractLane(a, 2),
                  SIMD.Int32x4.extractLane(b, 2)),
        Math.imul(SIMD.Int32x4.extractLane(a, 3),
                  SIMD.Int32x4.extractLane(b, 3)));
  }
}

if (typeof SIMD.Int32x4.swizzle === "undefined") {
  /**
    * @param {Int32x4} t An instance of Int32x4 to be swizzled.
    * @param {integer} x - Index in t for lane x
    * @param {integer} y - Index in t for lane y
    * @param {integer} z - Index in t for lane z
    * @param {integer} w - Index in t for lane w
    * @return {Int32x4} New instance of Int32x4 with lanes swizzled.
    */
  SIMD.Int32x4.swizzle = function(t, x, y, z, w) {
    t = SIMD.Int32x4.check(t);
    check4(x);
    check4(y);
    check4(z);
    check4(w);
    var storage = _i32x4;
    storage[0] = SIMD.Int32x4.extractLane(t, 0);
    storage[1] = SIMD.Int32x4.extractLane(t, 1);
    storage[2] = SIMD.Int32x4.extractLane(t, 2);
    storage[3] = SIMD.Int32x4.extractLane(t, 3);
    return SIMD.Int32x4(storage[x], storage[y], storage[z], storage[w]);
  }
}

if (typeof SIMD.Int32x4.shuffle === "undefined") {

  _i32x8 = new Int32Array(8);

  /**
    * @param {Int32x4} t1 An instance of Int32x4 to be shuffled.
    * @param {Int32x4} t2 An instance of Int32x4 to be shuffled.
    * @param {integer} x - Index in concatenation of t1 and t2 for lane x
    * @param {integer} y - Index in concatenation of t1 and t2 for lane y
    * @param {integer} z - Index in concatenation of t1 and t2 for lane z
    * @param {integer} w - Index in concatenation of t1 and t2 for lane w
    * @return {Int32x4} New instance of Int32x4 with lanes shuffled.
    */
  SIMD.Int32x4.shuffle = function(t1, t2, x, y, z, w) {
    t1 = SIMD.Int32x4.check(t1);
    t2 = SIMD.Int32x4.check(t2);
    check8(x);
    check8(y);
    check8(z);
    check8(w);
    var storage = _i32x8;
    storage[0] = SIMD.Int32x4.extractLane(t1, 0);
    storage[1] = SIMD.Int32x4.extractLane(t1, 1);
    storage[2] = SIMD.Int32x4.extractLane(t1, 2);
    storage[3] = SIMD.Int32x4.extractLane(t1, 3);
    storage[4] = SIMD.Int32x4.extractLane(t2, 0);
    storage[5] = SIMD.Int32x4.extractLane(t2, 1);
    storage[6] = SIMD.Int32x4.extractLane(t2, 2);
    storage[7] = SIMD.Int32x4.extractLane(t2, 3);
    return SIMD.Int32x4(storage[x], storage[y], storage[z], storage[w]);
  }
}

if (typeof SIMD.Int32x4.unsignedHorizontalSum === "undefined") {
  /**
    * @param {Int32x4} a An instance of 32x4.
    * @return {Number} The sum of all the lanes in a, extracted as unsigned values.
    */
  SIMD.Int32x4.unsignedHorizontalSum = function(a) {
    a = SIMD.Int32x4.check(a);
    return (SIMD.Int32x4.extractLane(a, 0)>>>0) +
           (SIMD.Int32x4.extractLane(a, 1)>>>0) +
           (SIMD.Int32x4.extractLane(a, 2)>>>0) +
           (SIMD.Int32x4.extractLane(a, 3)>>>0);
  }
}

if (typeof SIMD.Int32x4.select === "undefined") {
  /**
    * @param {Bool32x4} t Selector mask. An instance of Bool32x4
    * @param {Int32x4} trueValue Pick lane from here if corresponding
    * selector lane is true
    * @param {Int32x4} falseValue Pick lane from here if corresponding
    * selector lane is false
    * @return {Int32x4} Mix of lanes from trueValue or falseValue as
    * indicated
    */
  SIMD.Int32x4.select = function(t, trueValue, falseValue) {
    t = SIMD.Bool32x4.check(t);
    trueValue = SIMD.Int32x4.check(trueValue);
    falseValue = SIMD.Int32x4.check(falseValue);
    return SIMD.Int32x4(
        SIMD.Bool32x4.extractLane(t, 0) ?
            SIMD.Int32x4.extractLane(trueValue, 0) :
                SIMD.Int32x4.extractLane(falseValue, 0),
        SIMD.Bool32x4.extractLane(t, 1) ?
            SIMD.Int32x4.extractLane(trueValue, 1) :
                SIMD.Int32x4.extractLane(falseValue, 1),
        SIMD.Bool32x4.extractLane(t, 2) ?
            SIMD.Int32x4.extractLane(trueValue, 2) :
                SIMD.Int32x4.extractLane(falseValue, 2),
        SIMD.Bool32x4.extractLane(t, 3) ?
            SIMD.Int32x4.extractLane(trueValue, 3) :
                SIMD.Int32x4.extractLane(falseValue, 3));
  }
}

if (typeof SIMD.Int32x4.selectBits === "undefined") {
  /**
    * @param {Int32x4} t Selector mask. An instance of Int32x4
    * @param {Int32x4} trueValue Pick bit from here if corresponding
    * selector bit is 1
    * @param {Int32x4} falseValue Pick bit from here if corresponding
    * selector bit is 0
    * @return {Int32x4} Mix of bits from trueValue or falseValue as
    * indicated
    */
  SIMD.Int32x4.selectBits = function(t, trueValue, falseValue) {
    t = SIMD.Int32x4.check(t);
    trueValue = SIMD.Int32x4.check(trueValue);
    falseValue = SIMD.Int32x4.check(falseValue);
    var tr = SIMD.Int32x4.and(t, trueValue);
    var fr = SIMD.Int32x4.and(SIMD.Int32x4.not(t), falseValue);
    return SIMD.Int32x4.or(tr, fr);
  }
}

if (typeof SIMD.Int32x4.equal === "undefined") {
  /**
    * @param {Int32x4} t An instance of Int32x4.
    * @param {Int32x4} other An instance of Int32x4.
    * @return {Bool32x4} true or false in each lane depending on
    * the result of t == other.
    */
  SIMD.Int32x4.equal = function(t, other) {
    t = SIMD.Int32x4.check(t);
    other = SIMD.Int32x4.check(other);
    var cx =
        SIMD.Int32x4.extractLane(t, 0) == SIMD.Int32x4.extractLane(other, 0);
    var cy =
        SIMD.Int32x4.extractLane(t, 1) == SIMD.Int32x4.extractLane(other, 1);
    var cz =
        SIMD.Int32x4.extractLane(t, 2) == SIMD.Int32x4.extractLane(other, 2);
    var cw =
        SIMD.Int32x4.extractLane(t, 3) == SIMD.Int32x4.extractLane(other, 3);
    return SIMD.Bool32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Int32x4.notEqual === "undefined") {
  /**
    * @param {Int32x4} t An instance of Int32x4.
    * @param {Int32x4} other An instance of Int32x4.
    * @return {Bool32x4} true or false in each lane depending on
    * the result of t != other.
    */
  SIMD.Int32x4.notEqual = function(t, other) {
    t = SIMD.Int32x4.check(t);
    other = SIMD.Int32x4.check(other);
    var cx =
        SIMD.Int32x4.extractLane(t, 0) != SIMD.Int32x4.extractLane(other, 0);
    var cy =
        SIMD.Int32x4.extractLane(t, 1) != SIMD.Int32x4.extractLane(other, 1);
    var cz =
        SIMD.Int32x4.extractLane(t, 2) != SIMD.Int32x4.extractLane(other, 2);
    var cw =
        SIMD.Int32x4.extractLane(t, 3) != SIMD.Int32x4.extractLane(other, 3);
    return SIMD.Bool32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Int32x4.greaterThan === "undefined") {
  /**
    * @param {Int32x4} t An instance of Int32x4.
    * @param {Int32x4} other An instance of Int32x4.
    * @return {Bool32x4} true or false in each lane depending on
    * the result of t > other.
    */
  SIMD.Int32x4.greaterThan = function(t, other) {
    t = SIMD.Int32x4.check(t);
    other = SIMD.Int32x4.check(other);
    var cx =
        SIMD.Int32x4.extractLane(t, 0) > SIMD.Int32x4.extractLane(other, 0);
    var cy =
        SIMD.Int32x4.extractLane(t, 1) > SIMD.Int32x4.extractLane(other, 1);
    var cz =
        SIMD.Int32x4.extractLane(t, 2) > SIMD.Int32x4.extractLane(other, 2);
    var cw =
        SIMD.Int32x4.extractLane(t, 3) > SIMD.Int32x4.extractLane(other, 3);
    return SIMD.Bool32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Int32x4.greaterThanOrEqual === "undefined") {
  /**
    * @param {Int32x4} t An instance of Int32x4.
    * @param {Int32x4} other An instance of Int32x4.
    * @return {Bool32x4} true or false in each lane depending on
    * the result of t >= other.
    */
  SIMD.Int32x4.greaterThanOrEqual = function(t, other) {
    t = SIMD.Int32x4.check(t);
    other = SIMD.Int32x4.check(other);
    var cx =
        SIMD.Int32x4.extractLane(t, 0) >= SIMD.Int32x4.extractLane(other, 0);
    var cy =
        SIMD.Int32x4.extractLane(t, 1) >= SIMD.Int32x4.extractLane(other, 1);
    var cz =
        SIMD.Int32x4.extractLane(t, 2) >= SIMD.Int32x4.extractLane(other, 2);
    var cw =
        SIMD.Int32x4.extractLane(t, 3) >= SIMD.Int32x4.extractLane(other, 3);
    return SIMD.Bool32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Int32x4.lessThan === "undefined") {
  /**
    * @param {Int32x4} t An instance of Int32x4.
    * @param {Int32x4} other An instance of Int32x4.
    * @return {Bool32x4} true or false in each lane depending on
    * the result of t < other.
    */
  SIMD.Int32x4.lessThan = function(t, other) {
    t = SIMD.Int32x4.check(t);
    other = SIMD.Int32x4.check(other);
    var cx =
        SIMD.Int32x4.extractLane(t, 0) < SIMD.Int32x4.extractLane(other, 0);
    var cy =
        SIMD.Int32x4.extractLane(t, 1) < SIMD.Int32x4.extractLane(other, 1);
    var cz =
        SIMD.Int32x4.extractLane(t, 2) < SIMD.Int32x4.extractLane(other, 2);
    var cw =
        SIMD.Int32x4.extractLane(t, 3) < SIMD.Int32x4.extractLane(other, 3);
    return SIMD.Bool32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Int32x4.lessThanOrEqual === "undefined") {
  /**
    * @param {Int32x4} t An instance of Int32x4.
    * @param {Int32x4} other An instance of Int32x4.
    * @return {Bool32x4} true or false in each lane depending on
    * the result of t <= other.
    */
  SIMD.Int32x4.lessThanOrEqual = function(t, other) {
    t = SIMD.Int32x4.check(t);
    other = SIMD.Int32x4.check(other);
    var cx =
        SIMD.Int32x4.extractLane(t, 0) <= SIMD.Int32x4.extractLane(other, 0);
    var cy =
        SIMD.Int32x4.extractLane(t, 1) <= SIMD.Int32x4.extractLane(other, 1);
    var cz =
        SIMD.Int32x4.extractLane(t, 2) <= SIMD.Int32x4.extractLane(other, 2);
    var cw =
        SIMD.Int32x4.extractLane(t, 3) <= SIMD.Int32x4.extractLane(other, 3);
    return SIMD.Bool32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.Int32x4.shiftLeftByScalar === "undefined") {
  /**
    * @param {Int32x4} a An instance of Int32x4.
    * @param {integer} bits Bit count to shift by.
    * @return {Int32x4} lanes in a shifted by bits.
    */
  SIMD.Int32x4.shiftLeftByScalar = function(a, bits) {
    a = SIMD.Int32x4.check(a);
    if (bits>>>0 >= 32)
      return SIMD.Int32x4.splat(0.0);
    var x = SIMD.Int32x4.extractLane(a, 0) << bits;
    var y = SIMD.Int32x4.extractLane(a, 1) << bits;
    var z = SIMD.Int32x4.extractLane(a, 2) << bits;
    var w = SIMD.Int32x4.extractLane(a, 3) << bits;
    return SIMD.Int32x4(x, y, z, w);
  }
}

if (typeof SIMD.Int32x4.shiftRightLogicalByScalar === "undefined") {
  /**
    * @param {Int32x4} a An instance of Int32x4.
    * @param {integer} bits Bit count to shift by.
    * @return {Int32x4} lanes in a shifted by bits.
    */
  SIMD.Int32x4.shiftRightLogicalByScalar = function(a, bits) {
    a = SIMD.Int32x4.check(a);
    if (bits>>>0 >= 32)
      return SIMD.Int32x4.splat(0.0);
    var x = SIMD.Int32x4.extractLane(a, 0) >>> bits;
    var y = SIMD.Int32x4.extractLane(a, 1) >>> bits;
    var z = SIMD.Int32x4.extractLane(a, 2) >>> bits;
    var w = SIMD.Int32x4.extractLane(a, 3) >>> bits;
    return SIMD.Int32x4(x, y, z, w);
  }
}

if (typeof SIMD.Int32x4.shiftRightArithmeticByScalar === "undefined") {
  /**
    * @param {Int32x4} a An instance of Int32x4.
    * @param {integer} bits Bit count to shift by.
    * @return {Int32x4} lanes in a shifted by bits.
    */
  SIMD.Int32x4.shiftRightArithmeticByScalar = function(a, bits) {
    a = SIMD.Int32x4.check(a);
    if (bits>>>0 >= 32)
      bits = 31;
    var x = SIMD.Int32x4.extractLane(a, 0) >> bits;
    var y = SIMD.Int32x4.extractLane(a, 1) >> bits;
    var z = SIMD.Int32x4.extractLane(a, 2) >> bits;
    var w = SIMD.Int32x4.extractLane(a, 3) >> bits;
    return SIMD.Int32x4(x, y, z, w);
  }
}

if (typeof SIMD.Int32x4.load === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {Int32x4} New instance of Int32x4.
    */
  SIMD.Int32x4.load = function(tarray, index) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var i32temp = _i32x4;
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : i32temp) :
                _f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.Int32x4(i32temp[0], i32temp[1], i32temp[2], i32temp[3]);
  }
}

if (typeof SIMD.Int32x4.load1 === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {Int32x4} New instance of Int32x4.
    */
  SIMD.Int32x4.load1 = function(tarray, index) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 4) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var i32temp = _i32x4;
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : i32temp) :
                _f64x2;
    var n = 4 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.Int32x4(i32temp[0], 0, 0, 0);
  }
}

if (typeof SIMD.Int32x4.load2 === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {Int32x4} New instance of Int32x4.
    */
  SIMD.Int32x4.load2 = function(tarray, index) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 8) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var i32temp = _i32x4;
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : i32temp) :
                _f64x2;
    var n = 8 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.Int32x4(i32temp[0], i32temp[1], 0, 0);
  }
}

if (typeof SIMD.Int32x4.load3 === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {Int32x4} New instance of Int32x4.
    */
  SIMD.Int32x4.load3 = function(tarray, index) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 12) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var i32temp = _i32x4;
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : i32temp) :
                _f64x2;
    var n = 12 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.Int32x4(i32temp[0], i32temp[1], i32temp[2], 0);
  }
}

if (typeof SIMD.Int32x4.store === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {Int32x4} value An instance of Int32x4.
    * @return {Int32x4} value
    */
  SIMD.Int32x4.store = function(tarray, index, value) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.Int32x4.check(value);
    _i32x4[0] = SIMD.Int32x4.extractLane(value, 0);
    _i32x4[1] = SIMD.Int32x4.extractLane(value, 1);
    _i32x4[2] = SIMD.Int32x4.extractLane(value, 2);
    _i32x4[3] = SIMD.Int32x4.extractLane(value, 3);
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : _i32x4) :
                _f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      tarray[index + i] = array[i];
    return value;
  }
}

if (typeof SIMD.Int32x4.store1 === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {Int32x4} value An instance of Int32x4.
    * @return {Int32x4} value
    */
  SIMD.Int32x4.store1 = function(tarray, index, value) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 4) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.Int32x4.check(value);
    if (bpe == 8) {
      // tarray's elements are too wide. Just create a new view; this is rare.
      var view = new Int32Array(tarray.buffer,
                                tarray.byteOffset + index * 8, 1);
      view[0] = SIMD.Int32x4.extractLane(value, 0);
    } else {
      _i32x4[0] = SIMD.Int32x4.extractLane(value, 0);
      var array = bpe == 1 ? _i8x16 :
                  bpe == 2 ? _i16x8 :
                  (tarray instanceof Float32Array ? _f32x4 : _i32x4);
      var n = 4 / bpe;
      for (var i = 0; i < n; ++i)
        tarray[index + i] = array[i];
      return value;
    }
  }
}

if (typeof SIMD.Int32x4.store2 === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {Int32x4} value An instance of Int32x4.
    * @return {Int32x4} value
    */
  SIMD.Int32x4.store2 = function(tarray, index, value) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 8) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.Int32x4.check(value);
    _i32x4[0] = SIMD.Int32x4.extractLane(value, 0);
    _i32x4[1] = SIMD.Int32x4.extractLane(value, 1);
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : _i32x4) :
                _f64x2;
    var n = 8 / bpe;
    for (var i = 0; i < n; ++i)
      tarray[index + i] = array[i];
    return value;
  }
}

if (typeof SIMD.Int32x4.store3 === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {Int32x4} value An instance of Int32x4.
    * @return {Int32x4} value
    */
  SIMD.Int32x4.store3 = function(tarray, index, value) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 12) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.Int32x4.check(value);
    if (bpe == 8) {
      // tarray's elements are too wide. Just create a new view; this is rare.
      var view = new Int32Array(tarray.buffer,
                                tarray.byteOffset + index * 8, 3);
      view[0] = SIMD.Int32x4.extractLane(value, 0);
      view[1] = SIMD.Int32x4.extractLane(value, 1);
      view[2] = SIMD.Int32x4.extractLane(value, 2);
    } else {
      _i32x4[0] = SIMD.Int32x4.extractLane(value, 0);
      _i32x4[1] = SIMD.Int32x4.extractLane(value, 1);
      _i32x4[2] = SIMD.Int32x4.extractLane(value, 2);
      var array = bpe == 1 ? _i8x16 :
                  bpe == 2 ? _i16x8 :
                  (tarray instanceof Float32Array ? _f32x4 : _i32x4);
      var n = 12 / bpe;
      for (var i = 0; i < n; ++i)
        tarray[index + i] = array[i];
      return value;
    }
  }
}

if (typeof SIMD.Int16x8.and === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @param {Int16x8} b An instance of Int16x8.
    * @return {Int16x8} New instance of Int16x8 with values of a & b.
    */
  SIMD.Int16x8.and = function(a, b) {
    a = SIMD.Int16x8.check(a);
    b = SIMD.Int16x8.check(b);
    return SIMD.Int16x8(
        SIMD.Int16x8.extractLane(a, 0) & SIMD.Int16x8.extractLane(b, 0),
        SIMD.Int16x8.extractLane(a, 1) & SIMD.Int16x8.extractLane(b, 1),
        SIMD.Int16x8.extractLane(a, 2) & SIMD.Int16x8.extractLane(b, 2),
        SIMD.Int16x8.extractLane(a, 3) & SIMD.Int16x8.extractLane(b, 3),
        SIMD.Int16x8.extractLane(a, 4) & SIMD.Int16x8.extractLane(b, 4),
        SIMD.Int16x8.extractLane(a, 5) & SIMD.Int16x8.extractLane(b, 5),
        SIMD.Int16x8.extractLane(a, 6) & SIMD.Int16x8.extractLane(b, 6),
        SIMD.Int16x8.extractLane(a, 7) & SIMD.Int16x8.extractLane(b, 7));
  }
}

if (typeof SIMD.Int16x8.or === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @param {Int16x8} b An instance of Int16x8.
    * @return {Int16x8} New instance of Int16x8 with values of a | b.
    */
  SIMD.Int16x8.or = function(a, b) {
    a = SIMD.Int16x8.check(a);
    b = SIMD.Int16x8.check(b);
    return SIMD.Int16x8(
        SIMD.Int16x8.extractLane(a, 0) | SIMD.Int16x8.extractLane(b, 0),
        SIMD.Int16x8.extractLane(a, 1) | SIMD.Int16x8.extractLane(b, 1),
        SIMD.Int16x8.extractLane(a, 2) | SIMD.Int16x8.extractLane(b, 2),
        SIMD.Int16x8.extractLane(a, 3) | SIMD.Int16x8.extractLane(b, 3),
        SIMD.Int16x8.extractLane(a, 4) | SIMD.Int16x8.extractLane(b, 4),
        SIMD.Int16x8.extractLane(a, 5) | SIMD.Int16x8.extractLane(b, 5),
        SIMD.Int16x8.extractLane(a, 6) | SIMD.Int16x8.extractLane(b, 6),
        SIMD.Int16x8.extractLane(a, 7) | SIMD.Int16x8.extractLane(b, 7));
  }
}

if (typeof SIMD.Int16x8.xor === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @param {Int16x8} b An instance of Int16x8.
    * @return {Int16x8} New instance of Int16x8 with values of a ^ b.
    */
  SIMD.Int16x8.xor = function(a, b) {
    a = SIMD.Int16x8.check(a);
    b = SIMD.Int16x8.check(b);
    return SIMD.Int16x8(
        SIMD.Int16x8.extractLane(a, 0) ^ SIMD.Int16x8.extractLane(b, 0),
        SIMD.Int16x8.extractLane(a, 1) ^ SIMD.Int16x8.extractLane(b, 1),
        SIMD.Int16x8.extractLane(a, 2) ^ SIMD.Int16x8.extractLane(b, 2),
        SIMD.Int16x8.extractLane(a, 3) ^ SIMD.Int16x8.extractLane(b, 3),
        SIMD.Int16x8.extractLane(a, 4) ^ SIMD.Int16x8.extractLane(b, 4),
        SIMD.Int16x8.extractLane(a, 5) ^ SIMD.Int16x8.extractLane(b, 5),
        SIMD.Int16x8.extractLane(a, 6) ^ SIMD.Int16x8.extractLane(b, 6),
        SIMD.Int16x8.extractLane(a, 7) ^ SIMD.Int16x8.extractLane(b, 7));
  }
}

if (typeof SIMD.Int16x8.not === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @return {Int16x8} New instance of Int16x8 with values of ~t
    */
  SIMD.Int16x8.not = function(t) {
    t = SIMD.Int16x8.check(t);
    return SIMD.Int16x8(~SIMD.Int16x8.extractLane(t, 0),
                        ~SIMD.Int16x8.extractLane(t, 1),
                        ~SIMD.Int16x8.extractLane(t, 2),
                        ~SIMD.Int16x8.extractLane(t, 3),
                        ~SIMD.Int16x8.extractLane(t, 4),
                        ~SIMD.Int16x8.extractLane(t, 5),
                        ~SIMD.Int16x8.extractLane(t, 6),
                        ~SIMD.Int16x8.extractLane(t, 7));
  }
}

if (typeof SIMD.Int16x8.neg === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @return {Int16x8} New instance of Int16x8 with values of -t
    */
  SIMD.Int16x8.neg = function(t) {
    t = SIMD.Int16x8.check(t);
    return SIMD.Int16x8(-SIMD.Int16x8.extractLane(t, 0),
                        -SIMD.Int16x8.extractLane(t, 1),
                        -SIMD.Int16x8.extractLane(t, 2),
                        -SIMD.Int16x8.extractLane(t, 3),
                        -SIMD.Int16x8.extractLane(t, 4),
                        -SIMD.Int16x8.extractLane(t, 5),
                        -SIMD.Int16x8.extractLane(t, 6),
                        -SIMD.Int16x8.extractLane(t, 7));
  }
}

if (typeof SIMD.Int16x8.add === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @param {Int16x8} b An instance of Int16x8.
    * @return {Int16x8} New instance of Int16x8 with values of a + b.
    */
  SIMD.Int16x8.add = function(a, b) {
    a = SIMD.Int16x8.check(a);
    b = SIMD.Int16x8.check(b);
    return SIMD.Int16x8(
        SIMD.Int16x8.extractLane(a, 0) + SIMD.Int16x8.extractLane(b, 0),
        SIMD.Int16x8.extractLane(a, 1) + SIMD.Int16x8.extractLane(b, 1),
        SIMD.Int16x8.extractLane(a, 2) + SIMD.Int16x8.extractLane(b, 2),
        SIMD.Int16x8.extractLane(a, 3) + SIMD.Int16x8.extractLane(b, 3),
        SIMD.Int16x8.extractLane(a, 4) + SIMD.Int16x8.extractLane(b, 4),
        SIMD.Int16x8.extractLane(a, 5) + SIMD.Int16x8.extractLane(b, 5),
        SIMD.Int16x8.extractLane(a, 6) + SIMD.Int16x8.extractLane(b, 6),
        SIMD.Int16x8.extractLane(a, 7) + SIMD.Int16x8.extractLane(b, 7));
  }
}

if (typeof SIMD.Int16x8.sub === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @param {Int16x8} b An instance of Int16x8.
    * @return {Int16x8} New instance of Int16x8 with values of a - b.
    */
  SIMD.Int16x8.sub = function(a, b) {
    a = SIMD.Int16x8.check(a);
    b = SIMD.Int16x8.check(b);
    return SIMD.Int16x8(
        SIMD.Int16x8.extractLane(a, 0) - SIMD.Int16x8.extractLane(b, 0),
        SIMD.Int16x8.extractLane(a, 1) - SIMD.Int16x8.extractLane(b, 1),
        SIMD.Int16x8.extractLane(a, 2) - SIMD.Int16x8.extractLane(b, 2),
        SIMD.Int16x8.extractLane(a, 3) - SIMD.Int16x8.extractLane(b, 3),
        SIMD.Int16x8.extractLane(a, 4) - SIMD.Int16x8.extractLane(b, 4),
        SIMD.Int16x8.extractLane(a, 5) - SIMD.Int16x8.extractLane(b, 5),
        SIMD.Int16x8.extractLane(a, 6) - SIMD.Int16x8.extractLane(b, 6),
        SIMD.Int16x8.extractLane(a, 7) - SIMD.Int16x8.extractLane(b, 7));
  }
}

if (typeof SIMD.Int16x8.mul === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @param {Int16x8} b An instance of Int16x8.
    * @return {Int16x8} New instance of Int16x8 with values of a * b.
    */
  SIMD.Int16x8.mul = function(a, b) {
    a = SIMD.Int16x8.check(a);
    b = SIMD.Int16x8.check(b);
    return SIMD.Int16x8(Math.imul(SIMD.Int16x8.extractLane(a, 0),
                                  SIMD.Int16x8.extractLane(b, 0)),
                        Math.imul(SIMD.Int16x8.extractLane(a, 1),
                                  SIMD.Int16x8.extractLane(b, 1)),
                        Math.imul(SIMD.Int16x8.extractLane(a, 2),
                                  SIMD.Int16x8.extractLane(b, 2)),
                        Math.imul(SIMD.Int16x8.extractLane(a, 3),
                                  SIMD.Int16x8.extractLane(b, 3)),
                        Math.imul(SIMD.Int16x8.extractLane(a, 4),
                                  SIMD.Int16x8.extractLane(b, 4)),
                        Math.imul(SIMD.Int16x8.extractLane(a, 5),
                                  SIMD.Int16x8.extractLane(b, 5)),
                        Math.imul(SIMD.Int16x8.extractLane(a, 6),
                                  SIMD.Int16x8.extractLane(b, 6)),
                        Math.imul(SIMD.Int16x8.extractLane(a, 7),
                                  SIMD.Int16x8.extractLane(b, 7)));
  }
}

if (typeof SIMD.Int16x8.swizzle === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8 to be swizzled.
    * @param {integer} s0 - Index in t for lane s0
    * @param {integer} s1 - Index in t for lane s1
    * @param {integer} s2 - Index in t for lane s2
    * @param {integer} s3 - Index in t for lane s3
    * @param {integer} s4 - Index in t for lane s4
    * @param {integer} s5 - Index in t for lane s5
    * @param {integer} s6 - Index in t for lane s6
    * @param {integer} s7 - Index in t for lane s7
    * @return {Int16x8} New instance of Int16x8 with lanes swizzled.
    */
  SIMD.Int16x8.swizzle = function(t, s0, s1, s2, s3, s4, s5, s6, s7) {
    t = SIMD.Int16x8.check(t);
    check8(s0);
    check8(s1);
    check8(s2);
    check8(s3);
    check8(s4);
    check8(s5);
    check8(s6);
    check8(s7);
    var storage = _i16x8;
    storage[0] = SIMD.Int16x8.extractLane(t, 0);
    storage[1] = SIMD.Int16x8.extractLane(t, 1);
    storage[2] = SIMD.Int16x8.extractLane(t, 2);
    storage[3] = SIMD.Int16x8.extractLane(t, 3);
    storage[4] = SIMD.Int16x8.extractLane(t, 4);
    storage[5] = SIMD.Int16x8.extractLane(t, 5);
    storage[6] = SIMD.Int16x8.extractLane(t, 6);
    storage[7] = SIMD.Int16x8.extractLane(t, 7);
    return SIMD.Int16x8(storage[s0], storage[s1], storage[s2], storage[s3],
                        storage[s4], storage[s5], storage[s6], storage[s7]);
  }
}

if (typeof SIMD.Int16x8.shuffle === "undefined") {

  _i16x16 = new Int16Array(16);

  /**
    * @param {Int16x8} t0 An instance of Int16x8 to be shuffled.
    * @param {Int16x8} t1 An instance of Int16x8 to be shuffled.
    * @param {integer} s0 - Index in concatenation of t0 and t1 for lane s0
    * @param {integer} s1 - Index in concatenation of t0 and t1 for lane s1
    * @param {integer} s2 - Index in concatenation of t0 and t1 for lane s2
    * @param {integer} s3 - Index in concatenation of t0 and t1 for lane s3
    * @param {integer} s4 - Index in concatenation of t0 and t1 for lane s4
    * @param {integer} s5 - Index in concatenation of t0 and t1 for lane s5
    * @param {integer} s6 - Index in concatenation of t0 and t1 for lane s6
    * @param {integer} s7 - Index in concatenation of t0 and t1 for lane s7
    * @return {Int16x8} New instance of Int16x8 with lanes shuffled.
    */
  SIMD.Int16x8.shuffle = function(t0, t1, s0, s1, s2, s3, s4, s5, s6, s7) {
    t0 = SIMD.Int16x8.check(t0);
    t1 = SIMD.Int16x8.check(t1);
    check16(s0);
    check16(s1);
    check16(s2);
    check16(s3);
    check16(s4);
    check16(s5);
    check16(s6);
    check16(s7);
    var storage = _i16x16;
    storage[0] = SIMD.Int16x8.extractLane(t0, 0);
    storage[1] = SIMD.Int16x8.extractLane(t0, 1);
    storage[2] = SIMD.Int16x8.extractLane(t0, 2);
    storage[3] = SIMD.Int16x8.extractLane(t0, 3);
    storage[4] = SIMD.Int16x8.extractLane(t0, 4);
    storage[5] = SIMD.Int16x8.extractLane(t0, 5);
    storage[6] = SIMD.Int16x8.extractLane(t0, 6);
    storage[7] = SIMD.Int16x8.extractLane(t0, 7);
    storage[8] = SIMD.Int16x8.extractLane(t1, 0);
    storage[9] = SIMD.Int16x8.extractLane(t1, 1);
    storage[10] = SIMD.Int16x8.extractLane(t1, 2);
    storage[11] = SIMD.Int16x8.extractLane(t1, 3);
    storage[12] = SIMD.Int16x8.extractLane(t1, 4);
    storage[13] = SIMD.Int16x8.extractLane(t1, 5);
    storage[14] = SIMD.Int16x8.extractLane(t1, 6);
    storage[15] = SIMD.Int16x8.extractLane(t1, 7);
    return SIMD.Int16x8(storage[s0], storage[s1], storage[s2], storage[s3],
                        storage[s4], storage[s5], storage[s6], storage[s7]);
  }
}

if (typeof SIMD.Int16x8.addSaturate === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @param {Int16x8} b An instance of Int16x8.
    * @return {Int16x8} New instance of Int16x8 with values of a + b with
    * signed saturating behavior on overflow.
    */
  SIMD.Int16x8.addSaturate = function(a, b) {
    a = SIMD.Int16x8.check(a);
    b = SIMD.Int16x8.check(b);
    var c = SIMD.Int16x8.add(a, b);
    var max = SIMD.Int16x8.splat(0x7fff);
    var min = SIMD.Int16x8.splat(0x8000);
    var mask = SIMD.Int16x8.lessThan(c, a);
    var bneg = SIMD.Int16x8.lessThan(b, SIMD.Int16x8.splat(0));
    return SIMD.Int16x8.select(SIMD.Bool16x8.and(mask, SIMD.Bool16x8.not(bneg)), max,
             SIMD.Int16x8.select(SIMD.Bool16x8.and(SIMD.Bool16x8.not(mask), bneg), min,
               c));
  }
}

if (typeof SIMD.Int16x8.unsignedAddSaturate === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @param {Int16x8} b An instance of Int16x8.
    * @return {Int16x8} New instance of Int16x8 with values of a + b with
    * unsigned saturating behavior on overflow.
    */
  SIMD.Int16x8.unsignedAddSaturate = function(a, b) {
    a = SIMD.Int16x8.check(a);
    b = SIMD.Int16x8.check(b);
    var c = SIMD.Int16x8.add(a, b);
    var max = SIMD.Int16x8.splat(0xffff);
    var min = SIMD.Int16x8.splat(0x0000);
    var mask = SIMD.Int16x8.unsignedLessThan(c, a);
    var bneg = SIMD.Int16x8.unsignedLessThan(b, SIMD.Int16x8.splat(0));
    return SIMD.Int16x8.select(SIMD.Bool16x8.and(mask, SIMD.Bool16x8.not(bneg)), max,
             SIMD.Int16x8.select(SIMD.Bool16x8.and(SIMD.Bool16x8.not(mask), bneg), min,
               c));
  }
}

if (typeof SIMD.Int16x8.subSaturate === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @param {Int16x8} b An instance of Int16x8.
    * @return {Int16x8} New instance of Int16x8 with values of a - b with
    * signed saturating behavior on overflow.
    */
  SIMD.Int16x8.subSaturate = function(a, b) {
    a = SIMD.Int16x8.check(a);
    b = SIMD.Int16x8.check(b);
    var c = SIMD.Int16x8.sub(a, b);
    var max = SIMD.Int16x8.splat(0x7fff);
    var min = SIMD.Int16x8.splat(0x8000);
    var mask = SIMD.Int16x8.greaterThan(c, a);
    var bneg = SIMD.Int16x8.lessThan(b, SIMD.Int16x8.splat(0));
    return SIMD.Int16x8.select(SIMD.Bool16x8.and(mask, SIMD.Bool16x8.not(bneg)), min,
             SIMD.Int16x8.select(SIMD.Bool16x8.and(SIMD.Bool16x8.not(mask), bneg), max,
               c));
  }
}

if (typeof SIMD.Int16x8.unsignedSubSaturate === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @param {Int16x8} b An instance of Int16x8.
    * @return {Int16x8} New instance of Int16x8 with values of a - b with
    * unsigned saturating behavior on overflow.
    */
  SIMD.Int16x8.unsignedSubSaturate = function(a, b) {
    a = SIMD.Int16x8.check(a);
    b = SIMD.Int16x8.check(b);
    var c = SIMD.Int16x8.sub(a, b);
    var max = SIMD.Int16x8.splat(0xffff);
    var min = SIMD.Int16x8.splat(0x0000);
    var mask = SIMD.Int16x8.unsignedGreaterThan(c, a);
    var bneg = SIMD.Int16x8.unsignedLessThan(b, SIMD.Int16x8.splat(0));
    return SIMD.Int16x8.select(SIMD.Bool16x8.and(mask, SIMD.Bool16x8.not(bneg)), min,
             SIMD.Int16x8.select(SIMD.Bool16x8.and(SIMD.Bool16x8.not(mask), bneg), max,
               c));
  }
}

if (typeof SIMD.Int16x8.unsignedAbsoluteDifference === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int8x16.
    * @param {Int16x8} b An instance of Int8x16.
    * @return {Int16x8} The absolute differences (abs(x - y)) of the
    * corresponding elements of a and b. x and y are interpreted as unsigned
    * integers.
    */
  SIMD.Int16x8.unsignedAbsoluteDifference = function(a, b) {
    a = SIMD.Int16x8.check(a);
    b = SIMD.Int16x8.check(b);
    var x = SIMD.Int16x8(
        Math.abs(
            SIMD.Int16x8.unsignedExtractLane(a, 0) - SIMD.Int16x8.unsignedExtractLane(b, 0)),
        Math.abs(
            SIMD.Int16x8.unsignedExtractLane(a, 1) - SIMD.Int16x8.unsignedExtractLane(b, 1)),
        Math.abs(
            SIMD.Int16x8.unsignedExtractLane(a, 2) - SIMD.Int16x8.unsignedExtractLane(b, 2)),
        Math.abs(
            SIMD.Int16x8.unsignedExtractLane(a, 3) - SIMD.Int16x8.unsignedExtractLane(b, 3)),
        Math.abs(
            SIMD.Int16x8.unsignedExtractLane(a, 4) - SIMD.Int16x8.unsignedExtractLane(b, 4)),
        Math.abs(
            SIMD.Int16x8.unsignedExtractLane(a, 5) - SIMD.Int16x8.unsignedExtractLane(b, 5)),
        Math.abs(
            SIMD.Int16x8.unsignedExtractLane(a, 6) - SIMD.Int16x8.unsignedExtractLane(b, 6)),
        Math.abs(
            SIMD.Int16x8.unsignedExtractLane(a, 7) - SIMD.Int16x8.unsignedExtractLane(b, 7)));
    return x;
  }
}

if (typeof SIMD.Int16x8.widenedUnsignedAbsoluteDifference === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @param {Int16x8} b An instance of Int16x8.
    * @return {Int32x4} The absolute differences (abs(x - y)) of the
    * first 4 corresponding elements of a and b, returning 32-bit results.
    * x and y are interpreted as unsigned integers.
    */
  SIMD.Int16x8.widenedUnsignedAbsoluteDifference = function(a, b) {
    a = SIMD.Int16x8.check(a);
    b = SIMD.Int16x8.check(b);
    return SIMD.Int32x4(
        Math.abs(
            SIMD.Int16x8.unsignedExtractLane(a, 0) - SIMD.Int16x8.unsignedExtractLane(b, 0)),
        Math.abs(
            SIMD.Int16x8.unsignedExtractLane(a, 1) - SIMD.Int16x8.unsignedExtractLane(b, 1)),
        Math.abs(
            SIMD.Int16x8.unsignedExtractLane(a, 2) - SIMD.Int16x8.unsignedExtractLane(b, 2)),
        Math.abs(
            SIMD.Int16x8.unsignedExtractLane(a, 3) - SIMD.Int16x8.unsignedExtractLane(b, 3)));
  }
}

if (typeof SIMD.Int16x8.unsignedHorizontalSum === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @return {Number} The sum of all the lanes in a, extracted as unsigned values.
    */
  SIMD.Int16x8.unsignedHorizontalSum = function(a) {
    a = SIMD.Int16x8.check(a);
    return SIMD.Int16x8.unsignedExtractLane(a, 0) +
           SIMD.Int16x8.unsignedExtractLane(a, 1) +
           SIMD.Int16x8.unsignedExtractLane(a, 2) +
           SIMD.Int16x8.unsignedExtractLane(a, 3) +
           SIMD.Int16x8.unsignedExtractLane(a, 4) +
           SIMD.Int16x8.unsignedExtractLane(a, 5) +
           SIMD.Int16x8.unsignedExtractLane(a, 6) +
           SIMD.Int16x8.unsignedExtractLane(a, 7);
  }
}

if (typeof SIMD.Int16x8.select === "undefined") {
  /**
    * @param {Bool16x8} t Selector mask. An instance of Bool16x8
    * @param {Int16x8} trueValue Pick lane from here if corresponding
    * selector lane is true
    * @param {Int16x8} falseValue Pick lane from here if corresponding
    * selector lane is false
    * @return {Int16x8} Mix of lanes from trueValue or falseValue as
    * indicated
    */
  SIMD.Int16x8.select = function(t, trueValue, falseValue) {
    t = SIMD.Bool16x8.check(t);
    trueValue = SIMD.Int16x8.check(trueValue);
    falseValue = SIMD.Int16x8.check(falseValue);
    return SIMD.Int16x8(
        SIMD.Bool16x8.extractLane(t, 0) ?
            SIMD.Int16x8.extractLane(trueValue, 0) :
                SIMD.Int16x8.extractLane(falseValue, 0),
        SIMD.Bool16x8.extractLane(t, 1) ?
            SIMD.Int16x8.extractLane(trueValue, 1) :
                SIMD.Int16x8.extractLane(falseValue, 1),
        SIMD.Bool16x8.extractLane(t, 2) ?
            SIMD.Int16x8.extractLane(trueValue, 2) :
                SIMD.Int16x8.extractLane(falseValue, 2),
        SIMD.Bool16x8.extractLane(t, 3) ?
            SIMD.Int16x8.extractLane(trueValue, 3) :
                SIMD.Int16x8.extractLane(falseValue, 3),
        SIMD.Bool16x8.extractLane(t, 4) ?
            SIMD.Int16x8.extractLane(trueValue, 4) :
                SIMD.Int16x8.extractLane(falseValue, 4),
        SIMD.Bool16x8.extractLane(t, 5) ?
            SIMD.Int16x8.extractLane(trueValue, 5) :
                SIMD.Int16x8.extractLane(falseValue, 5),
        SIMD.Bool16x8.extractLane(t, 6) ?
            SIMD.Int16x8.extractLane(trueValue, 6) :
                SIMD.Int16x8.extractLane(falseValue, 6),
        SIMD.Bool16x8.extractLane(t, 7) ?
            SIMD.Int16x8.extractLane(trueValue, 7) :
                SIMD.Int16x8.extractLane(falseValue, 7));
  }
}

if (typeof SIMD.Int16x8.selectBits === "undefined") {
  /**
    * @param {Int16x8} t Selector mask. An instance of Int16x8
    * @param {Int16x8} trueValue Pick bit from here if corresponding
    * selector bit is 1
    * @param {Int16x8} falseValue Pick bit from here if corresponding
    * selector bit is 0
    * @return {Int16x8} Mix of bits from trueValue or falseValue as
    * indicated
    */
  SIMD.Int16x8.selectBits = function(t, trueValue, falseValue) {
    t = SIMD.Int16x8.check(t);
    trueValue = SIMD.Int16x8.check(trueValue);
    falseValue = SIMD.Int16x8.check(falseValue);
    var tr = SIMD.Int16x8.and(t, trueValue);
    var fr = SIMD.Int16x8.and(SIMD.Int16x8.not(t), falseValue);
    return SIMD.Int16x8.or(tr, fr);
  }
}

if (typeof SIMD.Int16x8.equal === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @param {Int16x8} other An instance of Int16x8.
    * @return {Bool16x8} true or false in each lane depending on
    * the result of t == other.
    */
  SIMD.Int16x8.equal = function(t, other) {
    t = SIMD.Int16x8.check(t);
    other = SIMD.Int16x8.check(other);
    var cs0 =
        SIMD.Int16x8.extractLane(t, 0) == SIMD.Int16x8.extractLane(other, 0);
    var cs1 =
        SIMD.Int16x8.extractLane(t, 1) == SIMD.Int16x8.extractLane(other, 1);
    var cs2 =
        SIMD.Int16x8.extractLane(t, 2) == SIMD.Int16x8.extractLane(other, 2);
    var cs3 =
        SIMD.Int16x8.extractLane(t, 3) == SIMD.Int16x8.extractLane(other, 3);
    var cs4 =
        SIMD.Int16x8.extractLane(t, 4) == SIMD.Int16x8.extractLane(other, 4);
    var cs5 =
        SIMD.Int16x8.extractLane(t, 5) == SIMD.Int16x8.extractLane(other, 5);
    var cs6 =
        SIMD.Int16x8.extractLane(t, 6) == SIMD.Int16x8.extractLane(other, 6);
    var cs7 =
        SIMD.Int16x8.extractLane(t, 7) == SIMD.Int16x8.extractLane(other, 7);
    return SIMD.Bool16x8(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.Int16x8.notEqual === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @param {Int16x8} other An instance of Int16x8.
    * @return {Bool16x8} true or false in each lane depending on
    * the result of t != other.
    */
  SIMD.Int16x8.notEqual = function(t, other) {
    t = SIMD.Int16x8.check(t);
    other = SIMD.Int16x8.check(other);
    var cs0 =
        SIMD.Int16x8.extractLane(t, 0) != SIMD.Int16x8.extractLane(other, 0);
    var cs1 =
        SIMD.Int16x8.extractLane(t, 1) != SIMD.Int16x8.extractLane(other, 1);
    var cs2 =
        SIMD.Int16x8.extractLane(t, 2) != SIMD.Int16x8.extractLane(other, 2);
    var cs3 =
        SIMD.Int16x8.extractLane(t, 3) != SIMD.Int16x8.extractLane(other, 3);
    var cs4 =
        SIMD.Int16x8.extractLane(t, 4) != SIMD.Int16x8.extractLane(other, 4);
    var cs5 =
        SIMD.Int16x8.extractLane(t, 5) != SIMD.Int16x8.extractLane(other, 5);
    var cs6 =
        SIMD.Int16x8.extractLane(t, 6) != SIMD.Int16x8.extractLane(other, 6);
    var cs7 =
        SIMD.Int16x8.extractLane(t, 7) != SIMD.Int16x8.extractLane(other, 7);
    return SIMD.Bool16x8(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.Int16x8.greaterThan === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @param {Int16x8} other An instance of Int16x8.
    * @return {Bool16x8} true or false in each lane depending on
    * the result of t > other.
    */
  SIMD.Int16x8.greaterThan = function(t, other) {
    t = SIMD.Int16x8.check(t);
    other = SIMD.Int16x8.check(other);
    var cs0 =
        SIMD.Int16x8.extractLane(t, 0) > SIMD.Int16x8.extractLane(other, 0);
    var cs1 =
        SIMD.Int16x8.extractLane(t, 1) > SIMD.Int16x8.extractLane(other, 1);
    var cs2 =
        SIMD.Int16x8.extractLane(t, 2) > SIMD.Int16x8.extractLane(other, 2);
    var cs3 =
        SIMD.Int16x8.extractLane(t, 3) > SIMD.Int16x8.extractLane(other, 3);
    var cs4 =
        SIMD.Int16x8.extractLane(t, 4) > SIMD.Int16x8.extractLane(other, 4);
    var cs5 =
        SIMD.Int16x8.extractLane(t, 5) > SIMD.Int16x8.extractLane(other, 5);
    var cs6 =
        SIMD.Int16x8.extractLane(t, 6) > SIMD.Int16x8.extractLane(other, 6);
    var cs7 =
        SIMD.Int16x8.extractLane(t, 7) > SIMD.Int16x8.extractLane(other, 7);
    return SIMD.Bool16x8(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.Int16x8.unsignedGreaterThan === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @param {Int16x8} other An instance of Int16x8.
    * @return {Bool16x8} true or false in each lane depending on
    * the result of t > other as unsigned values.
    */
  SIMD.Int16x8.unsignedGreaterThan = function(t, other) {
    t = SIMD.Int16x8.check(t);
    other = SIMD.Int16x8.check(other);
    var cs0 =
        SIMD.Int16x8.unsignedExtractLane(t, 0) > SIMD.Int16x8.unsignedExtractLane(other, 0);
    var cs1 =
        SIMD.Int16x8.unsignedExtractLane(t, 1) > SIMD.Int16x8.unsignedExtractLane(other, 1);
    var cs2 =
        SIMD.Int16x8.unsignedExtractLane(t, 2) > SIMD.Int16x8.unsignedExtractLane(other, 2);
    var cs3 =
        SIMD.Int16x8.unsignedExtractLane(t, 3) > SIMD.Int16x8.unsignedExtractLane(other, 3);
    var cs4 =
        SIMD.Int16x8.unsignedExtractLane(t, 4) > SIMD.Int16x8.unsignedExtractLane(other, 4);
    var cs5 =
        SIMD.Int16x8.unsignedExtractLane(t, 5) > SIMD.Int16x8.unsignedExtractLane(other, 5);
    var cs6 =
        SIMD.Int16x8.unsignedExtractLane(t, 6) > SIMD.Int16x8.unsignedExtractLane(other, 6);
    var cs7 =
        SIMD.Int16x8.unsignedExtractLane(t, 7) > SIMD.Int16x8.unsignedExtractLane(other, 7);
    return SIMD.Bool16x8(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.Int16x8.greaterThanOrEqual === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @param {Int16x8} other An instance of Int16x8.
    * @return {Bool16x8} true or false in each lane depending on
    * the result of t >= other.
    */
  SIMD.Int16x8.greaterThanOrEqual = function(t, other) {
    t = SIMD.Int16x8.check(t);
    other = SIMD.Int16x8.check(other);
    var cs0 =
        SIMD.Int16x8.extractLane(t, 0) >= SIMD.Int16x8.extractLane(other, 0);
    var cs1 =
        SIMD.Int16x8.extractLane(t, 1) >= SIMD.Int16x8.extractLane(other, 1);
    var cs2 =
        SIMD.Int16x8.extractLane(t, 2) >= SIMD.Int16x8.extractLane(other, 2);
    var cs3 =
        SIMD.Int16x8.extractLane(t, 3) >= SIMD.Int16x8.extractLane(other, 3);
    var cs4 =
        SIMD.Int16x8.extractLane(t, 4) >= SIMD.Int16x8.extractLane(other, 4);
    var cs5 =
        SIMD.Int16x8.extractLane(t, 5) >= SIMD.Int16x8.extractLane(other, 5);
    var cs6 =
        SIMD.Int16x8.extractLane(t, 6) >= SIMD.Int16x8.extractLane(other, 6);
    var cs7 =
        SIMD.Int16x8.extractLane(t, 7) >= SIMD.Int16x8.extractLane(other, 7);
    return SIMD.Bool16x8(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.Int16x8.unsignedGreaterThanOrEqual === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @param {Int16x8} other An instance of Int16x8.
    * @return {Bool16x8} true or false in each lane depending on
    * the result of t >= other as unsigned values.
    */
  SIMD.Int16x8.unsignedGreaterThanOrEqual = function(t, other) {
    t = SIMD.Int16x8.check(t);
    other = SIMD.Int16x8.check(other);
    var cs0 =
        SIMD.Int16x8.unsignedExtractLane(t, 0) >= SIMD.Int16x8.unsignedExtractLane(other, 0);
    var cs1 =
        SIMD.Int16x8.unsignedExtractLane(t, 1) >= SIMD.Int16x8.unsignedExtractLane(other, 1);
    var cs2 =
        SIMD.Int16x8.unsignedExtractLane(t, 2) >= SIMD.Int16x8.unsignedExtractLane(other, 2);
    var cs3 =
        SIMD.Int16x8.unsignedExtractLane(t, 3) >= SIMD.Int16x8.unsignedExtractLane(other, 3);
    var cs4 =
        SIMD.Int16x8.unsignedExtractLane(t, 4) >= SIMD.Int16x8.unsignedExtractLane(other, 4);
    var cs5 =
        SIMD.Int16x8.unsignedExtractLane(t, 5) >= SIMD.Int16x8.unsignedExtractLane(other, 5);
    var cs6 =
        SIMD.Int16x8.unsignedExtractLane(t, 6) >= SIMD.Int16x8.unsignedExtractLane(other, 6);
    var cs7 =
        SIMD.Int16x8.unsignedExtractLane(t, 7) >= SIMD.Int16x8.unsignedExtractLane(other, 7);
    return SIMD.Bool16x8(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.Int16x8.lessThan === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @param {Int16x8} other An instance of Int16x8.
    * @return {Bool16x8} true or false in each lane depending on
    * the result of t < other.
    */
  SIMD.Int16x8.lessThan = function(t, other) {
    t = SIMD.Int16x8.check(t);
    other = SIMD.Int16x8.check(other);
    var cs0 =
        SIMD.Int16x8.extractLane(t, 0) < SIMD.Int16x8.extractLane(other, 0);
    var cs1 =
        SIMD.Int16x8.extractLane(t, 1) < SIMD.Int16x8.extractLane(other, 1);
    var cs2 =
        SIMD.Int16x8.extractLane(t, 2) < SIMD.Int16x8.extractLane(other, 2);
    var cs3 =
        SIMD.Int16x8.extractLane(t, 3) < SIMD.Int16x8.extractLane(other, 3);
    var cs4 =
        SIMD.Int16x8.extractLane(t, 4) < SIMD.Int16x8.extractLane(other, 4);
    var cs5 =
        SIMD.Int16x8.extractLane(t, 5) < SIMD.Int16x8.extractLane(other, 5);
    var cs6 =
        SIMD.Int16x8.extractLane(t, 6) < SIMD.Int16x8.extractLane(other, 6);
    var cs7 =
        SIMD.Int16x8.extractLane(t, 7) < SIMD.Int16x8.extractLane(other, 7);
    return SIMD.Bool16x8(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.Int16x8.unsignedLessThan === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @param {Int16x8} other An instance of Int16x8.
    * @return {Bool16x8} true or false in each lane depending on
    * the result of t < other as unsigned values.
    */
  SIMD.Int16x8.unsignedLessThan = function(t, other) {
    t = SIMD.Int16x8.check(t);
    other = SIMD.Int16x8.check(other);
    var cs0 =
        SIMD.Int16x8.unsignedExtractLane(t, 0) < SIMD.Int16x8.unsignedExtractLane(other, 0);
    var cs1 =
        SIMD.Int16x8.unsignedExtractLane(t, 1) < SIMD.Int16x8.unsignedExtractLane(other, 1);
    var cs2 =
        SIMD.Int16x8.unsignedExtractLane(t, 2) < SIMD.Int16x8.unsignedExtractLane(other, 2);
    var cs3 =
        SIMD.Int16x8.unsignedExtractLane(t, 3) < SIMD.Int16x8.unsignedExtractLane(other, 3);
    var cs4 =
        SIMD.Int16x8.unsignedExtractLane(t, 4) < SIMD.Int16x8.unsignedExtractLane(other, 4);
    var cs5 =
        SIMD.Int16x8.unsignedExtractLane(t, 5) < SIMD.Int16x8.unsignedExtractLane(other, 5);
    var cs6 =
        SIMD.Int16x8.unsignedExtractLane(t, 6) < SIMD.Int16x8.unsignedExtractLane(other, 6);
    var cs7 =
        SIMD.Int16x8.unsignedExtractLane(t, 7) < SIMD.Int16x8.unsignedExtractLane(other, 7);
    return SIMD.Bool16x8(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.Int16x8.lessThanOrEqual === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @param {Int16x8} other An instance of Int16x8.
    * @return {Bool16x8} true or false in each lane depending on
    * the result of t <= other.
    */
  SIMD.Int16x8.lessThanOrEqual = function(t, other) {
    t = SIMD.Int16x8.check(t);
    other = SIMD.Int16x8.check(other);
    var cs0 =
        SIMD.Int16x8.extractLane(t, 0) <= SIMD.Int16x8.extractLane(other, 0);
    var cs1 =
        SIMD.Int16x8.extractLane(t, 1) <= SIMD.Int16x8.extractLane(other, 1);
    var cs2 =
        SIMD.Int16x8.extractLane(t, 2) <= SIMD.Int16x8.extractLane(other, 2);
    var cs3 =
        SIMD.Int16x8.extractLane(t, 3) <= SIMD.Int16x8.extractLane(other, 3);
    var cs4 =
        SIMD.Int16x8.extractLane(t, 4) <= SIMD.Int16x8.extractLane(other, 4);
    var cs5 =
        SIMD.Int16x8.extractLane(t, 5) <= SIMD.Int16x8.extractLane(other, 5);
    var cs6 =
        SIMD.Int16x8.extractLane(t, 6) <= SIMD.Int16x8.extractLane(other, 6);
    var cs7 =
        SIMD.Int16x8.extractLane(t, 7) <= SIMD.Int16x8.extractLane(other, 7);
    return SIMD.Bool16x8(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.Int16x8.unsignedLessThanOrEqual === "undefined") {
  /**
    * @param {Int16x8} t An instance of Int16x8.
    * @param {Int16x8} other An instance of Int16x8.
    * @return {Bool16x8} true or false in each lane depending on
    * the result of t <= other as unsigned values.
    */
  SIMD.Int16x8.unsignedLessThanOrEqual = function(t, other) {
    t = SIMD.Int16x8.check(t);
    other = SIMD.Int16x8.check(other);
    var cs0 =
        SIMD.Int16x8.unsignedExtractLane(t, 0) <= SIMD.Int16x8.unsignedExtractLane(other, 0);
    var cs1 =
        SIMD.Int16x8.unsignedExtractLane(t, 1) <= SIMD.Int16x8.unsignedExtractLane(other, 1);
    var cs2 =
        SIMD.Int16x8.unsignedExtractLane(t, 2) <= SIMD.Int16x8.unsignedExtractLane(other, 2);
    var cs3 =
        SIMD.Int16x8.unsignedExtractLane(t, 3) <= SIMD.Int16x8.unsignedExtractLane(other, 3);
    var cs4 =
        SIMD.Int16x8.unsignedExtractLane(t, 4) <= SIMD.Int16x8.unsignedExtractLane(other, 4);
    var cs5 =
        SIMD.Int16x8.unsignedExtractLane(t, 5) <= SIMD.Int16x8.unsignedExtractLane(other, 5);
    var cs6 =
        SIMD.Int16x8.unsignedExtractLane(t, 6) <= SIMD.Int16x8.unsignedExtractLane(other, 6);
    var cs7 =
        SIMD.Int16x8.unsignedExtractLane(t, 7) <= SIMD.Int16x8.unsignedExtractLane(other, 7);
    return SIMD.Bool16x8(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.Int16x8.shiftLeftByScalar === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @param {integer} bits Bit count to shift by.
    * @return {Int16x8} lanes in a shifted by bits.
    */
  SIMD.Int16x8.shiftLeftByScalar = function(a, bits) {
    a = SIMD.Int16x8.check(a);
    if (bits>>>0 > 16)
      bits = 16;
    var s0 = SIMD.Int16x8.extractLane(a, 0) << bits;
    var s1 = SIMD.Int16x8.extractLane(a, 1) << bits;
    var s2 = SIMD.Int16x8.extractLane(a, 2) << bits;
    var s3 = SIMD.Int16x8.extractLane(a, 3) << bits;
    var s4 = SIMD.Int16x8.extractLane(a, 4) << bits;
    var s5 = SIMD.Int16x8.extractLane(a, 5) << bits;
    var s6 = SIMD.Int16x8.extractLane(a, 6) << bits;
    var s7 = SIMD.Int16x8.extractLane(a, 7) << bits;
    return SIMD.Int16x8(s0, s1, s2, s3, s4, s5, s6, s7);
  }
}

if (typeof SIMD.Int16x8.shiftRightLogicalByScalar === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @param {integer} bits Bit count to shift by.
    * @return {Int16x8} lanes in a shifted by bits.
    */
  SIMD.Int16x8.shiftRightLogicalByScalar = function(a, bits) {
    a = SIMD.Int16x8.check(a);
    if (bits>>>0 > 16)
      bits = 16;
    var s0 = (SIMD.Int16x8.extractLane(a, 0) & 0xffff) >>> bits;
    var s1 = (SIMD.Int16x8.extractLane(a, 1) & 0xffff) >>> bits;
    var s2 = (SIMD.Int16x8.extractLane(a, 2) & 0xffff) >>> bits;
    var s3 = (SIMD.Int16x8.extractLane(a, 3) & 0xffff) >>> bits;
    var s4 = (SIMD.Int16x8.extractLane(a, 4) & 0xffff) >>> bits;
    var s5 = (SIMD.Int16x8.extractLane(a, 5) & 0xffff) >>> bits;
    var s6 = (SIMD.Int16x8.extractLane(a, 6) & 0xffff) >>> bits;
    var s7 = (SIMD.Int16x8.extractLane(a, 7) & 0xffff) >>> bits;
    return SIMD.Int16x8(s0, s1, s2, s3, s4, s5, s6, s7);
  }
}

if (typeof SIMD.Int16x8.shiftRightArithmeticByScalar === "undefined") {
  /**
    * @param {Int16x8} a An instance of Int16x8.
    * @param {integer} bits Bit count to shift by.
    * @return {Int16x8} lanes in a shifted by bits.
    */
  SIMD.Int16x8.shiftRightArithmeticByScalar = function(a, bits) {
    a = SIMD.Int16x8.check(a);
    if (bits>>>0 > 16)
      bits = 16;
    var s0 = SIMD.Int16x8.extractLane(a, 0) >> bits;
    var s1 = SIMD.Int16x8.extractLane(a, 1) >> bits;
    var s2 = SIMD.Int16x8.extractLane(a, 2) >> bits;
    var s3 = SIMD.Int16x8.extractLane(a, 3) >> bits;
    var s4 = SIMD.Int16x8.extractLane(a, 4) >> bits;
    var s5 = SIMD.Int16x8.extractLane(a, 5) >> bits;
    var s6 = SIMD.Int16x8.extractLane(a, 6) >> bits;
    var s7 = SIMD.Int16x8.extractLane(a, 7) >> bits;
    return SIMD.Int16x8(s0, s1, s2, s3, s4, s5, s6, s7);
  }
}

if (typeof SIMD.Int16x8.load === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {Int16x8} New instance of Int16x8.
    */
  SIMD.Int16x8.load = function(tarray, index) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var i16temp = _i16x8;
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? i16temp :
                bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : _i32x4) :
                _f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.Int16x8(i16temp[0], i16temp[1], i16temp[2], i16temp[3],
                        i16temp[4], i16temp[5], i16temp[6], i16temp[7]);
  }
}

if (typeof SIMD.Int16x8.store === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {Int16x8} value An instance of Int16x8.
    * @return {Int16x8} value
    */
  SIMD.Int16x8.store = function(tarray, index, value) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.Int16x8.check(value);
    _i16x8[0] = SIMD.Int16x8.extractLane(value, 0);
    _i16x8[1] = SIMD.Int16x8.extractLane(value, 1);
    _i16x8[2] = SIMD.Int16x8.extractLane(value, 2);
    _i16x8[3] = SIMD.Int16x8.extractLane(value, 3);
    _i16x8[4] = SIMD.Int16x8.extractLane(value, 4);
    _i16x8[5] = SIMD.Int16x8.extractLane(value, 5);
    _i16x8[6] = SIMD.Int16x8.extractLane(value, 6);
    _i16x8[7] = SIMD.Int16x8.extractLane(value, 7);
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : _i32x4) :
                _f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      tarray[index + i] = array[i];
    return value;
  }
}

if (typeof SIMD.Int8x16.and === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {Int8x16} b An instance of Int8x16.
    * @return {Int8x16} New instance of Int8x16 with values of a & b.
    */
  SIMD.Int8x16.and = function(a, b) {
    a = SIMD.Int8x16.check(a);
    b = SIMD.Int8x16.check(b);
    return SIMD.Int8x16(
        SIMD.Int8x16.extractLane(a, 0) & SIMD.Int8x16.extractLane(b, 0),
        SIMD.Int8x16.extractLane(a, 1) & SIMD.Int8x16.extractLane(b, 1),
        SIMD.Int8x16.extractLane(a, 2) & SIMD.Int8x16.extractLane(b, 2),
        SIMD.Int8x16.extractLane(a, 3) & SIMD.Int8x16.extractLane(b, 3),
        SIMD.Int8x16.extractLane(a, 4) & SIMD.Int8x16.extractLane(b, 4),
        SIMD.Int8x16.extractLane(a, 5) & SIMD.Int8x16.extractLane(b, 5),
        SIMD.Int8x16.extractLane(a, 6) & SIMD.Int8x16.extractLane(b, 6),
        SIMD.Int8x16.extractLane(a, 7) & SIMD.Int8x16.extractLane(b, 7),
        SIMD.Int8x16.extractLane(a, 8) & SIMD.Int8x16.extractLane(b, 8),
        SIMD.Int8x16.extractLane(a, 9) & SIMD.Int8x16.extractLane(b, 9),
        SIMD.Int8x16.extractLane(a, 10) & SIMD.Int8x16.extractLane(b, 10),
        SIMD.Int8x16.extractLane(a, 11) & SIMD.Int8x16.extractLane(b, 11),
        SIMD.Int8x16.extractLane(a, 12) & SIMD.Int8x16.extractLane(b, 12),
        SIMD.Int8x16.extractLane(a, 13) & SIMD.Int8x16.extractLane(b, 13),
        SIMD.Int8x16.extractLane(a, 14) & SIMD.Int8x16.extractLane(b, 14),
        SIMD.Int8x16.extractLane(a, 15) & SIMD.Int8x16.extractLane(b, 15));
  }
}

if (typeof SIMD.Int8x16.or === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {Int8x16} b An instance of Int8x16.
    * @return {Int8x16} New instance of Int8x16 with values of a | b.
    */
  SIMD.Int8x16.or = function(a, b) {
    a = SIMD.Int8x16.check(a);
    b = SIMD.Int8x16.check(b);
    return SIMD.Int8x16(
        SIMD.Int8x16.extractLane(a, 0) | SIMD.Int8x16.extractLane(b, 0),
        SIMD.Int8x16.extractLane(a, 1) | SIMD.Int8x16.extractLane(b, 1),
        SIMD.Int8x16.extractLane(a, 2) | SIMD.Int8x16.extractLane(b, 2),
        SIMD.Int8x16.extractLane(a, 3) | SIMD.Int8x16.extractLane(b, 3),
        SIMD.Int8x16.extractLane(a, 4) | SIMD.Int8x16.extractLane(b, 4),
        SIMD.Int8x16.extractLane(a, 5) | SIMD.Int8x16.extractLane(b, 5),
        SIMD.Int8x16.extractLane(a, 6) | SIMD.Int8x16.extractLane(b, 6),
        SIMD.Int8x16.extractLane(a, 7) | SIMD.Int8x16.extractLane(b, 7),
        SIMD.Int8x16.extractLane(a, 8) | SIMD.Int8x16.extractLane(b, 8),
        SIMD.Int8x16.extractLane(a, 9) | SIMD.Int8x16.extractLane(b, 9),
        SIMD.Int8x16.extractLane(a, 10) | SIMD.Int8x16.extractLane(b, 10),
        SIMD.Int8x16.extractLane(a, 11) | SIMD.Int8x16.extractLane(b, 11),
        SIMD.Int8x16.extractLane(a, 12) | SIMD.Int8x16.extractLane(b, 12),
        SIMD.Int8x16.extractLane(a, 13) | SIMD.Int8x16.extractLane(b, 13),
        SIMD.Int8x16.extractLane(a, 14) | SIMD.Int8x16.extractLane(b, 14),
        SIMD.Int8x16.extractLane(a, 15) | SIMD.Int8x16.extractLane(b, 15));
  }
}

if (typeof SIMD.Int8x16.xor === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {Int8x16} b An instance of Int8x16.
    * @return {Int8x16} New instance of Int8x16 with values of a ^ b.
    */
  SIMD.Int8x16.xor = function(a, b) {
    a = SIMD.Int8x16.check(a);
    b = SIMD.Int8x16.check(b);
    return SIMD.Int8x16(
        SIMD.Int8x16.extractLane(a, 0) ^ SIMD.Int8x16.extractLane(b, 0),
        SIMD.Int8x16.extractLane(a, 1) ^ SIMD.Int8x16.extractLane(b, 1),
        SIMD.Int8x16.extractLane(a, 2) ^ SIMD.Int8x16.extractLane(b, 2),
        SIMD.Int8x16.extractLane(a, 3) ^ SIMD.Int8x16.extractLane(b, 3),
        SIMD.Int8x16.extractLane(a, 4) ^ SIMD.Int8x16.extractLane(b, 4),
        SIMD.Int8x16.extractLane(a, 5) ^ SIMD.Int8x16.extractLane(b, 5),
        SIMD.Int8x16.extractLane(a, 6) ^ SIMD.Int8x16.extractLane(b, 6),
        SIMD.Int8x16.extractLane(a, 7) ^ SIMD.Int8x16.extractLane(b, 7),
        SIMD.Int8x16.extractLane(a, 8) ^ SIMD.Int8x16.extractLane(b, 8),
        SIMD.Int8x16.extractLane(a, 9) ^ SIMD.Int8x16.extractLane(b, 9),
        SIMD.Int8x16.extractLane(a, 10) ^ SIMD.Int8x16.extractLane(b, 10),
        SIMD.Int8x16.extractLane(a, 11) ^ SIMD.Int8x16.extractLane(b, 11),
        SIMD.Int8x16.extractLane(a, 12) ^ SIMD.Int8x16.extractLane(b, 12),
        SIMD.Int8x16.extractLane(a, 13) ^ SIMD.Int8x16.extractLane(b, 13),
        SIMD.Int8x16.extractLane(a, 14) ^ SIMD.Int8x16.extractLane(b, 14),
        SIMD.Int8x16.extractLane(a, 15) ^ SIMD.Int8x16.extractLane(b, 15));
  }
}

if (typeof SIMD.Int8x16.not === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @return {Int8x16} New instance of Int8x16 with values of ~t
    */
  SIMD.Int8x16.not = function(t) {
    t = SIMD.Int8x16.check(t);
    return SIMD.Int8x16(~SIMD.Int8x16.extractLane(t, 0),
                        ~SIMD.Int8x16.extractLane(t, 1),
                        ~SIMD.Int8x16.extractLane(t, 2),
                        ~SIMD.Int8x16.extractLane(t, 3),
                        ~SIMD.Int8x16.extractLane(t, 4),
                        ~SIMD.Int8x16.extractLane(t, 5),
                        ~SIMD.Int8x16.extractLane(t, 6),
                        ~SIMD.Int8x16.extractLane(t, 7),
                        ~SIMD.Int8x16.extractLane(t, 8),
                        ~SIMD.Int8x16.extractLane(t, 9),
                        ~SIMD.Int8x16.extractLane(t, 10),
                        ~SIMD.Int8x16.extractLane(t, 11),
                        ~SIMD.Int8x16.extractLane(t, 12),
                        ~SIMD.Int8x16.extractLane(t, 13),
                        ~SIMD.Int8x16.extractLane(t, 14),
                        ~SIMD.Int8x16.extractLane(t, 15));
  }
}

if (typeof SIMD.Int8x16.neg === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @return {Int8x16} New instance of Int8x16 with values of -t
    */
  SIMD.Int8x16.neg = function(t) {
    t = SIMD.Int8x16.check(t);
    return SIMD.Int8x16(-SIMD.Int8x16.extractLane(t, 0),
                        -SIMD.Int8x16.extractLane(t, 1),
                        -SIMD.Int8x16.extractLane(t, 2),
                        -SIMD.Int8x16.extractLane(t, 3),
                        -SIMD.Int8x16.extractLane(t, 4),
                        -SIMD.Int8x16.extractLane(t, 5),
                        -SIMD.Int8x16.extractLane(t, 6),
                        -SIMD.Int8x16.extractLane(t, 7),
                        -SIMD.Int8x16.extractLane(t, 8),
                        -SIMD.Int8x16.extractLane(t, 9),
                        -SIMD.Int8x16.extractLane(t, 10),
                        -SIMD.Int8x16.extractLane(t, 11),
                        -SIMD.Int8x16.extractLane(t, 12),
                        -SIMD.Int8x16.extractLane(t, 13),
                        -SIMD.Int8x16.extractLane(t, 14),
                        -SIMD.Int8x16.extractLane(t, 15));
  }
}

if (typeof SIMD.Int8x16.add === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {Int8x16} b An instance of Int8x16.
    * @return {Int8x16} New instance of Int8x16 with values of a + b.
    */
  SIMD.Int8x16.add = function(a, b) {
    a = SIMD.Int8x16.check(a);
    b = SIMD.Int8x16.check(b);
    return SIMD.Int8x16(
        SIMD.Int8x16.extractLane(a, 0) + SIMD.Int8x16.extractLane(b, 0),
        SIMD.Int8x16.extractLane(a, 1) + SIMD.Int8x16.extractLane(b, 1),
        SIMD.Int8x16.extractLane(a, 2) + SIMD.Int8x16.extractLane(b, 2),
        SIMD.Int8x16.extractLane(a, 3) + SIMD.Int8x16.extractLane(b, 3),
        SIMD.Int8x16.extractLane(a, 4) + SIMD.Int8x16.extractLane(b, 4),
        SIMD.Int8x16.extractLane(a, 5) + SIMD.Int8x16.extractLane(b, 5),
        SIMD.Int8x16.extractLane(a, 6) + SIMD.Int8x16.extractLane(b, 6),
        SIMD.Int8x16.extractLane(a, 7) + SIMD.Int8x16.extractLane(b, 7),
        SIMD.Int8x16.extractLane(a, 8) + SIMD.Int8x16.extractLane(b, 8),
        SIMD.Int8x16.extractLane(a, 9) + SIMD.Int8x16.extractLane(b, 9),
        SIMD.Int8x16.extractLane(a, 10) + SIMD.Int8x16.extractLane(b, 10),
        SIMD.Int8x16.extractLane(a, 11) + SIMD.Int8x16.extractLane(b, 11),
        SIMD.Int8x16.extractLane(a, 12) + SIMD.Int8x16.extractLane(b, 12),
        SIMD.Int8x16.extractLane(a, 13) + SIMD.Int8x16.extractLane(b, 13),
        SIMD.Int8x16.extractLane(a, 14) + SIMD.Int8x16.extractLane(b, 14),
        SIMD.Int8x16.extractLane(a, 15) + SIMD.Int8x16.extractLane(b, 15));
  }
}

if (typeof SIMD.Int8x16.sub === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {Int8x16} b An instance of Int8x16.
    * @return {Int8x16} New instance of Int8x16 with values of a - b.
    */
  SIMD.Int8x16.sub = function(a, b) {
    a = SIMD.Int8x16.check(a);
    b = SIMD.Int8x16.check(b);
    return SIMD.Int8x16(
        SIMD.Int8x16.extractLane(a, 0) - SIMD.Int8x16.extractLane(b, 0),
        SIMD.Int8x16.extractLane(a, 1) - SIMD.Int8x16.extractLane(b, 1),
        SIMD.Int8x16.extractLane(a, 2) - SIMD.Int8x16.extractLane(b, 2),
        SIMD.Int8x16.extractLane(a, 3) - SIMD.Int8x16.extractLane(b, 3),
        SIMD.Int8x16.extractLane(a, 4) - SIMD.Int8x16.extractLane(b, 4),
        SIMD.Int8x16.extractLane(a, 5) - SIMD.Int8x16.extractLane(b, 5),
        SIMD.Int8x16.extractLane(a, 6) - SIMD.Int8x16.extractLane(b, 6),
        SIMD.Int8x16.extractLane(a, 7) - SIMD.Int8x16.extractLane(b, 7),
        SIMD.Int8x16.extractLane(a, 8) - SIMD.Int8x16.extractLane(b, 8),
        SIMD.Int8x16.extractLane(a, 9) - SIMD.Int8x16.extractLane(b, 9),
        SIMD.Int8x16.extractLane(a, 10) - SIMD.Int8x16.extractLane(b, 10),
        SIMD.Int8x16.extractLane(a, 11) - SIMD.Int8x16.extractLane(b, 11),
        SIMD.Int8x16.extractLane(a, 12) - SIMD.Int8x16.extractLane(b, 12),
        SIMD.Int8x16.extractLane(a, 13) - SIMD.Int8x16.extractLane(b, 13),
        SIMD.Int8x16.extractLane(a, 14) - SIMD.Int8x16.extractLane(b, 14),
        SIMD.Int8x16.extractLane(a, 15) - SIMD.Int8x16.extractLane(b, 15));
  }
}

if (typeof SIMD.Int8x16.mul === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {Int8x16} b An instance of Int8x16.
    * @return {Int8x16} New instance of Int8x16 with values of a * b.
    */
  SIMD.Int8x16.mul = function(a, b) {
    a = SIMD.Int8x16.check(a);
    b = SIMD.Int8x16.check(b);
    return SIMD.Int8x16(Math.imul(SIMD.Int8x16.extractLane(a, 0),
                                  SIMD.Int8x16.extractLane(b, 0)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 1),
                                  SIMD.Int8x16.extractLane(b, 1)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 2),
                                  SIMD.Int8x16.extractLane(b, 2)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 3),
                                  SIMD.Int8x16.extractLane(b, 3)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 4),
                                  SIMD.Int8x16.extractLane(b, 4)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 5),
                                  SIMD.Int8x16.extractLane(b, 5)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 6),
                                  SIMD.Int8x16.extractLane(b, 6)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 7),
                                  SIMD.Int8x16.extractLane(b, 7)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 8),
                                  SIMD.Int8x16.extractLane(b, 8)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 9),
                                  SIMD.Int8x16.extractLane(b, 9)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 10),
                                  SIMD.Int8x16.extractLane(b, 10)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 11),
                                  SIMD.Int8x16.extractLane(b, 11)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 12),
                                  SIMD.Int8x16.extractLane(b, 12)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 13),
                                  SIMD.Int8x16.extractLane(b, 13)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 14),
                                  SIMD.Int8x16.extractLane(b, 14)),
                        Math.imul(SIMD.Int8x16.extractLane(a, 15),
                                  SIMD.Int8x16.extractLane(b, 15)));
  }
}

if (typeof SIMD.Int8x16.swizzle === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16 to be swizzled.
    * @param {integer} s0 - Index in t for lane s0
    * @param {integer} s1 - Index in t for lane s1
    * @param {integer} s2 - Index in t for lane s2
    * @param {integer} s3 - Index in t for lane s3
    * @param {integer} s4 - Index in t for lane s4
    * @param {integer} s5 - Index in t for lane s5
    * @param {integer} s6 - Index in t for lane s6
    * @param {integer} s7 - Index in t for lane s7
    * @param {integer} s8 - Index in t for lane s8
    * @param {integer} s9 - Index in t for lane s9
    * @param {integer} s10 - Index in t for lane s10
    * @param {integer} s11 - Index in t for lane s11
    * @param {integer} s12 - Index in t for lane s12
    * @param {integer} s13 - Index in t for lane s13
    * @param {integer} s14 - Index in t for lane s14
    * @param {integer} s15 - Index in t for lane s15
    * @return {Int8x16} New instance of Int8x16 with lanes swizzled.
    */
  SIMD.Int8x16.swizzle = function(t, s0, s1, s2, s3, s4, s5, s6, s7,
                                     s8, s9, s10, s11, s12, s13, s14, s15) {
    t = SIMD.Int8x16.check(t);
    check16(s0);
    check16(s1);
    check16(s2);
    check16(s3);
    check16(s4);
    check16(s5);
    check16(s6);
    check16(s7);
    check16(s8);
    check16(s9);
    check16(s10);
    check16(s11);
    check16(s12);
    check16(s13);
    check16(s14);
    check16(s15);
    var storage = _i8x16;
    storage[0] = SIMD.Int8x16.extractLane(t, 0);
    storage[1] = SIMD.Int8x16.extractLane(t, 1);
    storage[2] = SIMD.Int8x16.extractLane(t, 2);
    storage[3] = SIMD.Int8x16.extractLane(t, 3);
    storage[4] = SIMD.Int8x16.extractLane(t, 4);
    storage[5] = SIMD.Int8x16.extractLane(t, 5);
    storage[6] = SIMD.Int8x16.extractLane(t, 6);
    storage[7] = SIMD.Int8x16.extractLane(t, 7);
    storage[8] = SIMD.Int8x16.extractLane(t, 8);
    storage[9] = SIMD.Int8x16.extractLane(t, 9);
    storage[10] = SIMD.Int8x16.extractLane(t, 10);
    storage[11] = SIMD.Int8x16.extractLane(t, 11);
    storage[12] = SIMD.Int8x16.extractLane(t, 12);
    storage[13] = SIMD.Int8x16.extractLane(t, 13);
    storage[14] = SIMD.Int8x16.extractLane(t, 14);
    storage[15] = SIMD.Int8x16.extractLane(t, 15);
    return SIMD.Int8x16(storage[s0], storage[s1], storage[s2], storage[s3],
                        storage[s4], storage[s5], storage[s6], storage[s7],
                        storage[s8], storage[s9], storage[s10], storage[s11],
                        storage[s12], storage[s13], storage[s14], storage[s15]);
  }
}

if (typeof SIMD.Int8x16.shuffle === "undefined") {

  _i8x32 = new Int8Array(32);

  /**
    * @param {Int8x16} t0 An instance of Int8x16 to be shuffled.
    * @param {Int8x16} t1 An instance of Int8x16 to be shuffled.
    * @param {integer} s0 - Index in concatenation of t0 and t1 for lane s0
    * @param {integer} s1 - Index in concatenation of t0 and t1 for lane s1
    * @param {integer} s2 - Index in concatenation of t0 and t1 for lane s2
    * @param {integer} s3 - Index in concatenation of t0 and t1 for lane s3
    * @param {integer} s4 - Index in concatenation of t0 and t1 for lane s4
    * @param {integer} s5 - Index in concatenation of t0 and t1 for lane s5
    * @param {integer} s6 - Index in concatenation of t0 and t1 for lane s6
    * @param {integer} s7 - Index in concatenation of t0 and t1 for lane s7
    * @param {integer} s8 - Index in concatenation of t0 and t1 for lane s8
    * @param {integer} s9 - Index in concatenation of t0 and t1 for lane s9
    * @param {integer} s10 - Index in concatenation of t0 and t1 for lane s10
    * @param {integer} s11 - Index in concatenation of t0 and t1 for lane s11
    * @param {integer} s12 - Index in concatenation of t0 and t1 for lane s12
    * @param {integer} s13 - Index in concatenation of t0 and t1 for lane s13
    * @param {integer} s14 - Index in concatenation of t0 and t1 for lane s14
    * @param {integer} s15 - Index in concatenation of t0 and t1 for lane s15
    * @return {Int8x16} New instance of Int8x16 with lanes shuffled.
    */
  SIMD.Int8x16.shuffle = function(t0, t1, s0, s1, s2, s3, s4, s5, s6, s7,
                                          s8, s9, s10, s11, s12, s13, s14, s15) {
    t0 = SIMD.Int8x16.check(t0);
    t1 = SIMD.Int8x16.check(t1);
    check32(s0);
    check32(s1);
    check32(s2);
    check32(s3);
    check32(s4);
    check32(s5);
    check32(s6);
    check32(s7);
    check32(s8);
    check32(s9);
    check32(s10);
    check32(s11);
    check32(s12);
    check32(s13);
    check32(s14);
    check32(s15);
    var storage = _i8x32;
    storage[0] = SIMD.Int8x16.extractLane(t0, 0);
    storage[1] = SIMD.Int8x16.extractLane(t0, 1);
    storage[2] = SIMD.Int8x16.extractLane(t0, 2);
    storage[3] = SIMD.Int8x16.extractLane(t0, 3);
    storage[4] = SIMD.Int8x16.extractLane(t0, 4);
    storage[5] = SIMD.Int8x16.extractLane(t0, 5);
    storage[6] = SIMD.Int8x16.extractLane(t0, 6);
    storage[7] = SIMD.Int8x16.extractLane(t0, 7);
    storage[8] = SIMD.Int8x16.extractLane(t0, 8);
    storage[9] = SIMD.Int8x16.extractLane(t0, 9);
    storage[10] = SIMD.Int8x16.extractLane(t0, 10);
    storage[11] = SIMD.Int8x16.extractLane(t0, 11);
    storage[12] = SIMD.Int8x16.extractLane(t0, 12);
    storage[13] = SIMD.Int8x16.extractLane(t0, 13);
    storage[14] = SIMD.Int8x16.extractLane(t0, 14);
    storage[15] = SIMD.Int8x16.extractLane(t0, 15);
    storage[16] = SIMD.Int8x16.extractLane(t1, 0);
    storage[17] = SIMD.Int8x16.extractLane(t1, 1);
    storage[18] = SIMD.Int8x16.extractLane(t1, 2);
    storage[19] = SIMD.Int8x16.extractLane(t1, 3);
    storage[20] = SIMD.Int8x16.extractLane(t1, 4);
    storage[21] = SIMD.Int8x16.extractLane(t1, 5);
    storage[22] = SIMD.Int8x16.extractLane(t1, 6);
    storage[23] = SIMD.Int8x16.extractLane(t1, 7);
    storage[24] = SIMD.Int8x16.extractLane(t1, 8);
    storage[25] = SIMD.Int8x16.extractLane(t1, 9);
    storage[26] = SIMD.Int8x16.extractLane(t1, 10);
    storage[27] = SIMD.Int8x16.extractLane(t1, 11);
    storage[28] = SIMD.Int8x16.extractLane(t1, 12);
    storage[29] = SIMD.Int8x16.extractLane(t1, 13);
    storage[30] = SIMD.Int8x16.extractLane(t1, 14);
    storage[31] = SIMD.Int8x16.extractLane(t1, 15);
    return SIMD.Int8x16(storage[s0], storage[s1], storage[s2], storage[s3],
                        storage[s4], storage[s5], storage[s6], storage[s7],
                        storage[s8], storage[s9], storage[s10], storage[s11],
                        storage[s12], storage[s13], storage[s14], storage[s15]);
  }
}

if (typeof SIMD.Int8x16.addSaturate === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {Int8x16} b An instance of Int8x16.
    * @return {Int8x16} New instance of Int8x16 with values of a + b with
    * signed saturating behavior on overflow.
    */
  SIMD.Int8x16.addSaturate = function(a, b) {
    a = SIMD.Int8x16.check(a);
    b = SIMD.Int8x16.check(b);
    var c = SIMD.Int8x16.add(a, b);
    var max = SIMD.Int8x16.splat(0x7f);
    var min = SIMD.Int8x16.splat(0x80);
    var mask = SIMD.Int8x16.lessThan(c, a);
    var bneg = SIMD.Int8x16.lessThan(b, SIMD.Int8x16.splat(0));
    return SIMD.Int8x16.select(SIMD.Bool8x16.and(mask, SIMD.Bool8x16.not(bneg)), max,
             SIMD.Int8x16.select(SIMD.Bool8x16.and(SIMD.Bool8x16.not(mask), bneg), min,
               c));
  }
}

if (typeof SIMD.Int8x16.unsignedAddSaturate === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {Int8x16} b An instance of Int8x16.
    * @return {Int8x16} New instance of Int8x16 with values of a + b with
    * unsigned saturating behavior on overflow.
    */
  SIMD.Int8x16.unsignedAddSaturate = function(a, b) {
    a = SIMD.Int8x16.check(a);
    b = SIMD.Int8x16.check(b);
    var c = SIMD.Int8x16.add(a, b);
    var max = SIMD.Int8x16.splat(0xff);
    var min = SIMD.Int8x16.splat(0x00);
    var mask = SIMD.Int8x16.unsignedLessThan(c, a);
    var bneg = SIMD.Int8x16.unsignedLessThan(b, SIMD.Int8x16.splat(0));
    return SIMD.Int8x16.select(SIMD.Bool8x16.and(mask, SIMD.Bool8x16.not(bneg)), max,
             SIMD.Int8x16.select(SIMD.Bool8x16.and(SIMD.Bool8x16.not(mask), bneg), min,
               c));
  }
}

if (typeof SIMD.Int8x16.subSaturate === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {Int8x16} b An instance of Int8x16.
    * @return {Int8x16} New instance of Int8x16 with values of a - b with
    * signed saturating behavior on overflow.
    */
  SIMD.Int8x16.subSaturate = function(a, b) {
    a = SIMD.Int8x16.check(a);
    b = SIMD.Int8x16.check(b);
    var c = SIMD.Int8x16.sub(a, b);
    var max = SIMD.Int8x16.splat(0x7f);
    var min = SIMD.Int8x16.splat(0x80);
    var mask = SIMD.Int8x16.greaterThan(c, a);
    var bneg = SIMD.Int8x16.lessThan(b, SIMD.Int8x16.splat(0));
    return SIMD.Int8x16.select(SIMD.Bool8x16.and(mask, SIMD.Bool8x16.not(bneg)), min,
             SIMD.Int8x16.select(SIMD.Bool8x16.and(SIMD.Bool8x16.not(mask), bneg), max,
               c));
  }
}

if (typeof SIMD.Int8x16.unsignedSubSaturate === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {Int8x16} b An instance of Int8x16.
    * @return {Int8x16} New instance of Int8x16 with values of a - b with
    * unsigned saturating behavior on overflow.
    */
  SIMD.Int8x16.unsignedSubSaturate = function(a, b) {
    a = SIMD.Int8x16.check(a);
    b = SIMD.Int8x16.check(b);
    var c = SIMD.Int8x16.sub(a, b);
    var max = SIMD.Int8x16.splat(0xff);
    var min = SIMD.Int8x16.splat(0x00);
    var mask = SIMD.Int8x16.unsignedGreaterThan(c, a);
    var bneg = SIMD.Int8x16.unsignedLessThan(b, SIMD.Int8x16.splat(0));
    return SIMD.Int8x16.select(SIMD.Bool8x16.and(mask, SIMD.Bool8x16.not(bneg)), min,
             SIMD.Int8x16.select(SIMD.Bool8x16.and(SIMD.Bool8x16.not(mask), bneg), max,
               c));
  }
}

if (typeof SIMD.Int8x16.unsignedAbsoluteDifference === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {Int8x16} b An instance of Int8x16.
    * @return {Int8x16} The absolute differences (abs(x - y)) of the
    * corresponding elements of a and b. x and y are interpreted as unsigned
    * integers.
    */
  SIMD.Int8x16.unsignedAbsoluteDifference = function(a, b) {
    a = SIMD.Int8x16.check(a);
    b = SIMD.Int8x16.check(b);
    var x = SIMD.Int8x16(
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 0) - SIMD.Int8x16.unsignedExtractLane(b, 0)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 1) - SIMD.Int8x16.unsignedExtractLane(b, 1)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 2) - SIMD.Int8x16.unsignedExtractLane(b, 2)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 3) - SIMD.Int8x16.unsignedExtractLane(b, 3)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 4) - SIMD.Int8x16.unsignedExtractLane(b, 4)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 5) - SIMD.Int8x16.unsignedExtractLane(b, 5)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 6) - SIMD.Int8x16.unsignedExtractLane(b, 6)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 7) - SIMD.Int8x16.unsignedExtractLane(b, 7)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 8) - SIMD.Int8x16.unsignedExtractLane(b, 8)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 9) - SIMD.Int8x16.unsignedExtractLane(b, 9)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 10) - SIMD.Int8x16.unsignedExtractLane(b, 10)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 11) - SIMD.Int8x16.unsignedExtractLane(b, 11)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 12) - SIMD.Int8x16.unsignedExtractLane(b, 12)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 13) - SIMD.Int8x16.unsignedExtractLane(b, 13)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 14) - SIMD.Int8x16.unsignedExtractLane(b, 14)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 15) - SIMD.Int8x16.unsignedExtractLane(b, 15)));
    return x;
  }
}

if (typeof SIMD.Int8x16.widenedUnsignedAbsoluteDifference === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {Int8x16} b An instance of Int8x16.
    * @return {Int16x8} The absolute differences (abs(x - y)) of the
    * first 8 corresponding elements of a and b, returning 16-bit results.
    * x and y are interpreted as unsigned integers.
    */
  SIMD.Int8x16.widenedUnsignedAbsoluteDifference = function(a, b) {
    a = SIMD.Int8x16.check(a);
    b = SIMD.Int8x16.check(b);
    return SIMD.Int16x8(
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 0) - SIMD.Int8x16.unsignedExtractLane(b, 0)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 1) - SIMD.Int8x16.unsignedExtractLane(b, 1)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 2) - SIMD.Int8x16.unsignedExtractLane(b, 2)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 3) - SIMD.Int8x16.unsignedExtractLane(b, 3)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 4) - SIMD.Int8x16.unsignedExtractLane(b, 4)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 5) - SIMD.Int8x16.unsignedExtractLane(b, 5)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 6) - SIMD.Int8x16.unsignedExtractLane(b, 6)),
        Math.abs(
            SIMD.Int8x16.unsignedExtractLane(a, 7) - SIMD.Int8x16.unsignedExtractLane(b, 7)));
  }
}

if (typeof SIMD.Int8x16.unsignedHorizontalSum === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @return {Number} The sum of all the lanes in a, extracted as unsigned values.
    */
  SIMD.Int8x16.unsignedHorizontalSum = function(a) {
    a = SIMD.Int8x16.check(a);
    return SIMD.Int8x16.unsignedExtractLane(a, 0) +
           SIMD.Int8x16.unsignedExtractLane(a, 1) +
           SIMD.Int8x16.unsignedExtractLane(a, 2) +
           SIMD.Int8x16.unsignedExtractLane(a, 3) +
           SIMD.Int8x16.unsignedExtractLane(a, 4) +
           SIMD.Int8x16.unsignedExtractLane(a, 5) +
           SIMD.Int8x16.unsignedExtractLane(a, 6) +
           SIMD.Int8x16.unsignedExtractLane(a, 7) +
           SIMD.Int8x16.unsignedExtractLane(a, 8) +
           SIMD.Int8x16.unsignedExtractLane(a, 9) +
           SIMD.Int8x16.unsignedExtractLane(a, 10) +
           SIMD.Int8x16.unsignedExtractLane(a, 11) +
           SIMD.Int8x16.unsignedExtractLane(a, 12) +
           SIMD.Int8x16.unsignedExtractLane(a, 13) +
           SIMD.Int8x16.unsignedExtractLane(a, 14) +
           SIMD.Int8x16.unsignedExtractLane(a, 15);
  }
}

if (typeof SIMD.Int8x16.select === "undefined") {
  /**
    * @param {Bool8x16} t Selector mask. An instance of Bool8x16
    * @param {Int8x16} trueValue Pick lane from here if corresponding
    * selector lane is true
    * @param {Int8x16} falseValue Pick lane from here if corresponding
    * selector lane is false
    * @return {Int8x16} Mix of lanes from trueValue or falseValue as
    * indicated
    */
  SIMD.Int8x16.select = function(t, trueValue, falseValue) {
    t = SIMD.Bool8x16.check(t);
    trueValue = SIMD.Int8x16.check(trueValue);
    falseValue = SIMD.Int8x16.check(falseValue);
    return SIMD.Int8x16(
        SIMD.Bool8x16.extractLane(t, 0) ?
            SIMD.Int8x16.extractLane(trueValue, 0) :
                SIMD.Int8x16.extractLane(falseValue, 0),
        SIMD.Bool8x16.extractLane(t, 1) ?
            SIMD.Int8x16.extractLane(trueValue, 1) :
                SIMD.Int8x16.extractLane(falseValue, 1),
        SIMD.Bool8x16.extractLane(t, 2) ?
            SIMD.Int8x16.extractLane(trueValue, 2) :
                SIMD.Int8x16.extractLane(falseValue, 2),
        SIMD.Bool8x16.extractLane(t, 3) ?
            SIMD.Int8x16.extractLane(trueValue, 3) :
                SIMD.Int8x16.extractLane(falseValue, 3),
        SIMD.Bool8x16.extractLane(t, 4) ?
            SIMD.Int8x16.extractLane(trueValue, 4) :
                SIMD.Int8x16.extractLane(falseValue, 4),
        SIMD.Bool8x16.extractLane(t, 5) ?
            SIMD.Int8x16.extractLane(trueValue, 5) :
                SIMD.Int8x16.extractLane(falseValue, 5),
        SIMD.Bool8x16.extractLane(t, 6) ?
            SIMD.Int8x16.extractLane(trueValue, 6) :
                SIMD.Int8x16.extractLane(falseValue, 6),
        SIMD.Bool8x16.extractLane(t, 7) ?
            SIMD.Int8x16.extractLane(trueValue, 7) :
                SIMD.Int8x16.extractLane(falseValue, 7),
        SIMD.Bool8x16.extractLane(t, 8) ?
            SIMD.Int8x16.extractLane(trueValue, 8) :
                SIMD.Int8x16.extractLane(falseValue, 8),
        SIMD.Bool8x16.extractLane(t, 9) ?
            SIMD.Int8x16.extractLane(trueValue, 9) :
                SIMD.Int8x16.extractLane(falseValue, 9),
        SIMD.Bool8x16.extractLane(t, 10) ?
            SIMD.Int8x16.extractLane(trueValue, 10) :
                SIMD.Int8x16.extractLane(falseValue, 10),
        SIMD.Bool8x16.extractLane(t, 11) ?
            SIMD.Int8x16.extractLane(trueValue, 11) :
                SIMD.Int8x16.extractLane(falseValue, 11),
        SIMD.Bool8x16.extractLane(t, 12) ?
            SIMD.Int8x16.extractLane(trueValue, 12) :
                SIMD.Int8x16.extractLane(falseValue, 12),
        SIMD.Bool8x16.extractLane(t, 13) ?
            SIMD.Int8x16.extractLane(trueValue, 13) :
                SIMD.Int8x16.extractLane(falseValue, 13),
        SIMD.Bool8x16.extractLane(t, 14) ?
            SIMD.Int8x16.extractLane(trueValue, 14) :
                SIMD.Int8x16.extractLane(falseValue, 14),
        SIMD.Bool8x16.extractLane(t, 15) ?
            SIMD.Int8x16.extractLane(trueValue, 15) :
                SIMD.Int8x16.extractLane(falseValue, 15));
  }
}

if (typeof SIMD.Int8x16.selectBits === "undefined") {
  /**
    * @param {Int8x16} t Selector mask. An instance of Int8x16
    * @param {Int8x16} trueValue Pick bit from here if corresponding
    * selector bit is 1
    * @param {Int8x16} falseValue Pick bit from here if corresponding
    * selector bit is 0
    * @return {Int8x16} Mix of bits from trueValue or falseValue as
    * indicated
    */
  SIMD.Int8x16.selectBits = function(t, trueValue, falseValue) {
    t = SIMD.Int8x16.check(t);
    trueValue = SIMD.Int8x16.check(trueValue);
    falseValue = SIMD.Int8x16.check(falseValue);
    var tr = SIMD.Int8x16.and(t, trueValue);
    var fr = SIMD.Int8x16.and(SIMD.Int8x16.not(t), falseValue);
    return SIMD.Int8x16.or(tr, fr);
  }
}

if (typeof SIMD.Int8x16.equal === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @param {Int8x16} other An instance of Int8x16.
    * @return {Bool8x16} true or false in each lane depending on
    * the result of t == other.
    */
  SIMD.Int8x16.equal = function(t, other) {
    t = SIMD.Int8x16.check(t);
    other = SIMD.Int8x16.check(other);
    var cs0 =
        SIMD.Int8x16.extractLane(t, 0) == SIMD.Int8x16.extractLane(other, 0);
    var cs1 =
        SIMD.Int8x16.extractLane(t, 1) == SIMD.Int8x16.extractLane(other, 1);
    var cs2 =
        SIMD.Int8x16.extractLane(t, 2) == SIMD.Int8x16.extractLane(other, 2);
    var cs3 =
        SIMD.Int8x16.extractLane(t, 3) == SIMD.Int8x16.extractLane(other, 3);
    var cs4 =
        SIMD.Int8x16.extractLane(t, 4) == SIMD.Int8x16.extractLane(other, 4);
    var cs5 =
        SIMD.Int8x16.extractLane(t, 5) == SIMD.Int8x16.extractLane(other, 5);
    var cs6 =
        SIMD.Int8x16.extractLane(t, 6) == SIMD.Int8x16.extractLane(other, 6);
    var cs7 =
        SIMD.Int8x16.extractLane(t, 7) == SIMD.Int8x16.extractLane(other, 7);
    var cs8 =
        SIMD.Int8x16.extractLane(t, 8) == SIMD.Int8x16.extractLane(other, 8);
    var cs9 =
        SIMD.Int8x16.extractLane(t, 9) == SIMD.Int8x16.extractLane(other, 9);
    var cs10 =
        SIMD.Int8x16.extractLane(t, 10) == SIMD.Int8x16.extractLane(other, 10);
    var cs11 =
        SIMD.Int8x16.extractLane(t, 11) == SIMD.Int8x16.extractLane(other, 11);
    var cs12 =
        SIMD.Int8x16.extractLane(t, 12) == SIMD.Int8x16.extractLane(other, 12);
    var cs13 =
        SIMD.Int8x16.extractLane(t, 13) == SIMD.Int8x16.extractLane(other, 13);
    var cs14 =
        SIMD.Int8x16.extractLane(t, 14) == SIMD.Int8x16.extractLane(other, 14);
    var cs15 =
        SIMD.Int8x16.extractLane(t, 15) == SIMD.Int8x16.extractLane(other, 15);
    return SIMD.Bool8x16(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                         cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.Int8x16.notEqual === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @param {Int8x16} other An instance of Int8x16.
    * @return {Bool8x16} true or false in each lane depending on
    * the result of t != other.
    */
  SIMD.Int8x16.notEqual = function(t, other) {
    t = SIMD.Int8x16.check(t);
    other = SIMD.Int8x16.check(other);
    var cs0 =
        SIMD.Int8x16.extractLane(t, 0) != SIMD.Int8x16.extractLane(other, 0);
    var cs1 =
        SIMD.Int8x16.extractLane(t, 1) != SIMD.Int8x16.extractLane(other, 1);
    var cs2 =
        SIMD.Int8x16.extractLane(t, 2) != SIMD.Int8x16.extractLane(other, 2);
    var cs3 =
        SIMD.Int8x16.extractLane(t, 3) != SIMD.Int8x16.extractLane(other, 3);
    var cs4 =
        SIMD.Int8x16.extractLane(t, 4) != SIMD.Int8x16.extractLane(other, 4);
    var cs5 =
        SIMD.Int8x16.extractLane(t, 5) != SIMD.Int8x16.extractLane(other, 5);
    var cs6 =
        SIMD.Int8x16.extractLane(t, 6) != SIMD.Int8x16.extractLane(other, 6);
    var cs7 =
        SIMD.Int8x16.extractLane(t, 7) != SIMD.Int8x16.extractLane(other, 7);
    var cs8 =
        SIMD.Int8x16.extractLane(t, 8) != SIMD.Int8x16.extractLane(other, 8);
    var cs9 =
        SIMD.Int8x16.extractLane(t, 9) != SIMD.Int8x16.extractLane(other, 9);
    var cs10 =
        SIMD.Int8x16.extractLane(t, 10) != SIMD.Int8x16.extractLane(other, 10);
    var cs11 =
        SIMD.Int8x16.extractLane(t, 11) != SIMD.Int8x16.extractLane(other, 11);
    var cs12 =
        SIMD.Int8x16.extractLane(t, 12) != SIMD.Int8x16.extractLane(other, 12);
    var cs13 =
        SIMD.Int8x16.extractLane(t, 13) != SIMD.Int8x16.extractLane(other, 13);
    var cs14 =
        SIMD.Int8x16.extractLane(t, 14) != SIMD.Int8x16.extractLane(other, 14);
    var cs15 =
        SIMD.Int8x16.extractLane(t, 15) != SIMD.Int8x16.extractLane(other, 15);
    return SIMD.Bool8x16(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                         cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.Int8x16.greaterThan === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @param {Int8x16} other An instance of Int8x16.
    * @return {Bool8x16} true or false in each lane depending on
    * the result of t > other.
    */
  SIMD.Int8x16.greaterThan = function(t, other) {
    t = SIMD.Int8x16.check(t);
    other = SIMD.Int8x16.check(other);
    var cs0 =
        SIMD.Int8x16.extractLane(t, 0) > SIMD.Int8x16.extractLane(other, 0);
    var cs1 =
        SIMD.Int8x16.extractLane(t, 1) > SIMD.Int8x16.extractLane(other, 1);
    var cs2 =
        SIMD.Int8x16.extractLane(t, 2) > SIMD.Int8x16.extractLane(other, 2);
    var cs3 =
        SIMD.Int8x16.extractLane(t, 3) > SIMD.Int8x16.extractLane(other, 3);
    var cs4 =
        SIMD.Int8x16.extractLane(t, 4) > SIMD.Int8x16.extractLane(other, 4);
    var cs5 =
        SIMD.Int8x16.extractLane(t, 5) > SIMD.Int8x16.extractLane(other, 5);
    var cs6 =
        SIMD.Int8x16.extractLane(t, 6) > SIMD.Int8x16.extractLane(other, 6);
    var cs7 =
        SIMD.Int8x16.extractLane(t, 7) > SIMD.Int8x16.extractLane(other, 7);
    var cs8 =
        SIMD.Int8x16.extractLane(t, 8) > SIMD.Int8x16.extractLane(other, 8);
    var cs9 =
        SIMD.Int8x16.extractLane(t, 9) > SIMD.Int8x16.extractLane(other, 9);
    var cs10 =
        SIMD.Int8x16.extractLane(t, 10) > SIMD.Int8x16.extractLane(other, 10);
    var cs11 =
        SIMD.Int8x16.extractLane(t, 11) > SIMD.Int8x16.extractLane(other, 11);
    var cs12 =
        SIMD.Int8x16.extractLane(t, 12) > SIMD.Int8x16.extractLane(other, 12);
    var cs13 =
        SIMD.Int8x16.extractLane(t, 13) > SIMD.Int8x16.extractLane(other, 13);
    var cs14 =
        SIMD.Int8x16.extractLane(t, 14) > SIMD.Int8x16.extractLane(other, 14);
    var cs15 =
        SIMD.Int8x16.extractLane(t, 15) > SIMD.Int8x16.extractLane(other, 15);
    return SIMD.Bool8x16(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                         cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.Int8x16.unsignedGreaterThan === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @param {Int8x16} other An instance of Int8x16.
    * @return {Bool8x16} true or false in each lane depending on
    * the result of t > other as unsigned values.
    */
  SIMD.Int8x16.unsignedGreaterThan = function(t, other) {
    t = SIMD.Int8x16.check(t);
    other = SIMD.Int8x16.check(other);
    var cs0 =
        SIMD.Int8x16.unsignedExtractLane(t, 0) > SIMD.Int8x16.unsignedExtractLane(other, 0);
    var cs1 =
        SIMD.Int8x16.unsignedExtractLane(t, 1) > SIMD.Int8x16.unsignedExtractLane(other, 1);
    var cs2 =
        SIMD.Int8x16.unsignedExtractLane(t, 2) > SIMD.Int8x16.unsignedExtractLane(other, 2);
    var cs3 =
        SIMD.Int8x16.unsignedExtractLane(t, 3) > SIMD.Int8x16.unsignedExtractLane(other, 3);
    var cs4 =
        SIMD.Int8x16.unsignedExtractLane(t, 4) > SIMD.Int8x16.unsignedExtractLane(other, 4);
    var cs5 =
        SIMD.Int8x16.unsignedExtractLane(t, 5) > SIMD.Int8x16.unsignedExtractLane(other, 5);
    var cs6 =
        SIMD.Int8x16.unsignedExtractLane(t, 6) > SIMD.Int8x16.unsignedExtractLane(other, 6);
    var cs7 =
        SIMD.Int8x16.unsignedExtractLane(t, 7) > SIMD.Int8x16.unsignedExtractLane(other, 7);
    var cs8 =
        SIMD.Int8x16.unsignedExtractLane(t, 8) > SIMD.Int8x16.unsignedExtractLane(other, 8);
    var cs9 =
        SIMD.Int8x16.unsignedExtractLane(t, 9) > SIMD.Int8x16.unsignedExtractLane(other, 9);
    var cs10 =
        SIMD.Int8x16.unsignedExtractLane(t, 10) > SIMD.Int8x16.unsignedExtractLane(other, 10);
    var cs11 =
        SIMD.Int8x16.unsignedExtractLane(t, 11) > SIMD.Int8x16.unsignedExtractLane(other, 11);
    var cs12 =
        SIMD.Int8x16.unsignedExtractLane(t, 12) > SIMD.Int8x16.unsignedExtractLane(other, 12);
    var cs13 =
        SIMD.Int8x16.unsignedExtractLane(t, 13) > SIMD.Int8x16.unsignedExtractLane(other, 13);
    var cs14 =
        SIMD.Int8x16.unsignedExtractLane(t, 14) > SIMD.Int8x16.unsignedExtractLane(other, 14);
    var cs15 =
        SIMD.Int8x16.unsignedExtractLane(t, 15) > SIMD.Int8x16.unsignedExtractLane(other, 15);
    return SIMD.Bool8x16(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                         cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.Int8x16.greaterThanOrEqual === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @param {Int8x16} other An instance of Int8x16.
    * @return {Bool8x16} true or false in each lane depending on
    * the result of t >= other.
    */
  SIMD.Int8x16.greaterThanOrEqual = function(t, other) {
    t = SIMD.Int8x16.check(t);
    other = SIMD.Int8x16.check(other);
    var cs0 =
        SIMD.Int8x16.extractLane(t, 0) >= SIMD.Int8x16.extractLane(other, 0);
    var cs1 =
        SIMD.Int8x16.extractLane(t, 1) >= SIMD.Int8x16.extractLane(other, 1);
    var cs2 =
        SIMD.Int8x16.extractLane(t, 2) >= SIMD.Int8x16.extractLane(other, 2);
    var cs3 =
        SIMD.Int8x16.extractLane(t, 3) >= SIMD.Int8x16.extractLane(other, 3);
    var cs4 =
        SIMD.Int8x16.extractLane(t, 4) >= SIMD.Int8x16.extractLane(other, 4);
    var cs5 =
        SIMD.Int8x16.extractLane(t, 5) >= SIMD.Int8x16.extractLane(other, 5);
    var cs6 =
        SIMD.Int8x16.extractLane(t, 6) >= SIMD.Int8x16.extractLane(other, 6);
    var cs7 =
        SIMD.Int8x16.extractLane(t, 7) >= SIMD.Int8x16.extractLane(other, 7);
    var cs8 =
        SIMD.Int8x16.extractLane(t, 8) >= SIMD.Int8x16.extractLane(other, 8);
    var cs9 =
        SIMD.Int8x16.extractLane(t, 9) >= SIMD.Int8x16.extractLane(other, 9);
    var cs10 =
        SIMD.Int8x16.extractLane(t, 10) >= SIMD.Int8x16.extractLane(other, 10);
    var cs11 =
        SIMD.Int8x16.extractLane(t, 11) >= SIMD.Int8x16.extractLane(other, 11);
    var cs12 =
        SIMD.Int8x16.extractLane(t, 12) >= SIMD.Int8x16.extractLane(other, 12);
    var cs13 =
        SIMD.Int8x16.extractLane(t, 13) >= SIMD.Int8x16.extractLane(other, 13);
    var cs14 =
        SIMD.Int8x16.extractLane(t, 14) >= SIMD.Int8x16.extractLane(other, 14);
    var cs15 =
        SIMD.Int8x16.extractLane(t, 15) >= SIMD.Int8x16.extractLane(other, 15);
    return SIMD.Bool8x16(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                         cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.Int8x16.unsignedGreaterThanOrEqual === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @param {Int8x16} other An instance of Int8x16.
    * @return {Bool8x16} true or false in each lane depending on
    * the result of t >= other as unsigned values.
    */
  SIMD.Int8x16.unsignedGreaterThanOrEqual = function(t, other) {
    t = SIMD.Int8x16.check(t);
    other = SIMD.Int8x16.check(other);
    var cs0 =
        SIMD.Int8x16.unsignedExtractLane(t, 0) >= SIMD.Int8x16.unsignedExtractLane(other, 0);
    var cs1 =
        SIMD.Int8x16.unsignedExtractLane(t, 1) >= SIMD.Int8x16.unsignedExtractLane(other, 1);
    var cs2 =
        SIMD.Int8x16.unsignedExtractLane(t, 2) >= SIMD.Int8x16.unsignedExtractLane(other, 2);
    var cs3 =
        SIMD.Int8x16.unsignedExtractLane(t, 3) >= SIMD.Int8x16.unsignedExtractLane(other, 3);
    var cs4 =
        SIMD.Int8x16.unsignedExtractLane(t, 4) >= SIMD.Int8x16.unsignedExtractLane(other, 4);
    var cs5 =
        SIMD.Int8x16.unsignedExtractLane(t, 5) >= SIMD.Int8x16.unsignedExtractLane(other, 5);
    var cs6 =
        SIMD.Int8x16.unsignedExtractLane(t, 6) >= SIMD.Int8x16.unsignedExtractLane(other, 6);
    var cs7 =
        SIMD.Int8x16.unsignedExtractLane(t, 7) >= SIMD.Int8x16.unsignedExtractLane(other, 7);
    var cs8 =
        SIMD.Int8x16.unsignedExtractLane(t, 8) >= SIMD.Int8x16.unsignedExtractLane(other, 8);
    var cs9 =
        SIMD.Int8x16.unsignedExtractLane(t, 9) >= SIMD.Int8x16.unsignedExtractLane(other, 9);
    var cs10 =
        SIMD.Int8x16.unsignedExtractLane(t, 10) >= SIMD.Int8x16.unsignedExtractLane(other, 10);
    var cs11 =
        SIMD.Int8x16.unsignedExtractLane(t, 11) >= SIMD.Int8x16.unsignedExtractLane(other, 11);
    var cs12 =
        SIMD.Int8x16.unsignedExtractLane(t, 12) >= SIMD.Int8x16.unsignedExtractLane(other, 12);
    var cs13 =
        SIMD.Int8x16.unsignedExtractLane(t, 13) >= SIMD.Int8x16.unsignedExtractLane(other, 13);
    var cs14 =
        SIMD.Int8x16.unsignedExtractLane(t, 14) >= SIMD.Int8x16.unsignedExtractLane(other, 14);
    var cs15 =
        SIMD.Int8x16.unsignedExtractLane(t, 15) >= SIMD.Int8x16.unsignedExtractLane(other, 15);
    return SIMD.Bool8x16(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                         cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.Int8x16.lessThan === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @param {Int8x16} other An instance of Int8x16.
    * @return {Bool8x16} true or false in each lane depending on
    * the result of t < other.
    */
  SIMD.Int8x16.lessThan = function(t, other) {
    t = SIMD.Int8x16.check(t);
    other = SIMD.Int8x16.check(other);
    var cs0 =
        SIMD.Int8x16.extractLane(t, 0) < SIMD.Int8x16.extractLane(other, 0);
    var cs1 =
        SIMD.Int8x16.extractLane(t, 1) < SIMD.Int8x16.extractLane(other, 1);
    var cs2 =
        SIMD.Int8x16.extractLane(t, 2) < SIMD.Int8x16.extractLane(other, 2);
    var cs3 =
        SIMD.Int8x16.extractLane(t, 3) < SIMD.Int8x16.extractLane(other, 3);
    var cs4 =
        SIMD.Int8x16.extractLane(t, 4) < SIMD.Int8x16.extractLane(other, 4);
    var cs5 =
        SIMD.Int8x16.extractLane(t, 5) < SIMD.Int8x16.extractLane(other, 5);
    var cs6 =
        SIMD.Int8x16.extractLane(t, 6) < SIMD.Int8x16.extractLane(other, 6);
    var cs7 =
        SIMD.Int8x16.extractLane(t, 7) < SIMD.Int8x16.extractLane(other, 7);
    var cs8 =
        SIMD.Int8x16.extractLane(t, 8) < SIMD.Int8x16.extractLane(other, 8);
    var cs9 =
        SIMD.Int8x16.extractLane(t, 9) < SIMD.Int8x16.extractLane(other, 9);
    var cs10 =
        SIMD.Int8x16.extractLane(t, 10) < SIMD.Int8x16.extractLane(other, 10);
    var cs11 =
        SIMD.Int8x16.extractLane(t, 11) < SIMD.Int8x16.extractLane(other, 11);
    var cs12 =
        SIMD.Int8x16.extractLane(t, 12) < SIMD.Int8x16.extractLane(other, 12);
    var cs13 =
        SIMD.Int8x16.extractLane(t, 13) < SIMD.Int8x16.extractLane(other, 13);
    var cs14 =
        SIMD.Int8x16.extractLane(t, 14) < SIMD.Int8x16.extractLane(other, 14);
    var cs15 =
        SIMD.Int8x16.extractLane(t, 15) < SIMD.Int8x16.extractLane(other, 15);
    return SIMD.Bool8x16(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                         cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.Int8x16.unsignedLessThan === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @param {Int8x16} other An instance of Int8x16.
    * @return {Bool8x16} true or false in each lane depending on
    * the result of t < other as unsigned values.
    */
  SIMD.Int8x16.unsignedLessThan = function(t, other) {
    t = SIMD.Int8x16.check(t);
    other = SIMD.Int8x16.check(other);
    var cs0 =
        SIMD.Int8x16.unsignedExtractLane(t, 0) < SIMD.Int8x16.unsignedExtractLane(other, 0);
    var cs1 =
        SIMD.Int8x16.unsignedExtractLane(t, 1) < SIMD.Int8x16.unsignedExtractLane(other, 1);
    var cs2 =
        SIMD.Int8x16.unsignedExtractLane(t, 2) < SIMD.Int8x16.unsignedExtractLane(other, 2);
    var cs3 =
        SIMD.Int8x16.unsignedExtractLane(t, 3) < SIMD.Int8x16.unsignedExtractLane(other, 3);
    var cs4 =
        SIMD.Int8x16.unsignedExtractLane(t, 4) < SIMD.Int8x16.unsignedExtractLane(other, 4);
    var cs5 =
        SIMD.Int8x16.unsignedExtractLane(t, 5) < SIMD.Int8x16.unsignedExtractLane(other, 5);
    var cs6 =
        SIMD.Int8x16.unsignedExtractLane(t, 6) < SIMD.Int8x16.unsignedExtractLane(other, 6);
    var cs7 =
        SIMD.Int8x16.unsignedExtractLane(t, 7) < SIMD.Int8x16.unsignedExtractLane(other, 7);
    var cs8 =
        SIMD.Int8x16.unsignedExtractLane(t, 8) < SIMD.Int8x16.unsignedExtractLane(other, 8);
    var cs9 =
        SIMD.Int8x16.unsignedExtractLane(t, 9) < SIMD.Int8x16.unsignedExtractLane(other, 9);
    var cs10 =
        SIMD.Int8x16.unsignedExtractLane(t, 10) < SIMD.Int8x16.unsignedExtractLane(other, 10);
    var cs11 =
        SIMD.Int8x16.unsignedExtractLane(t, 11) < SIMD.Int8x16.unsignedExtractLane(other, 11);
    var cs12 =
        SIMD.Int8x16.unsignedExtractLane(t, 12) < SIMD.Int8x16.unsignedExtractLane(other, 12);
    var cs13 =
        SIMD.Int8x16.unsignedExtractLane(t, 13) < SIMD.Int8x16.unsignedExtractLane(other, 13);
    var cs14 =
        SIMD.Int8x16.unsignedExtractLane(t, 14) < SIMD.Int8x16.unsignedExtractLane(other, 14);
    var cs15 =
        SIMD.Int8x16.unsignedExtractLane(t, 15) < SIMD.Int8x16.unsignedExtractLane(other, 15);
    return SIMD.Bool8x16(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                         cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.Int8x16.lessThanOrEqual === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @param {Int8x16} other An instance of Int8x16.
    * @return {Bool8x16} true or false in each lane depending on
    * the result of t <= other.
    */
  SIMD.Int8x16.lessThanOrEqual = function(t, other) {
    t = SIMD.Int8x16.check(t);
    other = SIMD.Int8x16.check(other);
    var cs0 =
        SIMD.Int8x16.extractLane(t, 0) <= SIMD.Int8x16.extractLane(other, 0);
    var cs1 =
        SIMD.Int8x16.extractLane(t, 1) <= SIMD.Int8x16.extractLane(other, 1);
    var cs2 =
        SIMD.Int8x16.extractLane(t, 2) <= SIMD.Int8x16.extractLane(other, 2);
    var cs3 =
        SIMD.Int8x16.extractLane(t, 3) <= SIMD.Int8x16.extractLane(other, 3);
    var cs4 =
        SIMD.Int8x16.extractLane(t, 4) <= SIMD.Int8x16.extractLane(other, 4);
    var cs5 =
        SIMD.Int8x16.extractLane(t, 5) <= SIMD.Int8x16.extractLane(other, 5);
    var cs6 =
        SIMD.Int8x16.extractLane(t, 6) <= SIMD.Int8x16.extractLane(other, 6);
    var cs7 =
        SIMD.Int8x16.extractLane(t, 7) <= SIMD.Int8x16.extractLane(other, 7);
    var cs8 =
        SIMD.Int8x16.extractLane(t, 8) <= SIMD.Int8x16.extractLane(other, 8);
    var cs9 =
        SIMD.Int8x16.extractLane(t, 9) <= SIMD.Int8x16.extractLane(other, 9);
    var cs10 =
        SIMD.Int8x16.extractLane(t, 10) <= SIMD.Int8x16.extractLane(other, 10);
    var cs11 =
        SIMD.Int8x16.extractLane(t, 11) <= SIMD.Int8x16.extractLane(other, 11);
    var cs12 =
        SIMD.Int8x16.extractLane(t, 12) <= SIMD.Int8x16.extractLane(other, 12);
    var cs13 =
        SIMD.Int8x16.extractLane(t, 13) <= SIMD.Int8x16.extractLane(other, 13);
    var cs14 =
        SIMD.Int8x16.extractLane(t, 14) <= SIMD.Int8x16.extractLane(other, 14);
    var cs15 =
        SIMD.Int8x16.extractLane(t, 15) <= SIMD.Int8x16.extractLane(other, 15);
    return SIMD.Bool8x16(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                         cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.Int8x16.unsignedLessThanOrEqual === "undefined") {
  /**
    * @param {Int8x16} t An instance of Int8x16.
    * @param {Int8x16} other An instance of Int8x16.
    * @return {Bool8x16} true or false in each lane depending on
    * the result of t <= other as unsigned values.
    */
  SIMD.Int8x16.unsignedLessThanOrEqual = function(t, other) {
    t = SIMD.Int8x16.check(t);
    other = SIMD.Int8x16.check(other);
    var cs0 =
        SIMD.Int8x16.unsignedExtractLane(t, 0) <= SIMD.Int8x16.unsignedExtractLane(other, 0);
    var cs1 =
        SIMD.Int8x16.unsignedExtractLane(t, 1) <= SIMD.Int8x16.unsignedExtractLane(other, 1);
    var cs2 =
        SIMD.Int8x16.unsignedExtractLane(t, 2) <= SIMD.Int8x16.unsignedExtractLane(other, 2);
    var cs3 =
        SIMD.Int8x16.unsignedExtractLane(t, 3) <= SIMD.Int8x16.unsignedExtractLane(other, 3);
    var cs4 =
        SIMD.Int8x16.unsignedExtractLane(t, 4) <= SIMD.Int8x16.unsignedExtractLane(other, 4);
    var cs5 =
        SIMD.Int8x16.unsignedExtractLane(t, 5) <= SIMD.Int8x16.unsignedExtractLane(other, 5);
    var cs6 =
        SIMD.Int8x16.unsignedExtractLane(t, 6) <= SIMD.Int8x16.unsignedExtractLane(other, 6);
    var cs7 =
        SIMD.Int8x16.unsignedExtractLane(t, 7) <= SIMD.Int8x16.unsignedExtractLane(other, 7);
    var cs8 =
        SIMD.Int8x16.unsignedExtractLane(t, 8) <= SIMD.Int8x16.unsignedExtractLane(other, 8);
    var cs9 =
        SIMD.Int8x16.unsignedExtractLane(t, 9) <= SIMD.Int8x16.unsignedExtractLane(other, 9);
    var cs10 =
        SIMD.Int8x16.unsignedExtractLane(t, 10) <= SIMD.Int8x16.unsignedExtractLane(other, 10);
    var cs11 =
        SIMD.Int8x16.unsignedExtractLane(t, 11) <= SIMD.Int8x16.unsignedExtractLane(other, 11);
    var cs12 =
        SIMD.Int8x16.unsignedExtractLane(t, 12) <= SIMD.Int8x16.unsignedExtractLane(other, 12);
    var cs13 =
        SIMD.Int8x16.unsignedExtractLane(t, 13) <= SIMD.Int8x16.unsignedExtractLane(other, 13);
    var cs14 =
        SIMD.Int8x16.unsignedExtractLane(t, 14) <= SIMD.Int8x16.unsignedExtractLane(other, 14);
    var cs15 =
        SIMD.Int8x16.unsignedExtractLane(t, 15) <= SIMD.Int8x16.unsignedExtractLane(other, 15);
    return SIMD.Bool8x16(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                         cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.Int8x16.shiftLeftByScalar === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {integer} bits Bit count to shift by.
    * @return {Int8x16} lanes in a shifted by bits.
    */
  SIMD.Int8x16.shiftLeftByScalar = function(a, bits) {
    a = SIMD.Int8x16.check(a);
    if (bits>>>0 > 8)
      bits = 8;
    var s0 = SIMD.Int8x16.extractLane(a, 0) << bits;
    var s1 = SIMD.Int8x16.extractLane(a, 1) << bits;
    var s2 = SIMD.Int8x16.extractLane(a, 2) << bits;
    var s3 = SIMD.Int8x16.extractLane(a, 3) << bits;
    var s4 = SIMD.Int8x16.extractLane(a, 4) << bits;
    var s5 = SIMD.Int8x16.extractLane(a, 5) << bits;
    var s6 = SIMD.Int8x16.extractLane(a, 6) << bits;
    var s7 = SIMD.Int8x16.extractLane(a, 7) << bits;
    var s8 = SIMD.Int8x16.extractLane(a, 8) << bits;
    var s9 = SIMD.Int8x16.extractLane(a, 9) << bits;
    var s10 = SIMD.Int8x16.extractLane(a, 10) << bits;
    var s11 = SIMD.Int8x16.extractLane(a, 11) << bits;
    var s12 = SIMD.Int8x16.extractLane(a, 12) << bits;
    var s13 = SIMD.Int8x16.extractLane(a, 13) << bits;
    var s14 = SIMD.Int8x16.extractLane(a, 14) << bits;
    var s15 = SIMD.Int8x16.extractLane(a, 15) << bits;
    return SIMD.Int8x16(s0, s1, s2, s3, s4, s5, s6, s7,
                        s8, s9, s10, s11, s12, s13, s14, s15);
  }
}

if (typeof SIMD.Int8x16.shiftRightLogicalByScalar === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {integer} bits Bit count to shift by.
    * @return {Int8x16} lanes in a shifted by bits.
    */
  SIMD.Int8x16.shiftRightLogicalByScalar = function(a, bits) {
    a = SIMD.Int8x16.check(a);
    if (bits>>>0 > 8)
      bits = 8;
    var s0 = (SIMD.Int8x16.extractLane(a, 0) & 0xff) >>> bits;
    var s1 = (SIMD.Int8x16.extractLane(a, 1) & 0xff) >>> bits;
    var s2 = (SIMD.Int8x16.extractLane(a, 2) & 0xff) >>> bits;
    var s3 = (SIMD.Int8x16.extractLane(a, 3) & 0xff) >>> bits;
    var s4 = (SIMD.Int8x16.extractLane(a, 4) & 0xff) >>> bits;
    var s5 = (SIMD.Int8x16.extractLane(a, 5) & 0xff) >>> bits;
    var s6 = (SIMD.Int8x16.extractLane(a, 6) & 0xff) >>> bits;
    var s7 = (SIMD.Int8x16.extractLane(a, 7) & 0xff) >>> bits;
    var s8 = (SIMD.Int8x16.extractLane(a, 8) & 0xff) >>> bits;
    var s9 = (SIMD.Int8x16.extractLane(a, 9) & 0xff) >>> bits;
    var s10 = (SIMD.Int8x16.extractLane(a, 10) & 0xff) >>> bits;
    var s11 = (SIMD.Int8x16.extractLane(a, 11) & 0xff) >>> bits;
    var s12 = (SIMD.Int8x16.extractLane(a, 12) & 0xff) >>> bits;
    var s13 = (SIMD.Int8x16.extractLane(a, 13) & 0xff) >>> bits;
    var s14 = (SIMD.Int8x16.extractLane(a, 14) & 0xff) >>> bits;
    var s15 = (SIMD.Int8x16.extractLane(a, 15) & 0xff) >>> bits;
    return SIMD.Int8x16(s0, s1, s2, s3, s4, s5, s6, s7,
                        s8, s9, s10, s11, s12, s13, s14, s15);
  }
}

if (typeof SIMD.Int8x16.shiftRightArithmeticByScalar === "undefined") {
  /**
    * @param {Int8x16} a An instance of Int8x16.
    * @param {integer} bits Bit count to shift by.
    * @return {Int8x16} lanes in a shifted by bits.
    */
  SIMD.Int8x16.shiftRightArithmeticByScalar = function(a, bits) {
    a = SIMD.Int8x16.check(a);
    if (bits>>>0 > 8)
      bits = 8;
    var s0 = SIMD.Int8x16.extractLane(a, 0) >> bits;
    var s1 = SIMD.Int8x16.extractLane(a, 1) >> bits;
    var s2 = SIMD.Int8x16.extractLane(a, 2) >> bits;
    var s3 = SIMD.Int8x16.extractLane(a, 3) >> bits;
    var s4 = SIMD.Int8x16.extractLane(a, 4) >> bits;
    var s5 = SIMD.Int8x16.extractLane(a, 5) >> bits;
    var s6 = SIMD.Int8x16.extractLane(a, 6) >> bits;
    var s7 = SIMD.Int8x16.extractLane(a, 7) >> bits;
    var s8 = SIMD.Int8x16.extractLane(a, 8) >> bits;
    var s9 = SIMD.Int8x16.extractLane(a, 9) >> bits;
    var s10 = SIMD.Int8x16.extractLane(a, 10) >> bits;
    var s11 = SIMD.Int8x16.extractLane(a, 11) >> bits;
    var s12 = SIMD.Int8x16.extractLane(a, 12) >> bits;
    var s13 = SIMD.Int8x16.extractLane(a, 13) >> bits;
    var s14 = SIMD.Int8x16.extractLane(a, 14) >> bits;
    var s15 = SIMD.Int8x16.extractLane(a, 15) >> bits;
    return SIMD.Int8x16(s0, s1, s2, s3, s4, s5, s6, s7,
                        s8, s9, s10, s11, s12, s13, s14, s15);
  }
}

if (typeof SIMD.Int8x16.load === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {Int8x16} New instance of Int8x16.
    */
  SIMD.Int8x16.load = function(tarray, index) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var i8temp = _i8x16;
    var array = bpe == 1 ? i8temp :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : _i32x4) :
                _f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.Int8x16(i8temp[0], i8temp[1], i8temp[2], i8temp[3],
                        i8temp[4], i8temp[5], i8temp[6], i8temp[7],
                        i8temp[8], i8temp[9], i8temp[10], i8temp[11],
                        i8temp[12], i8temp[13], i8temp[14], i8temp[15]);
  }
}

if (typeof SIMD.Int8x16.store === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {Int8x16} value An instance of Int8x16.
    * @return {Int8x16} value
    */
  SIMD.Int8x16.store = function(tarray, index, value) {
    if (!isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!isInt32(index))
      throw new TypeError("The 2nd argument must be an Int32.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.Int8x16.check(value);
    _i8x16[0] = SIMD.Int8x16.extractLane(value, 0);
    _i8x16[1] = SIMD.Int8x16.extractLane(value, 1);
    _i8x16[2] = SIMD.Int8x16.extractLane(value, 2);
    _i8x16[3] = SIMD.Int8x16.extractLane(value, 3);
    _i8x16[4] = SIMD.Int8x16.extractLane(value, 4);
    _i8x16[5] = SIMD.Int8x16.extractLane(value, 5);
    _i8x16[6] = SIMD.Int8x16.extractLane(value, 6);
    _i8x16[7] = SIMD.Int8x16.extractLane(value, 7);
    _i8x16[8] = SIMD.Int8x16.extractLane(value, 8);
    _i8x16[9] = SIMD.Int8x16.extractLane(value, 9);
    _i8x16[10] = SIMD.Int8x16.extractLane(value, 10);
    _i8x16[11] = SIMD.Int8x16.extractLane(value, 11);
    _i8x16[12] = SIMD.Int8x16.extractLane(value, 12);
    _i8x16[13] = SIMD.Int8x16.extractLane(value, 13);
    _i8x16[14] = SIMD.Int8x16.extractLane(value, 14);
    _i8x16[15] = SIMD.Int8x16.extractLane(value, 15);
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : _i32x4) :
                _f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      tarray[index + i] = array[i];
    return value;
  }
}

// If we're in a browser, the global namespace is named 'window'. If we're
// in node, it's named 'global'. If we're in a shell, 'this' might work.
})(typeof window !== "undefined"
   ? window
   : (typeof process === 'object' &&
      typeof require === 'function' &&
      typeof global === 'object')
     ? global
     : this);
