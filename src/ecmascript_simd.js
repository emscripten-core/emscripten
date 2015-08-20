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

// Buffers for bit casting and coercing lane values to those representable in
// the underlying lane type.
var _f32x4 = new Float32Array(4);
var _f64x2 = new Float64Array(_f32x4.buffer);
var _i32x4 = new Int32Array(_f32x4.buffer);
var _i16x8 = new Int16Array(_f32x4.buffer);
var _i8x16 = new Int8Array(_f32x4.buffer);
var _ui32x4 = new Uint32Array(_f32x4.buffer);
var _ui16x8 = new Uint16Array(_f32x4.buffer);
var _ui8x16 = new Uint8Array(_f32x4.buffer);

function convertValue(buffer, value) {
  buffer[0] = value;
  return buffer[0];
}

function convertArray(buffer, array) {
  for (var i = 0; i < array.length; i++)
    array[i] = convertValue(buffer, array[i]);
  return array;
}

// Utility functions.

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

function clamp(a, min, max) {
  if (a < min)
    return min;
  if (a > max)
    return max;
  return a;
}

// SIMD implementation functions

function simdCheckLaneIndex(index, lanes) {
  if (!isInt32(index))
    throw new TypeError('Lane index must be an int32');
  if (index < 0 || index >= lanes)
    throw new RangeError('Lane index must be in bounds');
}

// Global lanes array for constructing SIMD values.
var lanes = [];

function simdCreate(type) {
  return type.fn.apply(type.fn, lanes);
}

function simdToString(type, a) {
  a = type.fn.check(a);
  var str = "SIMD." + type.name + "(";
  str += type.fn.extractLane(a, 0);
  for (var i = 1; i < type.lanes; i++) {
    str += ", " + type.fn.extractLane(a, i);
  }
  return str + ")";
}

function simdToLocaleString(type, a) {
  a = type.fn.check(a);
  var str = "SIMD." + type.name + "(";
  str += type.fn.extractLane(a, 0).toLocaleString();
  for (var i = 1; i < type.lanes; i++) {
    str += ", " + type.fn.extractLane(a, i).toLocaleString();
  }
  return str + ")";
}

function simdSplat(type, s) {
  for (var i = 0; i < type.lanes; i++)
    lanes[i] = s;
  return simdCreate(type);
}

function simdReplaceLane(type, a, i, s) {
  a = type.fn.check(a);
  simdCheckLaneIndex(i, type.lanes);
  for (var j = 0; j < type.lanes; j++)
    lanes[j] = type.fn.extractLane(a, j);
  lanes[i] = s;
  return simdCreate(type);
}

function simdFrom(toType, fromType, a) {
  a = fromType.fn.check(a);
  for (var i = 0; i < fromType.lanes; i++) {
    var v = fromType.fn.extractLane(a, i);
    if (toType.minVal !== undefined &&
        (v < toType.minVal || v > toType.maxVal)) {
      throw new RangeError("Can't convert value");
    }
    lanes[i] = v;
  }
  return simdCreate(toType);
}

function simdFromBits(toType, fromType, a) {
  a = fromType.fn.check(a);
  for (var i = 0; i < fromType.lanes; i++)
    fromType.buffer[i] = fromType.fn.extractLane(a, i);
  for (var i = 0; i < toType.lanes; i++)
    lanes[i] = toType.buffer[i];
  return simdCreate(toType);
}

function simdSelect(type, selector, a, b) {
  selector = type.boolType.fn.check(selector);
  a = type.fn.check(a);
  b = type.fn.check(b);
  for (var i = 0; i < type.lanes; i++) {
    lanes[i] = type.boolType.fn.extractLane(selector, i) ?
               type.fn.extractLane(a, i) : type.fn.extractLane(b, i);
  }
  return simdCreate(type);
}

function simdSwizzle(type, a, indices) {
  a = type.fn.check(a);
  for (var i = 0; i < indices.length; i++) {
    simdCheckLaneIndex(indices[i], type.lanes);
    lanes[i] = type.fn.extractLane(a, indices[i]);
  }
  return simdCreate(type);
}

function simdShuffle(type, a, b, indices) {
  a = type.fn.check(a);
  b = type.fn.check(b);
  for (var i = 0; i < indices.length; i++) {
    simdCheckLaneIndex(indices[i], 2 * type.lanes);
    lanes[i] = indices[i] < type.lanes ?
               type.fn.extractLane(a, indices[i]) :
               type.fn.extractLane(b, indices[i] - type.lanes);
  }
  return simdCreate(type);
}

function unaryNeg(a) { return -a; }
function unaryBitwiseNot(a) { return ~a; }
function unaryLogicalNot(a) { return !a; }

function simdUnaryOp(type, op, a) {
  a = type.fn.check(a);
  for (var i = 0; i < type.lanes; i++)
    lanes[i] = op(type.fn.extractLane(a, i));
  return simdCreate(type);
}

function binaryAnd(a, b) { return a & b; }
function binaryOr(a, b) { return a | b; }
function binaryXor(a, b) { return a ^ b; }
function binaryAdd(a, b) { return a + b; }
function binarySub(a, b) { return a - b; }
function binaryMul(a, b) { return a * b; }
function binaryDiv(a, b) { return a / b; }
function binaryAbsDiff(a, b) { return Math.abs(a - b); }

var binaryImul;
if (typeof Math.imul !== 'undefined') {
  binaryImul = Math.imul;
} else {
  binaryImul = function(a, b) {
    var ah = (a >>> 16) & 0xffff;
    var al = a & 0xffff;
    var bh = (b >>> 16) & 0xffff;
    var bl = b & 0xffff;
    // the shift by 0 fixes the sign on the high part
    // the final |0 converts the unsigned value into a signed value
    return ((al * bl) + (((ah * bl + al * bh) << 16) >>> 0)|0);
  };
}

function simdBinaryOp(type, op, a, b) {
  a = type.fn.check(a);
  b = type.fn.check(b);
  for (var i = 0; i < type.lanes; i++)
    lanes[i] = op(type.fn.extractLane(a, i), type.fn.extractLane(b, i));
  return simdCreate(type);
}

function simdWideningBinaryOp(type, op, a, b) {
  a = type.fn.check(a);
  b = type.fn.check(b);
  for (var i = 0; i < type.wideType.lanes; i++)
    lanes[i] = op(type.fn.extractLane(a, i),
                  type.fn.extractLane(b, i));
  return simdCreate(type.wideType);
}

function binaryEqual(a, b) { return a == b; }
function binaryNotEqual(a, b) { return a != b; }
function binaryLess(a, b) { return a < b; }
function binaryLessEqual(a, b) { return a <= b; }
function binaryGreater(a, b) { return a > b; }
function binaryGreaterEqual(a, b) { return a >= b; }

function simdRelationalOp(type, op, a, b) {
  a = type.fn.check(a);
  b = type.fn.check(b);
  for (var i = 0; i < type.lanes; i++)
    lanes[i] = op(type.fn.extractLane(a, i), type.fn.extractLane(b, i));
  return simdCreate(type.boolType);
}

function simdAnyTrue(type, a) {
  a = type.fn.check(a);
  for (var i = 0; i < type.lanes; i++)
    if (type.fn.extractLane(a, i)) return true;
  return false;
}

function simdAllTrue(type, a) {
  a = type.fn.check(a);
  for (var i = 0; i < type.lanes; i++)
    if (!type.fn.extractLane(a, i)) return false;
  return true;
}

function binaryShiftLeft(a, bits) { return a << bits; }
function binaryShiftRightArithmetic(a, bits) { return a >> bits; }

function simdShiftOp(type, op, a, bits) {
  a = type.fn.check(a);
  for (var i = 0; i < type.lanes; i++)
    lanes[i] = op(type.fn.extractLane(a, i), bits);
  return simdCreate(type);
}

function simdHorizontalSum(type, a) {
  a = type.fn.check(a);
  var result = 0;
  for (var i = 0; i < type.lanes; i++)
    result += type.fn.extractLane(a, i);
  return result;
}

function simdLoad(type, tarray, index, count) {
  if (!isTypedArray(tarray))
    throw new TypeError("The 1st argument must be a typed array.");
  if (!isInt32(index))
    throw new TypeError("The 2nd argument must be an Int32.");
  var bpe = tarray.BYTES_PER_ELEMENT;
  var bytes = count * type.laneSize;
  if (index < 0 || (index * bpe + bytes) > tarray.byteLength)
    throw new RangeError("The value of index is invalid.");

  var buf = type.buffer;
  var array = bpe == 1 ? _i8x16 :
              bpe == 2 ? _i16x8 :
              bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : _i32x4) :
              _f64x2;
  var n = bytes / bpe;
  for (var i = 0; i < n; i++)
    array[i] = tarray[index + i];
  for (i = 0; i < count; i++) lanes[i] = buf[i];
  for (; i < type.lanes; i++) lanes[i] = 0;
  return simdCreate(type);
}

function simdStore(type, tarray, index, a, count) {
  if (!isTypedArray(tarray))
    throw new TypeError("The 1st argument must be a typed array.");
  if (!isInt32(index))
    throw new TypeError("The 2nd argument must be an Int32.");
  var bpe = tarray.BYTES_PER_ELEMENT;
  var bytes = count * type.laneSize;
  if (index < 0 || (index * bpe + bytes) > tarray.byteLength)
    throw new RangeError("The value of index is invalid.");

  a = type.fn.check(a);
  // If count is odd and tarray's elements are 8 bytes wide, we have to create
  // a new view.
  if ((count % 2 != 0) && bpe == 8) {
    var view = new type.view(tarray.buffer,
                             tarray.byteOffset + index * 8, count);
    for (var i = 0; i < count; i++)
      view[i] = type.fn.extractLane(a, i);
  } else {
    for (var i = 0; i < count; i++)
      type.buffer[i] = type.fn.extractLane(a, i);
    var array = bpe == 1 ? _i8x16 :
                bpe == 2 ? _i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : _i32x4) :
                _f64x2;
    var n = bytes / bpe;
    for (var i = 0; i < n; i++)
      tarray[index + i] = array[i];
  }
  return a;
}

// Constructors and extractLane functions are closely related and must be
// polyfilled together.

// Bool32x4
if (typeof SIMD.Bool32x4 === "undefined" ||
    typeof SIMD.Bool32x4.extractLane === "undefined") {
  SIMD.Bool32x4 = function(s0, s1, s2, s3) {
    if (!(this instanceof SIMD.Bool32x4)) {
      return new SIMD.Bool32x4(s0, s1, s2, s3);
    }
    this.s_ = [!!s0, !!s1, !!s2, !!s3];
  }

  SIMD.Bool32x4.extractLane = function(v, i) {
    v = SIMD.Bool32x4.check(v);
    simdCheckLaneIndex(i, 4);
    return v.s_[i];
  }
}

// Bool16x8
if (typeof SIMD.Bool16x8 === "undefined" ||
    typeof SIMD.Bool16x8.extractLane === "undefined") {
  SIMD.Bool16x8 = function(s0, s1, s2, s3, s4, s5, s6, s7) {
    if (!(this instanceof SIMD.Bool16x8)) {
      return new SIMD.Bool16x8(s0, s1, s2, s3, s4, s5, s6, s7);
    }
    this.s_ = [!!s0, !!s1, !!s2, !!s3, !!s4, !!s5, !!s6, !!s7];
  }

  SIMD.Bool16x8.extractLane = function(v, i) {
    v = SIMD.Bool16x8.check(v);
    simdCheckLaneIndex(i, 8);
    return v.s_[i];
  }
}

// Bool8x16
if (typeof SIMD.Bool8x16 === "undefined" ||
    typeof SIMD.Bool8x16.extractLane === "undefined") {
  SIMD.Bool8x16 = function(s0, s1, s2, s3, s4, s5, s6, s7,
                           s8, s9, s10, s11, s12, s13, s14, s15) {
    if (!(this instanceof SIMD.Bool8x16)) {
      return new SIMD.Bool8x16(s0, s1, s2, s3, s4, s5, s6, s7,
                               s8, s9, s10, s11, s12, s13, s14, s15);
    }
    this.s_ = [!!s0, !!s1, !!s2, !!s3, !!s4, !!s5, !!s6, !!s7,
               !!s8, !!s9, !!s10, !!s11, !!s12, !!s13, !!s14, !!s15];
  }

  SIMD.Bool8x16.extractLane = function(v, i) {
    v = SIMD.Bool8x16.check(v);
    simdCheckLaneIndex(i, 16);
    return v.s_[i];
  }
}

// Float32x4
if (typeof SIMD.Float32x4 === "undefined" ||
    typeof SIMD.Float32x4.extractLane === "undefined") {
  SIMD.Float32x4 = function(s0, s1, s2, s3) {
    if (!(this instanceof SIMD.Float32x4)) {
      return new SIMD.Float32x4(s0, s1, s2, s3);
    }
    this.s_ = convertArray(_f32x4, [s0, s1, s2, s3]);
  }

  SIMD.Float32x4.extractLane = function(v, i) {
    v = SIMD.Float32x4.check(v);
    simdCheckLaneIndex(i, 4);
    return v.s_[i];
  }
}

// Int32x4
if (typeof SIMD.Int32x4 === "undefined" ||
    typeof SIMD.Int32x4.extractLane === "undefined") {
  SIMD.Int32x4 = function(s0, s1, s2, s3) {
    if (!(this instanceof SIMD.Int32x4)) {
      return new SIMD.Int32x4(s0, s1, s2, s3);
    }
    this.s_ = convertArray(_i32x4, [s0, s1, s2, s3]);
  }

  SIMD.Int32x4.extractLane = function(v, i) {
    v = SIMD.Int32x4.check(v);
    simdCheckLaneIndex(i, 4);
    return v.s_[i];
  }
}

// Int16x8
if (typeof SIMD.Int16x8 === "undefined" ||
    typeof SIMD.Int16x8.extractLane === "undefined") {
  SIMD.Int16x8 = function(s0, s1, s2, s3, s4, s5, s6, s7) {
    if (!(this instanceof SIMD.Int16x8)) {
      return new SIMD.Int16x8(s0, s1, s2, s3, s4, s5, s6, s7);
    }
    this.s_ = convertArray(_i16x8, [s0, s1, s2, s3, s4, s5, s6, s7]);
  }

  SIMD.Int16x8.extractLane = function(v, i) {
    v = SIMD.Int16x8.check(v);
    simdCheckLaneIndex(i, 8);
    return v.s_[i];
  }
}

// Int8x16
if (typeof SIMD.Int8x16 === "undefined" ||
    typeof SIMD.Int8x16.extractLane === "undefined") {
  SIMD.Int8x16 = function(s0, s1, s2, s3, s4, s5, s6, s7,
                          s8, s9, s10, s11, s12, s13, s14, s15) {
    if (!(this instanceof SIMD.Int8x16)) {
      return new SIMD.Int8x16(s0, s1, s2, s3, s4, s5, s6, s7,
                              s8, s9, s10, s11, s12, s13, s14, s15);
    }
    this.s_ = convertArray(_i8x16, [s0, s1, s2, s3, s4, s5, s6, s7,
                                    s8, s9, s10, s11, s12, s13, s14, s15]);
  }

  SIMD.Int8x16.extractLane = function(v, i) {
    v = SIMD.Int8x16.check(v);
    simdCheckLaneIndex(i, 16);
    return v.s_[i];
  }
}

// Uint32x4
if (typeof SIMD.Uint32x4 === "undefined" ||
    typeof SIMD.Uint32x4.extractLane === "undefined") {
  SIMD.Uint32x4 = function(s0, s1, s2, s3) {
    if (!(this instanceof SIMD.Uint32x4)) {
      return new SIMD.Uint32x4(s0, s1, s2, s3);
    }
    this.s_ = convertArray(_ui32x4, [s0, s1, s2, s3]);
  }

  SIMD.Uint32x4.extractLane = function(v, i) {
    v = SIMD.Uint32x4.check(v);
    simdCheckLaneIndex(i, 4);
    return v.s_[i];
  }
}

// Uint16x8
if (typeof SIMD.Uint16x8 === "undefined" ||
    typeof SIMD.Uint16x8.extractLane === "undefined") {
  SIMD.Uint16x8 = function(s0, s1, s2, s3, s4, s5, s6, s7) {
    if (!(this instanceof SIMD.Uint16x8)) {
      return new SIMD.Uint16x8(s0, s1, s2, s3, s4, s5, s6, s7);
    }
    this.s_ = convertArray(_ui16x8, [s0, s1, s2, s3, s4, s5, s6, s7]);
  }

  SIMD.Uint16x8.extractLane = function(v, i) {
    v = SIMD.Uint16x8.check(v);
    simdCheckLaneIndex(i, 8);
    return v.s_[i];
  }
}

// Uint8x16
if (typeof SIMD.Uint8x16 === "undefined" ||
    typeof SIMD.Uint8x16.extractLane === "undefined") {
  SIMD.Uint8x16 = function(s0, s1, s2, s3, s4, s5, s6, s7,
                           s8, s9, s10, s11, s12, s13, s14, s15) {
    if (!(this instanceof SIMD.Uint8x16)) {
      return new SIMD.Uint8x16(s0, s1, s2, s3, s4, s5, s6, s7,
                               s8, s9, s10, s11, s12, s13, s14, s15);
    }
    this.s_ = convertArray(_ui8x16, [s0, s1, s2, s3, s4, s5, s6, s7,
                                     s8, s9, s10, s11, s12, s13, s14, s15]);
  }

  SIMD.Uint8x16.extractLane = function(v, i) {
    v = SIMD.Uint8x16.check(v);
    simdCheckLaneIndex(i, 16);
    return v.s_[i];
  }
}

var float32x4 = {
  name: "Float32x4",
  fn: SIMD.Float32x4,
  lanes: 4,
  laneSize: 4,
  buffer: _f32x4,
  view: Float32Array,
  mulFn: binaryMul,
  fns: ["check", "splat", "replaceLane", "select",
        "equal", "notEqual", "lessThan", "lessThanOrEqual", "greaterThan", "greaterThanOrEqual",
        "add", "sub", "mul", "div", "neg", "abs", "min", "max", "minNum", "maxNum",
        "reciprocalApproximation", "reciprocalSqrtApproximation", "sqrt",
        "load", "load1", "load2", "load3", "store", "store1", "store2", "store3"],
}

var int32x4 = {
  name: "Int32x4",
  fn: SIMD.Int32x4,
  lanes: 4,
  laneSize: 4,
  minVal: -0x80000000,
  maxVal: 0x7FFFFFFF,
  buffer: _i32x4,
  notFn: unaryBitwiseNot,
  view: Int32Array,
  mulFn: binaryImul,
  fns: ["check", "splat", "replaceLane", "select",
        "equal", "notEqual", "lessThan", "lessThanOrEqual", "greaterThan", "greaterThanOrEqual",
        "and", "or", "xor", "not",
        "add", "sub", "mul", "neg", "min", "max",
        "shiftLeftByScalar", "shiftRightArithmeticByScalar",
        "load", "load1", "load2", "load3", "store", "store1", "store2", "store3"],
}

var int16x8 = {
  name: "Int16x8",
  fn: SIMD.Int16x8,
  lanes: 8,
  laneSize: 2,
  minVal: -0x8000,
  maxVal: 0x7FFF,
  buffer: _i16x8,
  notFn: unaryBitwiseNot,
  view: Int16Array,
  mulFn: binaryMul,
  fns: ["check", "splat", "replaceLane", "select",
        "equal", "notEqual", "lessThan", "lessThanOrEqual", "greaterThan", "greaterThanOrEqual",
        "and", "or", "xor", "not",
        "add", "sub", "mul", "neg", "min", "max",
        "shiftLeftByScalar", "shiftRightArithmeticByScalar",
        "addSaturate", "subSaturate",
        "load", "store"],
}

var int8x16 = {
  name: "Int8x16",
  fn: SIMD.Int8x16,
  lanes: 16,
  laneSize: 1,
  minVal: -0x80,
  maxVal: 0x7F,
  buffer: _i8x16,
  notFn: unaryBitwiseNot,
  view: Int8Array,
  mulFn: binaryMul,
  fns: ["check", "splat", "replaceLane", "select",
        "equal", "notEqual", "lessThan", "lessThanOrEqual", "greaterThan", "greaterThanOrEqual",
        "and", "or", "xor", "not",
        "add", "sub", "mul", "neg", "min", "max",
        "shiftLeftByScalar", "shiftRightArithmeticByScalar",
        "addSaturate", "subSaturate",
        "load", "store"],
}

var uint32x4 = {
  name: "Uint32x4",
  fn: SIMD.Uint32x4,
  lanes: 4,
  laneSize: 4,
  minVal: 0,
  maxVal: 0xFFFFFFFF,
  unsigned: true,
  buffer: _ui32x4,
  notFn: unaryBitwiseNot,
  view: Uint32Array,
  mulFn: binaryImul,
  fns: ["check", "splat", "replaceLane", "select",
        "equal", "notEqual", "lessThan", "lessThanOrEqual", "greaterThan", "greaterThanOrEqual",
        "and", "or", "xor", "not",
        "add", "sub", "mul", "min", "max",
        "shiftLeftByScalar", "shiftRightLogicalByScalar",
        "horizontalSum",
        "load", "load1", "load2", "load3", "store", "store1", "store2", "store3"],
}

var uint16x8 = {
  name: "Uint16x8",
  fn: SIMD.Uint16x8,
  lanes: 8,
  laneSize: 2,
  unsigned: true,
  minVal: 0,
  maxVal: 0xFFFF,
  buffer: _ui16x8,
  notFn: unaryBitwiseNot,
  view: Uint16Array,
  mulFn: binaryMul,
  fns: ["check", "splat", "replaceLane", "select",
        "equal", "notEqual", "lessThan", "lessThanOrEqual", "greaterThan", "greaterThanOrEqual",
        "and", "or", "xor", "not",
        "add", "sub", "mul", "min", "max",
        "shiftLeftByScalar", "shiftRightLogicalByScalar",
        "horizontalSum", "absoluteDifference", "widenedAbsoluteDifference",
        "addSaturate", "subSaturate",
        "load", "store"],
}

var uint8x16 = {
  name: "Uint8x16",
  fn: SIMD.Uint8x16,
  lanes: 16,
  laneSize: 1,
  unsigned: true,
  minVal: 0,
  maxVal: 0xFF,
  buffer: _ui8x16,
  notFn: unaryBitwiseNot,
  view: Uint8Array,
  mulFn: binaryMul,
  fns: ["check", "splat", "replaceLane", "select",
        "equal", "notEqual", "lessThan", "lessThanOrEqual", "greaterThan", "greaterThanOrEqual",
        "and", "or", "xor", "not",
        "add", "sub", "mul", "min", "max",
        "shiftLeftByScalar", "shiftRightLogicalByScalar",
        "horizontalSum", "absoluteDifference", "widenedAbsoluteDifference",
        "addSaturate", "subSaturate",
        "load", "store"],
}

var bool32x4 = {
  name: "Bool32x4",
  fn: SIMD.Bool32x4,
  lanes: 4,
  laneSize: 4,
  notFn: unaryLogicalNot,
  fns: ["check", "splat", "replaceLane",
        "allTrue", "anyTrue", "and", "or", "xor", "not"],
}

var bool16x8 = {
  name: "Bool16x8",
  fn: SIMD.Bool16x8,
  lanes: 8,
  laneSize: 2,
  notFn: unaryLogicalNot,
  fns: ["check", "splat", "replaceLane",
        "allTrue", "anyTrue", "and", "or", "xor", "not"],
}

var bool8x16 = {
  name: "Bool8x16",
  fn: SIMD.Bool8x16,
  lanes: 16,
  laneSize: 1,
  notFn: unaryLogicalNot,
  fns: ["check", "splat", "replaceLane",
        "allTrue", "anyTrue", "and", "or", "xor", "not"],
}

// Each SIMD type has a corresponding Boolean SIMD type, which is returned by
// relational ops.
float32x4.boolType = int32x4.boolType = uint32x4.boolType = bool32x4.boolType = bool32x4;
int16x8.boolType = uint16x8.boolType = bool16x8.boolType = bool16x8;
int8x16.boolType = uint8x16.boolType = bool8x16.boolType = bool8x16;

// SIMD fromTIMD types.
float32x4.from = [int32x4, uint32x4];
int32x4.from = [float32x4, uint32x4];
int16x8.from = [uint16x8];
int8x16.from = [uint8x16];
uint32x4.from = [float32x4, int32x4];
uint16x8.from = [int16x8];
uint8x16.from = [int8x16];

// SIMD fromTIMDBits types.
float32x4.fromBits = [int32x4, int16x8, int8x16, uint32x4, uint16x8, uint8x16];
int32x4.fromBits = [float32x4, int16x8, int8x16, uint32x4, uint16x8, uint8x16];
int16x8.fromBits = [float32x4, int32x4, int8x16, uint32x4, uint16x8, uint8x16];
int8x16.fromBits = [float32x4, int32x4, int16x8, uint32x4, uint16x8, uint8x16];
uint32x4.fromBits = [float32x4, int32x4, int16x8, int8x16, uint16x8, uint8x16];
uint16x8.fromBits = [float32x4, int32x4, int16x8, int8x16, uint32x4, uint8x16];
uint8x16.fromBits = [float32x4, int32x4, int16x8, int8x16, uint32x4, uint16x8];

// SIMD widening types.
uint16x8.wideType = uint32x4;
uint8x16.wideType = uint16x8;

var allTypes = [float32x4,
                int32x4, int16x8, int8x16,
                uint32x4, uint16x8, uint8x16,
                bool32x4, bool16x8, bool8x16];

// SIMD prototype functions.
var prototypeFns = {
  valueOf:
    function(type) {
      return function() {
        throw new TypeError(type.name + " cannot be converted to a number");
      }
    },

  toString:
    function(type) {
      return function() {
        return simdToString(type, this);
      }
    },

  toLocaleString:
    function(type) {
      return function() {
        return simdToLocaleString(type, this);
      }
    },
};

// SIMD constructor functions.

var simdFns = {
  check:
    function(type) {
      return function(a) {
        if (!(a instanceof type.fn)) {
          throw new TypeError("Argument is not a " + type.name + ".");
        }
        return a;
      }
    },

  splat:
    function(type) {
      return function(s) { return simdSplat(type, s); }
    },

  replaceLane:
    function(type) {
      return function(a, i, s) { return simdReplaceLane(type, a, i, s); }
    },

  allTrue:
    function(type) {
      return function(a) { return simdAllTrue(type, a); }
    },

  anyTrue:
    function(type) {
      return function(a) { return simdAnyTrue(type, a); }
    },

  and:
    function(type) {
      return function(a, b) {
        return simdBinaryOp(type, binaryAnd, a, b);
      }
    },

  or:
    function(type) {
      return function(a, b) {
        return simdBinaryOp(type, binaryOr, a, b);
      }
    },

  xor:
    function(type) {
      return function(a, b) {
        return simdBinaryOp(type, binaryXor, a, b);
      }
    },

  not:
    function(type) {
      return function(a) {
        return simdUnaryOp(type, type.notFn, a);
      }
    },

  equal:
    function(type) {
      return function(a, b) {
        return simdRelationalOp(type, binaryEqual, a, b);
      }
    },

  notEqual:
    function(type) {
      return function(a, b) {
        return simdRelationalOp(type, binaryNotEqual, a, b);
      }
    },

  lessThan:
    function(type) {
      return function(a, b) {
        return simdRelationalOp(type, binaryLess, a, b);
      }
    },

  lessThanOrEqual:
    function(type) {
      return function(a, b) {
        return simdRelationalOp(type, binaryLessEqual, a, b);
      }
    },

  greaterThan:
    function(type) {
      return function(a, b) {
        return simdRelationalOp(type, binaryGreater, a, b);
      }
    },

  greaterThanOrEqual:
    function(type) {
      return function(a, b) {
        return simdRelationalOp(type, binaryGreaterEqual, a, b);
      }
    },

  add:
    function(type) {
      return function(a, b) {
        return simdBinaryOp(type, binaryAdd, a, b);
      }
    },

  sub:
    function(type) {
      return function(a, b) {
        return simdBinaryOp(type, binarySub, a, b);
      }
    },

  mul:
    function(type) {
      return function(a, b) {
        return simdBinaryOp(type, type.mulFn, a, b);
      }
    },

  div:
    function(type) {
      return function(a, b) {
        return simdBinaryOp(type, binaryDiv, a, b);
      }
    },

  neg:
    function(type) {
      return function(a) {
        return simdUnaryOp(type, unaryNeg, a);
      }
    },

  abs:
    function(type) {
      return function(a) {
        return simdUnaryOp(type, Math.abs, a);
      }
    },

  min:
    function(type) {
      return function(a, b) {
        return simdBinaryOp(type, Math.min, a, b);
      }
    },

  max:
    function(type) {
      return function(a, b) {
        return simdBinaryOp(type, Math.max, a, b);
      }
    },

  minNum:
    function(type) {
      return function(a, b) {
        return simdBinaryOp(type, minNum, a, b);
      }
    },

  maxNum:
    function(type) {
      return function(a, b) {
        return simdBinaryOp(type, maxNum, a, b);
      }
    },

  load:
    function(type) {
      return function(tarray, index) {
        return simdLoad(type, tarray, index, type.lanes);
      }
    },

  load1:
    function(type) {
      return function(tarray, index) {
        return simdLoad(type, tarray, index, 1);
      }
    },

  load2:
    function(type) {
      return function(tarray, index) {
        return simdLoad(type, tarray, index, 2);
      }
    },

  load3:
    function(type) {
      return function(tarray, index) {
        return simdLoad(type, tarray, index, 3);
      }
    },

  store:
    function(type) {
      return function(tarray, index, a) {
        return simdStore(type, tarray, index, a, type.lanes);
      }
    },

  store1:
    function(type) {
      return function(tarray, index, a) {
        return simdStore(type, tarray, index, a, 1);
      }
    },

  store2:
    function(type) {
      return function(tarray, index, a) {
        return simdStore(type, tarray, index, a, 2);
      }
    },

  store3:
    function(type) {
      return function(tarray, index, a) {
        return simdStore(type, tarray, index, a, 3);
      }
    },

  select:
    function(type) {
      return function(selector, a, b) {
        return simdSelect(type, selector, a, b);
      }
    },


  reciprocalApproximation:
    function(type) {
      return function(a) {
        a = type.fn.check(a);
        return type.fn.div(type.fn.splat(1.0), a);
      }
    },

  reciprocalSqrtApproximation:
    function(type) {
      return function(a) {
        a = type.fn.check(a);
        return type.fn.reciprocalApproximation(type.fn.sqrt(a));
      }
    },

  sqrt:
    function(type) {
      return function(a) {
        a = type.fn.check(a);
        return type.fn(Math.sqrt(type.fn.extractLane(a, 0)),
                       Math.sqrt(type.fn.extractLane(a, 1)),
                       Math.sqrt(type.fn.extractLane(a, 2)),
                       Math.sqrt(type.fn.extractLane(a, 3)));
      }
    },

  shiftLeftByScalar:
    function(type) {
      return function(a, bits) {
        if (bits>>>0 >= type.laneSize * 8)
          return type.fn.splat(0);
        return simdShiftOp(type, binaryShiftLeft, a, bits);
      }
    },

  shiftRightArithmeticByScalar:
    function(type) {
      return function(a, bits) {
        if (bits>>>0 >= type.laneSize * 8)
          bits = type.laneSize * 8 - 1;
        return simdShiftOp(type, binaryShiftRightArithmetic, a, bits);
      }
    },

  shiftRightLogicalByScalar:
    function(type) {
      return function(a, bits) {
        if (bits>>>0 >= type.laneSize * 8)
          return type.fn.splat(0);
        function shift(val, amount) {
          return val >>> amount;
        }
        return simdShiftOp(type, shift, a, bits);
      }
    },

  absoluteDifference:
    function(type) {
      return function(a, b) {
        return simdBinaryOp(type, binaryAbsDiff, a, b);
      }
    },

  horizontalSum:
    function(type) {
      return function(a) {
        return simdHorizontalSum(type, a);
      }
    },

  widenedAbsoluteDifference:
    function(type) {
      return function(a, b) {
        return simdWideningBinaryOp(type, binaryAbsDiff, a, b);
      }
    },

  addSaturate:
    function(type) {
      function addSaturate(a, b) {
        return clamp(a + b, type.minVal, type.maxVal);
      }
      return function(a, b) { return simdBinaryOp(type, addSaturate, a, b); }
    },

  subSaturate:
    function(type) {
      function subSaturate(a, b) {
        return clamp(a - b, type.minVal, type.maxVal);
      }
      return function(a, b) { return simdBinaryOp(type, subSaturate, a, b); }
    },
}

// Install functions.

allTypes.forEach(function(type) {
  // Install each prototype function on each SIMD prototype.
  var simdFn = type.fn;
  var proto = simdFn.prototype;
  for (var name in prototypeFns) {
    if (!proto.hasOwnProperty(name))
      proto[name] = prototypeFns[name](type);
  }
  // Install regular functions.
  type.fns.forEach(function(name) {
    if (typeof simdFn[name] === "undefined")
      simdFn[name] = simdFns[name](type);
  });
  // Install 'fromTIMD' functions.
  if (type.from) {
    type.from.forEach(function(fromType) {
      var name = "from" + fromType.name;
      var toType = type;  // pull type into closure.
      if (typeof type.fn[name] === "undefined") {
        type.fn[name] =
            function(a) { return simdFrom(toType, fromType, a); }
      }
    });
  }
  // Install 'fromTIMDBits' functions.
  if (type.fromBits) {
    type.fromBits.forEach(function(fromType) {
      var name = "from" + fromType.name + "Bits";
      var toType = type;  // pull type into closure.
      if (typeof type.fn[name] === "undefined") {
        type.fn[name] =
            function(a) { return simdFromBits(toType, fromType, a); }
      }
    });
  }
});

// Miscellaneous functions that aren't easily parameterized on type.

if (typeof SIMD.Float32x4.swizzle === "undefined") {
  SIMD.Float32x4.swizzle = function(a, s0, s1, s2, s3) {
    return simdSwizzle(float32x4, a, [s0, s1, s2, s3]);
  }
}

if (typeof SIMD.Float32x4.shuffle === "undefined") {
  SIMD.Float32x4.shuffle = function(a, b, s0, s1, s2, s3) {
    return simdShuffle(float32x4, a, b, [s0, s1, s2, s3]);
  }
}

if (typeof SIMD.Int32x4.swizzle === "undefined") {
  SIMD.Int32x4.swizzle = function(a, s0, s1, s2, s3) {
    return simdSwizzle(int32x4, a, [s0, s1, s2, s3]);
  }
}

if (typeof SIMD.Int32x4.shuffle === "undefined") {
  SIMD.Int32x4.shuffle = function(a, b, s0, s1, s2, s3) {
    return simdShuffle(int32x4, a, b, [s0, s1, s2, s3]);
  }
}

if (typeof SIMD.Uint32x4.swizzle === "undefined") {
  SIMD.Uint32x4.swizzle = function(a, s0, s1, s2, s3) {
    return simdSwizzle(uint32x4, a, [s0, s1, s2, s3]);
  }
}

if (typeof SIMD.Uint32x4.shuffle === "undefined") {
  SIMD.Uint32x4.shuffle = function(a, b, s0, s1, s2, s3) {
    return simdShuffle(uint32x4, a, b, [s0, s1, s2, s3]);
  }
}

if (typeof SIMD.Int16x8.swizzle === "undefined") {
  SIMD.Int16x8.swizzle = function(a, s0, s1, s2, s3, s4, s5, s6, s7) {
    return simdSwizzle(int16x8, a, [s0, s1, s2, s3, s4, s5, s6, s7]);
  }
}

if (typeof SIMD.Int16x8.shuffle === "undefined") {
  SIMD.Int16x8.shuffle = function(a, b, s0, s1, s2, s3, s4, s5, s6, s7) {
    return simdShuffle(int16x8, a, b, [s0, s1, s2, s3, s4, s5, s6, s7]);
  }
}

if (typeof SIMD.Uint16x8.swizzle === "undefined") {
  SIMD.Uint16x8.swizzle = function(a, s0, s1, s2, s3, s4, s5, s6, s7) {
    return simdSwizzle(uint16x8, a, [s0, s1, s2, s3, s4, s5, s6, s7]);
  }
}

if (typeof SIMD.Uint16x8.shuffle === "undefined") {
  SIMD.Uint16x8.shuffle = function(a, b, s0, s1, s2, s3, s4, s5, s6, s7) {
    return simdShuffle(uint16x8, a, b, [s0, s1, s2, s3, s4, s5, s6, s7]);
  }
}

if (typeof SIMD.Int8x16.swizzle === "undefined") {
  SIMD.Int8x16.swizzle = function(a, s0, s1, s2, s3, s4, s5, s6, s7,
                                     s8, s9, s10, s11, s12, s13, s14, s15) {
    return simdSwizzle(int8x16, a, [s0, s1, s2, s3, s4, s5, s6, s7,
                                    s8, s9, s10, s11, s12, s13, s14, s15]);
  }
}

if (typeof SIMD.Int8x16.shuffle === "undefined") {
  SIMD.Int8x16.shuffle = function(a, b, s0, s1, s2, s3, s4, s5, s6, s7,
                                        s8, s9, s10, s11, s12, s13, s14, s15) {
    return simdShuffle(int8x16, a, b, [s0, s1, s2, s3, s4, s5, s6, s7,
                                       s8, s9, s10, s11, s12, s13, s14, s15]);
  }
}

if (typeof SIMD.Uint8x16.swizzle === "undefined") {
  SIMD.Uint8x16.swizzle = function(a, s0, s1, s2, s3, s4, s5, s6, s7,
                                      s8, s9, s10, s11, s12, s13, s14, s15) {
    return simdSwizzle(uint8x16, a, [s0, s1, s2, s3, s4, s5, s6, s7,
                                     s8, s9, s10, s11, s12, s13, s14, s15]);
  }
}

if (typeof SIMD.Uint8x16.shuffle === "undefined") {
  SIMD.Uint8x16.shuffle = function(a, b, s0, s1, s2, s3, s4, s5, s6, s7,
                                         s8, s9, s10, s11, s12, s13, s14, s15) {
    return simdShuffle(uint8x16, a, b, [s0, s1, s2, s3, s4, s5, s6, s7,
                                        s8, s9, s10, s11, s12, s13, s14, s15]);
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
