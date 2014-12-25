/*
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

if (typeof SIMD === "undefined") {
  // SIMD module. We don't use the var keyword here, so that we put the
  // SIMD object in the global scope even if this polyfill code is included
  // within some other scope. The theory is that we're anticipating a
  // future where SIMD is predefined in the global scope.
  SIMD = {};
}

// private stuff.
var _SIMD_PRIVATE = {};

// Temporary buffers for swizzles and bitcasts.
_SIMD_PRIVATE._f32x4 = new Float32Array(4);
_SIMD_PRIVATE._f64x2 = new Float64Array(_SIMD_PRIVATE._f32x4.buffer);
_SIMD_PRIVATE._i32x4 = new Int32Array(_SIMD_PRIVATE._f32x4.buffer);
_SIMD_PRIVATE._i16x8 = new Int16Array(_SIMD_PRIVATE._f32x4.buffer);
_SIMD_PRIVATE._i8x16 = new Int8Array(_SIMD_PRIVATE._f32x4.buffer);

if (typeof Math.fround !== 'undefined') {
  _SIMD_PRIVATE.truncatef32 = Math.fround;
} else {
  _SIMD_PRIVATE._f32 = new Float32Array(1);

  _SIMD_PRIVATE.truncatef32 = function(x) {
    _SIMD_PRIVATE._f32[0] = x;
    return _SIMD_PRIVATE._f32[0];
  }
}

_SIMD_PRIVATE.minNum = function(x, y) {
  return x != x ? y :
         y != y ? x :
         Math.min(x, y);
}

_SIMD_PRIVATE.maxNum = function(x, y) {
  return x != x ? y :
         y != y ? x :
         Math.max(x, y);
}

// Type checking functions.

_SIMD_PRIVATE.isTypedArray = function(o) {
  return (o instanceof Int8Array) ||
         (o instanceof Uint8Array) ||
         (o instanceof Uint8ClampedArray) ||
         (o instanceof Int16Array) ||
         (o instanceof Uint16Array) ||
         (o instanceof Int32Array) ||
         (o instanceof Uint32Array) ||
         (o instanceof Float32Array) ||
         (o instanceof Float64Array) ||
         (o instanceof Int32x4Array) ||
         (o instanceof Float32x4Array) ||
         (o instanceof Float64x2Array);
}

_SIMD_PRIVATE.isNumber = function(o) {
  return typeof o == "number" || (typeof o == "object" && o.constructor === Number);
}

// Save/Restore utilities for implementing bitwise conversions.

_SIMD_PRIVATE.saveFloat64x2 = function(x) {
  x = SIMD.float64x2(x);
  _SIMD_PRIVATE._f64x2[0] = x.x;
  _SIMD_PRIVATE._f64x2[1] = x.y;
}

_SIMD_PRIVATE.saveFloat32x4 = function(x) {
  x = SIMD.float32x4(x);
  _SIMD_PRIVATE._f32x4[0] = x.x;
  _SIMD_PRIVATE._f32x4[1] = x.y;
  _SIMD_PRIVATE._f32x4[2] = x.z;
  _SIMD_PRIVATE._f32x4[3] = x.w;
}

_SIMD_PRIVATE.saveInt32x4 = function(x) {
  x = SIMD.int32x4(x);
  _SIMD_PRIVATE._i32x4[0] = x.x;
  _SIMD_PRIVATE._i32x4[1] = x.y;
  _SIMD_PRIVATE._i32x4[2] = x.z;
  _SIMD_PRIVATE._i32x4[3] = x.w;
}

_SIMD_PRIVATE.saveInt16x8 = function(x) {
  x = SIMD.int16x8(x);
  _SIMD_PRIVATE._i16x8[0] = x.s0;
  _SIMD_PRIVATE._i16x8[1] = x.s1;
  _SIMD_PRIVATE._i16x8[2] = x.s2;
  _SIMD_PRIVATE._i16x8[3] = x.s3;
  _SIMD_PRIVATE._i16x8[4] = x.s4;
  _SIMD_PRIVATE._i16x8[5] = x.s5;
  _SIMD_PRIVATE._i16x8[6] = x.s6;
  _SIMD_PRIVATE._i16x8[7] = x.s7;
}

_SIMD_PRIVATE.saveInt8x16 = function(x) {
  x = SIMD.int8x16(x);
  _SIMD_PRIVATE._i8x16[0] = x.s0;
  _SIMD_PRIVATE._i8x16[1] = x.s1;
  _SIMD_PRIVATE._i8x16[2] = x.s2;
  _SIMD_PRIVATE._i8x16[3] = x.s3;
  _SIMD_PRIVATE._i8x16[4] = x.s4;
  _SIMD_PRIVATE._i8x16[5] = x.s5;
  _SIMD_PRIVATE._i8x16[6] = x.s6;
  _SIMD_PRIVATE._i8x16[7] = x.s7;
  _SIMD_PRIVATE._i8x16[8] = x.s8;
  _SIMD_PRIVATE._i8x16[9] = x.s9;
  _SIMD_PRIVATE._i8x16[10] = x.s10;
  _SIMD_PRIVATE._i8x16[11] = x.s11;
  _SIMD_PRIVATE._i8x16[12] = x.s12;
  _SIMD_PRIVATE._i8x16[13] = x.s13;
  _SIMD_PRIVATE._i8x16[14] = x.s14;
  _SIMD_PRIVATE._i8x16[15] = x.s15;
}

_SIMD_PRIVATE.restoreFloat64x2 = function() {
  var alias = _SIMD_PRIVATE._f64x2;
  return SIMD.float64x2(alias[0], alias[1]);
}

_SIMD_PRIVATE.restoreFloat32x4 = function() {
  var alias = _SIMD_PRIVATE._f32x4;
  return SIMD.float32x4(alias[0], alias[1], alias[2], alias[3]);
}

_SIMD_PRIVATE.restoreInt32x4 = function() {
  var alias = _SIMD_PRIVATE._i32x4;
  return SIMD.int32x4(alias[0], alias[1], alias[2], alias[3]);
}

_SIMD_PRIVATE.restoreInt16x8 = function() {
  var alias = _SIMD_PRIVATE._i16x8;
  return SIMD.int16x8(alias[0], alias[1], alias[2], alias[3],
                      alias[4], alias[5], alias[6], alias[7]);
}

_SIMD_PRIVATE.restoreInt8x16 = function() {
  var alias = _SIMD_PRIVATE._i8x16;
  return SIMD.int8x16(alias[0], alias[1], alias[2], alias[3],
                      alias[4], alias[5], alias[6], alias[7],
                      alias[8], alias[9], alias[10], alias[11],
                      alias[12], alias[13], alias[14], alias[15]);
}

if (typeof SIMD.float32x4 === "undefined") {
  /**
    * Construct a new instance of float32x4 number.
    * @param {double} value used for x lane.
    * @param {double} value used for y lane.
    * @param {double} value used for z lane.
    * @param {double} value used for w lane.
    * @constructor
    */
  SIMD.float32x4 = function(x, y, z, w) {
    if (arguments.length == 1) {
      if (!(x instanceof SIMD.float32x4))
        throw new TypeError("argument is not a float32x4.");
      return x;
    }

    if (!(this instanceof SIMD.float32x4)) {
      return new SIMD.float32x4(x, y, z, w);
    }

    this.x_ = _SIMD_PRIVATE.truncatef32(x);
    this.y_ = _SIMD_PRIVATE.truncatef32(y);
    this.z_ = _SIMD_PRIVATE.truncatef32(z);
    this.w_ = _SIMD_PRIVATE.truncatef32(w);
  }

  Object.defineProperty(SIMD.float32x4.prototype, 'x', {
    get: function() { return this.x_; }
  });

  Object.defineProperty(SIMD.float32x4.prototype, 'y', {
    get: function() { return this.y_; }
  });

  Object.defineProperty(SIMD.float32x4.prototype, 'z', {
    get: function() { return this.z_; }
  });

  Object.defineProperty(SIMD.float32x4.prototype, 'w', {
    get: function() { return this.w_; }
  });

  /**
    * Extract the sign bit from each lane return them in the first 4 bits.
    */
  Object.defineProperty(SIMD.float32x4.prototype, 'signMaskPolyfill', {
    get: function() {
      var mx = (this.x < 0.0 || 1/this.x === -Infinity) ? 1 : 0;
      var my = (this.y < 0.0 || 1/this.y === -Infinity) ? 1 : 0;
      var mz = (this.z < 0.0 || 1/this.z === -Infinity) ? 1 : 0;
      var mw = (this.w < 0.0 || 1/this.w === -Infinity) ? 1 : 0;
      return mx | my << 1 | mz << 2 | mw << 3;
    }
  });
}

if (typeof SIMD.float32x4.zero === "undefined") {
  /**
    * Construct a new instance of float32x4 number with 0.0 in all lanes.
    * @constructor
    */
  SIMD.float32x4.zero = function() {
    return SIMD.float32x4(0.0, 0.0, 0.0, 0.0);
  }
}

if (typeof SIMD.float32x4.splat === "undefined") {
  /**
    * Construct a new instance of float32x4 number with the same value
    * in all lanes.
    * @param {double} value used for all lanes.
    * @constructor
    */
  SIMD.float32x4.splat = function(s) {
    return SIMD.float32x4(s, s, s, s);
  }
}

if (typeof SIMD.float32x4.fromFloat64x2 === "undefined") {
  /**
    * @param {float64x2} t An instance of float64x2.
    * @return {float32x4} A float32x4 with .x and .y from t
    */
  SIMD.float32x4.fromFloat64x2 = function(t) {
    t = SIMD.float64x2(t);
    return SIMD.float32x4(t.x, t.y, 0, 0);
  }
}

if (typeof SIMD.float32x4.fromInt32x4 === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @return {float32x4} An integer to float conversion copy of t.
    */
  SIMD.float32x4.fromInt32x4 = function(t) {
    t = SIMD.int32x4(t);
    return SIMD.float32x4(t.x, t.y, t.z, t.w);
  }
}

if (typeof SIMD.float32x4.fromUnsignedInt32x4 === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @return {float32x4} An unsigned integer to float conversion copy of t.
    */
  SIMD.float32x4.fromUnsignedInt32x4 = function(t) {
    t = SIMD.int32x4(t);
    return SIMD.float32x4(t.x>>>0, t.y>>>0, t.z>>>0, t.w>>>0);
  }
}

if (typeof SIMD.float32x4.fromFloat64x2Bits === "undefined") {
  /**
   * @param {float64x2} t An instance of float64x2.
   * @return {float32x4} a bit-wise copy of t as a float32x4.
   */
  SIMD.float32x4.fromFloat64x2Bits = function(t) {
    _SIMD_PRIVATE.saveFloat64x2(t);
    return _SIMD_PRIVATE.restoreFloat32x4();
  }
}

if (typeof SIMD.float32x4.fromInt32x4Bits === "undefined") {
  /**
   * @param {int32x4} t An instance of int32x4.
   * @return {float32x4} a bit-wise copy of t as a float32x4.
   */
  SIMD.float32x4.fromInt32x4Bits = function(t) {
    _SIMD_PRIVATE.saveInt32x4(t);
    return _SIMD_PRIVATE.restoreFloat32x4();
  }
}

if (typeof SIMD.float32x4.fromInt16x8Bits === "undefined") {
  /**
   * @param {int16x8} t An instance of int16x8.
   * @return {float32x4} a bit-wise copy of t as a float32x4.
   */
  SIMD.float32x4.fromInt16x8Bits = function(t) {
    _SIMD_PRIVATE.saveInt16x8(t);
    return _SIMD_PRIVATE.restoreFloat32x4();
  }
}

if (typeof SIMD.float32x4.fromInt8x16Bits === "undefined") {
  /**
   * @param {int8x16} t An instance of int8x16.
   * @return {float32x4} a bit-wise copy of t as a float32x4.
   */
  SIMD.float32x4.fromInt8x16Bits = function(t) {
    _SIMD_PRIVATE.saveInt8x16(t);
    return _SIMD_PRIVATE.restoreFloat32x4();
  }
}

if (typeof SIMD.float64x2 === "undefined") {
  /**
    * Construct a new instance of float64x2 number.
    * @param {double} value used for x lane.
    * @param {double} value used for y lane.
    * @constructor
    */
  SIMD.float64x2 = function(x, y) {
    if (arguments.length == 1) {
      if (!(x instanceof SIMD.float64x2))
        throw new TypeError("argument is not a float64x2.");
      return x;
    }

    if (!(this instanceof SIMD.float64x2)) {
      return new SIMD.float64x2(x, y);
    }

    // Use unary + to force coersion to Number.
    this.x_ = +x;
    this.y_ = +y;
  }

  Object.defineProperty(SIMD.float64x2.prototype, 'x', {
    get: function() { return this.x_; }
  });

  Object.defineProperty(SIMD.float64x2.prototype, 'y', {
    get: function() { return this.y_; }
  });

  /**
    * Extract the sign bit from each lane return them in the first 2 bits.
    */
  Object.defineProperty(SIMD.float64x2.prototype, 'signMaskPolyfill', {
    get: function() {
      var mx = (this.x < 0.0 || 1/this.x === -Infinity) ? 1 : 0;
      var my = (this.y < 0.0 || 1/this.y === -Infinity) ? 1 : 0;
      return mx | my << 1;
    }
  });
}

if (typeof SIMD.float64x2.zero === "undefined") {
  /**
    * Construct a new instance of float64x2 number with 0.0 in all lanes.
    * @constructor
    */
  SIMD.float64x2.zero = function() {
    return SIMD.float64x2(0.0, 0.0);
  }
}

if (typeof SIMD.float64x2.splat === "undefined") {
  /**
    * Construct a new instance of float64x2 number with the same value
    * in all lanes.
    * @param {double} value used for all lanes.
    * @constructor
    */
  SIMD.float64x2.splat = function(s) {
    return SIMD.float64x2(s, s);
  }
}

if (typeof SIMD.float64x2.fromFloat32x4 === "undefined") {
  /**
    * @param {float32x4} t An instance of float32x4.
    * @return {float64x2} A float64x2 with .x and .y from t
    */
  SIMD.float64x2.fromFloat32x4 = function(t) {
    t = SIMD.float32x4(t);
    return SIMD.float64x2(t.x, t.y);
  }
}

if (typeof SIMD.float64x2.fromInt32x4 === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @return {float64x2} A float64x2 with .x and .y from t
    */
  SIMD.float64x2.fromInt32x4 = function(t) {
    t = SIMD.int32x4(t);
    return SIMD.float64x2(t.x, t.y);
  }
}

if (typeof SIMD.float64x2.fromUnsignedInt32x4 === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @return {float64x2} A float64x2 with .x>>>0 and .y>>>0 from t
    */
  SIMD.float64x2.fromUnsignedInt32x4 = function(t) {
    t = SIMD.int32x4(t);
    return SIMD.float64x2(t.x>>>0, t.y>>>0);
  }
}

if (typeof SIMD.float64x2.fromFloat32x4Bits === "undefined") {
  /**
   * @param {float32x4} t An instance of float32x4.
   * @return {float64x2} a bit-wise copy of t as a float64x2.
   */
  SIMD.float64x2.fromFloat32x4Bits = function(t) {
    _SIMD_PRIVATE.saveFloat32x4(t);
    return _SIMD_PRIVATE.restoreFloat64x2();
  }
}

if (typeof SIMD.float64x2.fromInt32x4Bits === "undefined") {
  /**
   * @param {int32x4} t An instance of int32x4.
   * @return {float64x2} a bit-wise copy of t as a float64x2.
   */
  SIMD.float64x2.fromInt32x4Bits = function(t) {
    _SIMD_PRIVATE.saveInt32x4(t);
    return _SIMD_PRIVATE.restoreFloat64x2();
  }
}

if (typeof SIMD.float64x2.fromInt16x8Bits === "undefined") {
  /**
   * @param {int16x8} t An instance of int16x8.
   * @return {float64x2} a bit-wise copy of t as a float64x2.
   */
  SIMD.float64x2.fromInt16x8Bits = function(t) {
    _SIMD_PRIVATE.saveInt16x8(t);
    return _SIMD_PRIVATE.restoreFloat64x2();
  }
}

if (typeof SIMD.float64x2.fromInt8x16Bits === "undefined") {
  /**
   * @param {int8x16} t An instance of int8x16.
   * @return {float64x2} a bit-wise copy of t as a float64x2.
   */
  SIMD.float64x2.fromInt8x16Bits = function(t) {
    _SIMD_PRIVATE.saveInt8x16(t);
    return _SIMD_PRIVATE.restoreFloat64x2();
  }
}

if (typeof SIMD.int32x4 === "undefined") {
  /**
    * Construct a new instance of int32x4 number.
    * @param {integer} 32-bit value used for x lane.
    * @param {integer} 32-bit value used for y lane.
    * @param {integer} 32-bit value used for z lane.
    * @param {integer} 32-bit value used for w lane.
    * @constructor
    */
  SIMD.int32x4 = function(x, y, z, w) {
    if (arguments.length == 1) {
      if (!(x instanceof SIMD.int32x4))
        throw new TypeError("argument is not a int32x4.");
      return x;
    }

    if (!(this instanceof SIMD.int32x4)) {
      return new SIMD.int32x4(x, y, z, w);
    }

    this.x_ = x|0;
    this.y_ = y|0;
    this.z_ = z|0;
    this.w_ = w|0;
  }

  Object.defineProperty(SIMD.int32x4.prototype, 'x', {
    get: function() { return this.x_; }
  });

  Object.defineProperty(SIMD.int32x4.prototype, 'y', {
    get: function() { return this.y_; }
  });

  Object.defineProperty(SIMD.int32x4.prototype, 'z', {
    get: function() { return this.z_; }
  });

  Object.defineProperty(SIMD.int32x4.prototype, 'w', {
    get: function() { return this.w_; }
  });

  Object.defineProperty(SIMD.int32x4.prototype, 'flagX', {
    get: function() { return this.x != 0x0; }
  });

  Object.defineProperty(SIMD.int32x4.prototype, 'flagY', {
    get: function() { return this.y != 0x0; }
  });

  Object.defineProperty(SIMD.int32x4.prototype, 'flagZ', {
    get: function() { return this.z != 0x0; }
  });

  Object.defineProperty(SIMD.int32x4.prototype, 'flagW', {
    get: function() { return this.w != 0x0; }
  });

  /**
    * Extract the sign bit from each lane return them in the first 4 bits.
    */
  Object.defineProperty(SIMD.int32x4.prototype, 'signMaskPolyfill', {
    get: function() {
      var mx = (this.x & 0x80000000) >>> 31;
      var my = (this.y & 0x80000000) >>> 31;
      var mz = (this.z & 0x80000000) >>> 31;
      var mw = (this.w & 0x80000000) >>> 31;
      return mx | my << 1 | mz << 2 | mw << 3;
    }
  });
}

if (typeof SIMD.int32x4.zero === "undefined") {
  /**
    * Construct a new instance of int32x4 number with 0 in all lanes.
    * @constructor
    */
  SIMD.int32x4.zero = function() {
    return SIMD.int32x4(0, 0, 0, 0);
  }
}

if (typeof SIMD.int32x4.bool === "undefined") {
  /**
    * Construct a new instance of int32x4 number with 0xFFFFFFFF or 0x0 in each
    * lane, depending on the truth value in x, y, z, and w.
    * @param {boolean} flag used for x lane.
    * @param {boolean} flag used for y lane.
    * @param {boolean} flag used for z lane.
    * @param {boolean} flag used for w lane.
    * @constructor
    */
  SIMD.int32x4.bool = function(x, y, z, w) {
    return SIMD.int32x4(x ? -1 : 0x0,
                        y ? -1 : 0x0,
                        z ? -1 : 0x0,
                        w ? -1 : 0x0);
  }
}

if (typeof SIMD.int32x4.splat === "undefined") {
  /**
    * Construct a new instance of int32x4 number with the same value
    * in all lanes.
    * @param {integer} value used for all lanes.
    * @constructor
    */
  SIMD.int32x4.splat = function(s) {
    return SIMD.int32x4(s, s, s, s);
  }
}

if (typeof SIMD.int32x4.fromFloat32x4 === "undefined") {
  /**
    * @param {float32x4} t An instance of float32x4.
    * @return {int32x4} with a integer to float conversion of t.
    */
  SIMD.int32x4.fromFloat32x4 = function(t) {
    t = SIMD.float32x4(t);
    return SIMD.int32x4(t.x, t.y, t.z, t.w);
  }
}

if (typeof SIMD.int32x4.fromFloat32x4ToUnsigned === "undefined") {
  /**
    * @param {float32x4} t An instance of float32x4.
    * @return {int32x4} with an unsigned integer to float conversion of t.
    */
  SIMD.int32x4.fromFloat32x4ToUnsigned = function(t) {
    t = SIMD.float32x4(t);
    return SIMD.int32x4(t.x>>>0, t.y>>>0, t.z>>>0, t.w>>>0);
  }
}

if (typeof SIMD.int32x4.fromFloat64x2 === "undefined") {
  /**
    * @param {float64x2} t An instance of float64x2.
    * @return {int32x4}  An int32x4 with .x and .y from t
    */
  SIMD.int32x4.fromFloat64x2 = function(t) {
    t = SIMD.float64x2(t);
    return SIMD.int32x4(t.x, t.y, 0, 0);
  }
}

if (typeof SIMD.int32x4.fromFloat64x2ToUnsigned === "undefined") {
  /**
    * @param {float64x2} t An instance of float64x2.
    * @return {int32x4}  An int32x4 with .x>>>0 and .y>>>0 from t
    */
  SIMD.int32x4.fromFloat64x2ToUnsigned = function(t) {
    t = SIMD.float64x2(t);
    return SIMD.int32x4(t.x>>>0, t.y>>>0, 0, 0);
  }
}

if (typeof SIMD.int32x4.fromInt16x8 === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @return {int32x4} with the s0, s1, s2, and s3 from t, sign-extended
    */
  SIMD.int32x4.fromInt16x8 = function(t) {
    t = SIMD.int16x8(t);
    return SIMD.int32x4(t.s0, t.s1, t.s2, t.s3);
  }
}

if (typeof SIMD.int32x4.fromUnsignedInt8x16 === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @return {int32x4} with the s0, s1, s2, and s3 from t, zero-extended
    */
  SIMD.int32x4.fromUnsignedInt8x16 = function(t) {
    t = SIMD.int8x16(t);
    return SIMD.int32x4(t.s0>>>0, t.s1>>>0, t.s2>>>0, t.s3>>>0);
  }
}

if (typeof SIMD.int32x4.fromUnsignedInt16x8 === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @return {int32x4} with the s0, s1, s2, and s3 from t, zero-extended
    */
  SIMD.int32x4.fromUnsignedInt16x8 = function(t) {
    t = SIMD.int16x8(t);
    return SIMD.int32x4(t.s0>>>0, t.s1>>>0, t.s2>>>0, t.s3>>>0);
  }
}

if (typeof SIMD.int32x4.fromInt8x16 === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @return {int32x4} with the s0, s1, s2, and s3 from t
    */
  SIMD.int32x4.fromInt8x16 = function(t) {
    t = SIMD.int8x16(t);
    return SIMD.int32x4(t.s0, t.s1, t.s2, t.s3);
  }
}

if (typeof SIMD.int32x4.fromFloat32x4Bits === "undefined") {
  /**
    * @param {float32x4} t An instance of float32x4.
    * @return {int32x4} a bit-wise copy of t as a int32x4.
    */
  SIMD.int32x4.fromFloat32x4Bits = function(t) {
    _SIMD_PRIVATE.saveFloat32x4(t);
    return _SIMD_PRIVATE.restoreInt32x4();
  }
}

if (typeof SIMD.int32x4.fromFloat64x2Bits === "undefined") {
  /**
   * @param {float64x2} t An instance of float64x2.
   * @return {int32x4} a bit-wise copy of t as an int32x4.
   */
  SIMD.int32x4.fromFloat64x2Bits = function(t) {
    _SIMD_PRIVATE.saveFloat64x2(t);
    return _SIMD_PRIVATE.restoreInt32x4();
  }
}

if (typeof SIMD.int32x4.fromInt16x8Bits === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @return {int32x4} a bit-wise copy of t as a int32x4.
    */
  SIMD.int32x4.fromInt16x8Bits = function(t) {
    _SIMD_PRIVATE.saveInt16x8(t);
    return _SIMD_PRIVATE.restoreInt32x4();
  }
}

if (typeof SIMD.int32x4.fromInt8x16Bits === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @return {int32x4} a bit-wise copy of t as a int32x4.
    */
  SIMD.int32x4.fromInt8x16Bits = function(t) {
    _SIMD_PRIVATE.saveInt8x16(t);
    return _SIMD_PRIVATE.restoreInt32x4();
  }
}

if (typeof SIMD.int16x8 === "undefined") {
  /**
    * Construct a new instance of int16x8 number.
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
  SIMD.int16x8 = function(s0, s1, s2, s3, s4, s5, s6, s7) {
    if (arguments.length == 1) {
      if (!(x instanceof SIMD.int16x8))
        throw new TypeError("argument is not a int16x8.");
      return x;
    }

    if (!(this instanceof SIMD.int16x8)) {
      return new SIMD.int16x8(s0, s1, s2, s3, s4, s5, s6, s7);
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

  Object.defineProperty(SIMD.int16x8.prototype, 's0', {
    get: function() { return this.s0_; }
  });

  Object.defineProperty(SIMD.int16x8.prototype, 's1', {
    get: function() { return this.s1_; }
  });

  Object.defineProperty(SIMD.int16x8.prototype, 's2', {
    get: function() { return this.s2_; }
  });

  Object.defineProperty(SIMD.int16x8.prototype, 's3', {
    get: function() { return this.s3_; }
  });

  Object.defineProperty(SIMD.int16x8.prototype, 's4', {
    get: function() { return this.s4_; }
  });

  Object.defineProperty(SIMD.int16x8.prototype, 's5', {
    get: function() { return this.s5_; }
  });

  Object.defineProperty(SIMD.int16x8.prototype, 's6', {
    get: function() { return this.s6_; }
  });

  Object.defineProperty(SIMD.int16x8.prototype, 's7', {
    get: function() { return this.s7_; }
  });

  /**
    * Extract the sign bit from each lane return them in the first 8 bits.
    */
  Object.defineProperty(SIMD.int16x8.prototype, 'signMaskPolyfill', {
    get: function() {
      var ms0 = (this.s0 & 0x8000) >>> 15;
      var ms1 = (this.s1 & 0x8000) >>> 15;
      var ms2 = (this.s2 & 0x8000) >>> 15;
      var ms3 = (this.s3 & 0x8000) >>> 15;
      var ms4 = (this.s4 & 0x8000) >>> 15;
      var ms5 = (this.s5 & 0x8000) >>> 15;
      var ms6 = (this.s6 & 0x8000) >>> 15;
      var ms7 = (this.s7 & 0x8000) >>> 15;
      return ms0 | ms1 << 1 | ms2 << 2 | ms3 << 3 |
             ms4 << 4 | ms5 << 5 | ms6 << 6 | ms7 << 7;
    }
  });
}

if (typeof SIMD.int16x8.zero === "undefined") {
  /**
    * Construct a new instance of int16x8 number with 0 in all lanes.
    * @constructor
    */
  SIMD.int16x8.zero = function() {
    return SIMD.int16x8(0, 0, 0, 0, 0, 0, 0, 0);
  }
}

if (typeof SIMD.int16x8.bool === "undefined") {
  /**
    * Construct a new instance of int16x8 number with 0xFFFF or 0x0 in each
    * lane, depending on the truth value in s0, s1, s2, s3, s4, s5, s6, and s7.
    * @param {boolean} flag used for s0 lane.
    * @param {boolean} flag used for s1 lane.
    * @param {boolean} flag used for s2 lane.
    * @param {boolean} flag used for s3 lane.
    * @param {boolean} flag used for s4 lane.
    * @param {boolean} flag used for s5 lane.
    * @param {boolean} flag used for s6 lane.
    * @param {boolean} flag used for s7 lane.
    * @constructor
    */
  SIMD.int16x8.bool = function(s0, s1, s2, s3, s4, s5, s6, s7) {
    return SIMD.int16x8(s0 ? -1 : 0x0,
                        s1 ? -1 : 0x0,
                        s2 ? -1 : 0x0,
                        s3 ? -1 : 0x0,
                        s4 ? -1 : 0x0,
                        s5 ? -1 : 0x0,
                        s6 ? -1 : 0x0,
                        s7 ? -1 : 0x0);
  }
}

if (typeof SIMD.int16x8.splat === "undefined") {
  /**
    * Construct a new instance of int16x8 number with the same value
    * in all lanes.
    * @param {integer} value used for all lanes.
    * @constructor
    */
  SIMD.int16x8.splat = function(s) {
    return SIMD.int16x8(s, s, s, s, s, s, s, s);
  }
}

if (typeof SIMD.int16x8.fromFloat32x4Bits === "undefined") {
  /**
    * @param {float32x4} t An instance of float32x4.
    * @return {int16x8} a bit-wise copy of t as a int16x8.
    */
  SIMD.int16x8.fromFloat32x4Bits = function(t) {
    _SIMD_PRIVATE.saveFloat32x4(t);
    return _SIMD_PRIVATE.restoreInt16x8();
  }
}

if (typeof SIMD.int16x8.fromFloat64x2Bits === "undefined") {
  /**
   * @param {float64x2} t An instance of float64x2.
   * @return {int16x8} a bit-wise copy of t as an int16x8.
   */
  SIMD.int16x8.fromFloat64x2Bits = function(t) {
    _SIMD_PRIVATE.saveFloat64x2(t);
    return _SIMD_PRIVATE.restoreInt16x8();
  }
}

if (typeof SIMD.int16x8.fromInt32x4Bits === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @return {int16x8} a bit-wise copy of t as a int16x8.
    */
  SIMD.int16x8.fromInt32x4Bits = function(t) {
    _SIMD_PRIVATE.saveInt32x4(t);
    return _SIMD_PRIVATE.restoreInt16x8();
  }
}

if (typeof SIMD.int16x8.fromInt8x16Bits === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @return {int16x8} a bit-wise copy of t as a int16x8.
    */
  SIMD.int16x8.fromInt8x16Bits = function(t) {
    saveInt8x16(t);
    return restoreInt16x8();
  }
}

if (typeof SIMD.int8x16 === "undefined") {
  /**
    * Construct a new instance of int8x16 number.
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
  SIMD.int8x16 = function(s0, s1, s2, s3, s4, s5, s6, s7,
                          s8, s9, s10, s11, s12, s13, s14, s15) {
    if (arguments.length == 1) {
      if (!(x instanceof SIMD.int8x16))
        throw new TypeError("argument is not a int8x16.");
      return x;
    }

    if (!(this instanceof SIMD.int8x16)) {
      return new SIMD.int8x16(s0, s1, s2, s3, s4, s5, s6, s7,
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

  Object.defineProperty(SIMD.int8x16.prototype, 's0', {
    get: function() { return this.s0_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's1', {
    get: function() { return this.s1_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's2', {
    get: function() { return this.s2_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's3', {
    get: function() { return this.s3_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's4', {
    get: function() { return this.s4_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's5', {
    get: function() { return this.s5_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's6', {
    get: function() { return this.s6_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's7', {
    get: function() { return this.s7_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's8', {
    get: function() { return this.s8_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's9', {
    get: function() { return this.s9_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's10', {
    get: function() { return this.s10_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's11', {
    get: function() { return this.s11_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's12', {
    get: function() { return this.s12_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's13', {
    get: function() { return this.s13_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's14', {
    get: function() { return this.s14_; }
  });

  Object.defineProperty(SIMD.int8x16.prototype, 's15', {
    get: function() { return this.s15_; }
  });

  /**
    * Extract the sign bit from each lane return them in the first 16 bits.
    */
  Object.defineProperty(SIMD.int8x16.prototype, 'signMaskPolyfill', {
    get: function() {
      var ms0 = (this.s0 & 0x80) >>> 7;
      var ms1 = (this.s1 & 0x80) >>> 7;
      var ms2 = (this.s2 & 0x80) >>> 7;
      var ms3 = (this.s3 & 0x80) >>> 7;
      var ms4 = (this.s4 & 0x80) >>> 7;
      var ms5 = (this.s5 & 0x80) >>> 7;
      var ms6 = (this.s6 & 0x80) >>> 7;
      var ms7 = (this.s7 & 0x80) >>> 7;
      var ms8 = (this.s8 & 0x80) >>> 7;
      var ms9 = (this.s9 & 0x80) >>> 7;
      var ms10 = (this.s10 & 0x80) >>> 7;
      var ms11 = (this.s11 & 0x80) >>> 7;
      var ms12 = (this.s12 & 0x80) >>> 7;
      var ms13 = (this.s13 & 0x80) >>> 7;
      var ms14 = (this.s14 & 0x80) >>> 7;
      var ms15 = (this.s15 & 0x80) >>> 7;
      return ms0 | ms1 << 1 | ms2 << 2 | ms3 << 3 |
             ms4 << 4 | ms5 << 5 | ms6 << 6 | ms7 << 7;
             ms8 << 8 | ms9 << 9 | ms10 << 10 | ms11 << 11;
             ms12 << 12 | ms13 << 13 | ms14 << 14 | ms15 << 15;
    }
  });
}

if (typeof SIMD.int8x16.zero === "undefined") {
  /**
    * Construct a new instance of int8x16 number with 0 in all lanes.
    * @constructor
    */
  SIMD.int8x16.zero = function() {
    return SIMD.int8x16(0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0);
  }
}

if (typeof SIMD.int8x16.bool === "undefined") {
  /**
    * Construct a new instance of int8x16 number with 0xFF or 0x0 in each
    * lane, depending on the truth value in s0, s1, s2, s3, s4, s5, s6, s7,
    * s8, s9, s10, s11, s12, s13, s14, and s15.
    * @param {boolean} flag used for s0 lane.
    * @param {boolean} flag used for s1 lane.
    * @param {boolean} flag used for s2 lane.
    * @param {boolean} flag used for s3 lane.
    * @param {boolean} flag used for s4 lane.
    * @param {boolean} flag used for s5 lane.
    * @param {boolean} flag used for s6 lane.
    * @param {boolean} flag used for s7 lane.
    * @param {boolean} flag used for s8 lane.
    * @param {boolean} flag used for s9 lane.
    * @param {boolean} flag used for s10 lane.
    * @param {boolean} flag used for s11 lane.
    * @param {boolean} flag used for s12 lane.
    * @param {boolean} flag used for s13 lane.
    * @param {boolean} flag used for s14 lane.
    * @param {boolean} flag used for s15 lane.
    * @constructor
    */
  SIMD.int8x16.bool = function(s0, s1, s2, s3, s4, s5, s6, s7,
                               s8, s9, s10, s11, s12, s13, s14, s15) {
    return SIMD.int8x16(s0 ? -1 : 0x0,
                        s1 ? -1 : 0x0,
                        s2 ? -1 : 0x0,
                        s3 ? -1 : 0x0,
                        s4 ? -1 : 0x0,
                        s5 ? -1 : 0x0,
                        s6 ? -1 : 0x0,
                        s7 ? -1 : 0x0,
                        s8 ? -1 : 0x0,
                        s9 ? -1 : 0x0,
                        s10 ? -1 : 0x0,
                        s11 ? -1 : 0x0,
                        s12 ? -1 : 0x0,
                        s13 ? -1 : 0x0,
                        s14 ? -1 : 0x0,
                        s15 ? -1 : 0x0);
  }
}

if (typeof SIMD.int8x16.splat === "undefined") {
  /**
    * Construct a new instance of int8x16 number with the same value
    * in all lanes.
    * @param {integer} value used for all lanes.
    * @constructor
    */
  SIMD.int8x16.splat = function(s) {
    return SIMD.int8x16(s, s, s, s, s, s, s, s,
                        s, s, s, s, s, s, s, s);
  }
}

if (typeof SIMD.int8x16.fromFloat32x4Bits === "undefined") {
  /**
    * @param {float32x4} t An instance of float32x4.
    * @return {int8x16} a bit-wise copy of t as a int8x16.
    */
  SIMD.int8x16.fromFloat32x4Bits = function(t) {
    _SIMD_PRIVATE.saveFloat32x4(t);
    return _SIMD_PRIVATE.restoreInt8x16();
  }
}

if (typeof SIMD.int8x16.fromFloat64x2Bits === "undefined") {
  /**
   * @param {float64x2} t An instance of float64x2.
   * @return {int8x16} a bit-wise copy of t as an int8x16.
   */
  SIMD.int8x16.fromFloat64x2Bits = function(t) {
    _SIMD_PRIVATE.saveFloat64x2(t);
    return _SIMD_PRIVATE.restoreInt8x16();
  }
}

if (typeof SIMD.int8x16.fromInt32x4Bits === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @return {int8x16} a bit-wise copy of t as a int8x16.
    */
  SIMD.int8x16.fromInt32x4Bits = function(t) {
    saveInt32x4(t);
    return restoreInt8x16();
  }
}

if (typeof SIMD.int8x16.fromInt16x8Bits === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @return {int8x16} a bit-wise copy of t as a int8x16.
    */
  SIMD.int8x16.fromInt16x8Bits = function(t) {
    saveInt16x8(t);
    return restoreInt8x16();
  }
}

if (typeof SIMD.float32x4.abs === "undefined") {
  /**
  * @return {float32x4} New instance of float32x4 with absolute values of
  * t.
  */
  SIMD.float32x4.abs = function(t) {
    t = SIMD.float32x4(t);
    return SIMD.float32x4(Math.abs(t.x), Math.abs(t.y), Math.abs(t.z),
                          Math.abs(t.w));
  }
}

if (typeof SIMD.float32x4.neg === "undefined") {
  /**
    * @return {float32x4} New instance of float32x4 with negated values of
    * t.
    */
  SIMD.float32x4.neg = function(t) {
    t = SIMD.float32x4(t);
    return SIMD.float32x4(-t.x, -t.y, -t.z, -t.w);
  }
}

if (typeof SIMD.float32x4.add === "undefined") {
  /**
    * @return {float32x4} New instance of float32x4 with a + b.
    */
  SIMD.float32x4.add = function(a, b) {
    a = SIMD.float32x4(a);
    b = SIMD.float32x4(b);
    return SIMD.float32x4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
  }
}

if (typeof SIMD.float32x4.sub === "undefined") {
  /**
    * @return {float32x4} New instance of float32x4 with a - b.
    */
  SIMD.float32x4.sub = function(a, b) {
    a = SIMD.float32x4(a);
    b = SIMD.float32x4(b);
    return SIMD.float32x4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
  }
}

if (typeof SIMD.float32x4.mul === "undefined") {
  /**
    * @return {float32x4} New instance of float32x4 with a * b.
    */
  SIMD.float32x4.mul = function(a, b) {
    a = SIMD.float32x4(a);
    b = SIMD.float32x4(b);
    return SIMD.float32x4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
  }
}

if (typeof SIMD.float32x4.div === "undefined") {
  /**
    * @return {float32x4} New instance of float32x4 with a / b.
    */
  SIMD.float32x4.div = function(a, b) {
    a = SIMD.float32x4(a);
    b = SIMD.float32x4(b);
    return SIMD.float32x4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
  }
}

if (typeof SIMD.float32x4.clamp === "undefined") {
  /**
    * @return {float32x4} New instance of float32x4 with t's values clamped
    * between lowerLimit and upperLimit.
    */
  SIMD.float32x4.clamp = function(t, lowerLimit, upperLimit) {
    t = SIMD.float32x4(t);
    lowerLimit = SIMD.float32x4(lowerLimit);
    upperLimit = SIMD.float32x4(upperLimit);
    var cx = t.x < lowerLimit.x ? lowerLimit.x : t.x;
    var cy = t.y < lowerLimit.y ? lowerLimit.y : t.y;
    var cz = t.z < lowerLimit.z ? lowerLimit.z : t.z;
    var cw = t.w < lowerLimit.w ? lowerLimit.w : t.w;
    cx = cx > upperLimit.x ? upperLimit.x : cx;
    cy = cy > upperLimit.y ? upperLimit.y : cy;
    cz = cz > upperLimit.z ? upperLimit.z : cz;
    cw = cw > upperLimit.w ? upperLimit.w : cw;
    return SIMD.float32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.float32x4.min === "undefined") {
  /**
    * @return {float32x4} New instance of float32x4 with the minimum value of
    * t and other.
    */
  SIMD.float32x4.min = function(t, other) {
    t = SIMD.float32x4(t);
    other = SIMD.float32x4(other);
    var cx = Math.min(t.x, other.x);
    var cy = Math.min(t.y, other.y);
    var cz = Math.min(t.z, other.z);
    var cw = Math.min(t.w, other.w);
    return SIMD.float32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.float32x4.max === "undefined") {
  /**
    * @return {float32x4} New instance of float32x4 with the maximum value of
    * t and other.
    */
  SIMD.float32x4.max = function(t, other) {
    t = SIMD.float32x4(t);
    other = SIMD.float32x4(other);
    var cx = Math.max(t.x, other.x);
    var cy = Math.max(t.y, other.y);
    var cz = Math.max(t.z, other.z);
    var cw = Math.max(t.w, other.w);
    return SIMD.float32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.float32x4.minNum === "undefined") {
  /**
    * @return {float32x4} New instance of float32x4 with the minimum value of
    * t and other, preferring numbers over NaNs.
    */
  SIMD.float32x4.minNum = function(t, other) {
    t = SIMD.float32x4(t);
    other = SIMD.float32x4(other);
    var cx = _SIMD_PRIVATE.minNum(t.x, other.x);
    var cy = _SIMD_PRIVATE.minNum(t.y, other.y);
    var cz = _SIMD_PRIVATE.minNum(t.z, other.z);
    var cw = _SIMD_PRIVATE.minNum(t.w, other.w);
    return SIMD.float32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.float32x4.maxNum === "undefined") {
  /**
    * @return {float32x4} New instance of float32x4 with the maximum value of
    * t and other, preferring numbers over NaNs.
    */
  SIMD.float32x4.maxNum = function(t, other) {
    t = SIMD.float32x4(t);
    other = SIMD.float32x4(other);
    var cx = _SIMD_PRIVATE.maxNum(t.x, other.x);
    var cy = _SIMD_PRIVATE.maxNum(t.y, other.y);
    var cz = _SIMD_PRIVATE.maxNum(t.z, other.z);
    var cw = _SIMD_PRIVATE.maxNum(t.w, other.w);
    return SIMD.float32x4(cx, cy, cz, cw);
  }
}

if (typeof SIMD.float32x4.reciprocal === "undefined") {
  /**
    * @return {float32x4} New instance of float32x4 with reciprocal value of
    * t.
    */
  SIMD.float32x4.reciprocal = function(t) {
    t = SIMD.float32x4(t);
    return SIMD.float32x4(1.0 / t.x, 1.0 / t.y, 1.0 / t.z, 1.0 / t.w);
  }
}

if (typeof SIMD.float32x4.reciprocalSqrt === "undefined") {
  /**
    * @return {float32x4} New instance of float32x4 with square root of the
    * reciprocal value of t.
    */
  SIMD.float32x4.reciprocalSqrt = function(t) {
    t = SIMD.float32x4(t);
    return SIMD.float32x4(Math.sqrt(1.0 / t.x), Math.sqrt(1.0 / t.y),
                          Math.sqrt(1.0 / t.z), Math.sqrt(1.0 / t.w));
  }
}

if (typeof SIMD.float32x4.scale === "undefined") {
  /**
    * @return {float32x4} New instance of float32x4 with values of t
    * scaled by s.
    */
  SIMD.float32x4.scale = function(t, s) {
    t = SIMD.float32x4(t);
    var s4 = SIMD.float32x4.splat(s);
    return SIMD.float32x4.mul(t,s4);
  }
}

if (typeof SIMD.float32x4.sqrt === "undefined") {
  /**
    * @return {float32x4} New instance of float32x4 with square root of
    * values of t.
    */
  SIMD.float32x4.sqrt = function(t) {
    t = SIMD.float32x4(t);
    return SIMD.float32x4(Math.sqrt(t.x), Math.sqrt(t.y),
                          Math.sqrt(t.z), Math.sqrt(t.w));
  }
}

if (typeof SIMD.float32x4.swizzle === "undefined") {
  /**
    * @param {float32x4} t An instance of float32x4 to be swizzled.
    * @param {integer} x - Index for lane x
    * @param {integer} y - Index for lane y
    * @param {integer} z - Index for lane z
    * @param {integer} w - Index for lane w
    * @return {float32x4} New instance of float32x4 with lanes swizzled.
    */
  SIMD.float32x4.swizzle = function(t, x, y, z, w) {
    t = SIMD.float32x4(t);
    _SIMD_PRIVATE._f32x4[0] = t.x;
    _SIMD_PRIVATE._f32x4[1] = t.y;
    _SIMD_PRIVATE._f32x4[2] = t.z;
    _SIMD_PRIVATE._f32x4[3] = t.w;
    var storage = _SIMD_PRIVATE._f32x4;
    return SIMD.float32x4(storage[x], storage[y], storage[z], storage[w]);
  }
}

if (typeof SIMD.float32x4.shuffle === "undefined" ||
    typeof SIMD.float32x4.shuffleMix !== "undefined") {

  _SIMD_PRIVATE._f32x8 = new Float32Array(8);

  /**
    * @param {float32x4} t1 An instance of float32x4 to be shuffled.
    * @param {float32x4} t2 An instance of float32x4 to be shuffled.
    * @param {integer} x - Index in concatenation of t1 and t2 for lane x
    * @param {integer} y - Index in concatenation of t1 and t2 for lane y
    * @param {integer} z - Index in concatenation of t1 and t2 for lane z
    * @param {integer} w - Index in concatenation of t1 and t2 for lane w
    * @return {float32x4} New instance of float32x4 with lanes shuffled.
    */
  SIMD.float32x4.shuffle = function(t1, t2, x, y, z, w) {
    t1 = SIMD.float32x4(t1);
    t2 = SIMD.float32x4(t2);
    var storage = _SIMD_PRIVATE._f32x8;
    storage[0] = t1.x;
    storage[1] = t1.y;
    storage[2] = t1.z;
    storage[3] = t1.w;
    storage[4] = t2.x;
    storage[5] = t2.y;
    storage[6] = t2.z;
    storage[7] = t2.w;
    return SIMD.float32x4(storage[x], storage[y], storage[z], storage[w]);
  }
}

if (typeof SIMD.float32x4.withX === "undefined") {
  /**
    * @param {double} value used for x lane.
    * @return {float32x4} New instance of float32x4 with the values in t and
    * x replaced with {x}.
    */
  SIMD.float32x4.withX = function(t, x) {
    t = SIMD.float32x4(t);
    return SIMD.float32x4(x, t.y, t.z, t.w);
  }
}

if (typeof SIMD.float32x4.withY === "undefined") {
  /**
    * @param {double} value used for y lane.
    * @return {float32x4} New instance of float32x4 with the values in t and
    * y replaced with {y}.
    */
  SIMD.float32x4.withY = function(t, y) {
    t = SIMD.float32x4(t);
    return SIMD.float32x4(t.x, y, t.z, t.w);
  }
}

if (typeof SIMD.float32x4.withZ === "undefined") {
  /**
    * @param {double} value used for z lane.
    * @return {float32x4} New instance of float32x4 with the values in t and
    * z replaced with {z}.
    */
  SIMD.float32x4.withZ = function(t, z) {
    t = SIMD.float32x4(t);
    return SIMD.float32x4(t.x, t.y, z, t.w);
  }
}

if (typeof SIMD.float32x4.withW === "undefined") {
  /**
    * @param {double} value used for w lane.
    * @return {float32x4} New instance of float32x4 with the values in t and
    * w replaced with {w}.
    */
  SIMD.float32x4.withW = function(t, w) {
    t = SIMD.float32x4(t);
    return SIMD.float32x4(t.x, t.y, t.z, w);
  }
}

if (typeof SIMD.float32x4.lessThan === "undefined") {
  /**
    * @param {float32x4} t An instance of float32x4.
    * @param {float32x4} other An instance of float32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t < other.
    */
  SIMD.float32x4.lessThan = function(t, other) {
    t = SIMD.float32x4(t);
    other = SIMD.float32x4(other);
    var cx = t.x < other.x;
    var cy = t.y < other.y;
    var cz = t.z < other.z;
    var cw = t.w < other.w;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.float32x4.lessThanOrEqual === "undefined") {
  /**
    * @param {float32x4} t An instance of float32x4.
    * @param {float32x4} other An instance of float32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t <= other.
    */
  SIMD.float32x4.lessThanOrEqual = function(t, other) {
    t = SIMD.float32x4(t);
    other = SIMD.float32x4(other);
    var cx = t.x <= other.x;
    var cy = t.y <= other.y;
    var cz = t.z <= other.z;
    var cw = t.w <= other.w;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.float32x4.equal === "undefined") {
  /**
    * @param {float32x4} t An instance of float32x4.
    * @param {float32x4} other An instance of float32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t == other.
    */
  SIMD.float32x4.equal = function(t, other) {
    t = SIMD.float32x4(t);
    other = SIMD.float32x4(other);
    var cx = t.x == other.x;
    var cy = t.y == other.y;
    var cz = t.z == other.z;
    var cw = t.w == other.w;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.float32x4.notEqual === "undefined") {
  /**
    * @param {float32x4} t An instance of float32x4.
    * @param {float32x4} other An instance of float32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t != other.
    */
  SIMD.float32x4.notEqual = function(t, other) {
    t = SIMD.float32x4(t);
    other = SIMD.float32x4(other);
    var cx = t.x != other.x;
    var cy = t.y != other.y;
    var cz = t.z != other.z;
    var cw = t.w != other.w;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.float32x4.greaterThanOrEqual === "undefined") {
  /**
    * @param {float32x4} t An instance of float32x4.
    * @param {float32x4} other An instance of float32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t >= other.
    */
  SIMD.float32x4.greaterThanOrEqual = function(t, other) {
    t = SIMD.float32x4(t);
    other = SIMD.float32x4(other);
    var cx = t.x >= other.x;
    var cy = t.y >= other.y;
    var cz = t.z >= other.z;
    var cw = t.w >= other.w;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.float32x4.greaterThan === "undefined") {
  /**
    * @param {float32x4} t An instance of float32x4.
    * @param {float32x4} other An instance of float32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t > other.
    */
  SIMD.float32x4.greaterThan = function(t, other) {
    t = SIMD.float32x4(t);
    other = SIMD.float32x4(other);
    var cx = t.x > other.x;
    var cy = t.y > other.y;
    var cz = t.z > other.z;
    var cw = t.w > other.w;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.float32x4.select === "undefined") {
  /**
    * @param {int32x4} t Selector mask. An instance of int32x4
    * @param {float32x4} trueValue Pick lane from here if corresponding
    * selector lane is 0xFFFFFFFF
    * @param {float32x4} falseValue Pick lane from here if corresponding
    * selector lane is 0x0
    * @return {float32x4} Mix of lanes from trueValue or falseValue as
    * indicated
    */
  SIMD.float32x4.select = function(t, trueValue, falseValue) {
    t = SIMD.int32x4(t);
    trueValue = SIMD.float32x4(trueValue);
    falseValue = SIMD.float32x4(falseValue);
    var tv = SIMD.int32x4.fromFloat32x4Bits(trueValue);
    var fv = SIMD.int32x4.fromFloat32x4Bits(falseValue);
    var tr = SIMD.int32x4.and(t, tv);
    var fr = SIMD.int32x4.and(SIMD.int32x4.not(t), fv);
    return SIMD.float32x4.fromInt32x4Bits(SIMD.int32x4.or(tr, fr));
  }
}

if (typeof SIMD.float32x4.and === "undefined") {
  /**
    * @param {float32x4} a An instance of float32x4.
    * @param {float32x4} b An instance of float32x4.
    * @return {float32x4} New instance of float32x4 with values of a & b.
    */
  SIMD.float32x4.and = function(a, b) {
    a = SIMD.float32x4(a);
    b = SIMD.float32x4(b);
    var aInt = SIMD.int32x4.fromFloat32x4Bits(a);
    var bInt = SIMD.int32x4.fromFloat32x4Bits(b);
    return SIMD.float32x4.fromInt32x4Bits(SIMD.int32x4.and(aInt, bInt));
  }
}

if (typeof SIMD.float32x4.or === "undefined") {
  /**
    * @param {float32x4} a An instance of float32x4.
    * @param {float32x4} b An instance of float32x4.
    * @return {float32x4} New instance of float32x4 with values of a | b.
    */
  SIMD.float32x4.or = function(a, b) {
    a = SIMD.float32x4(a);
    b = SIMD.float32x4(b);
    var aInt = SIMD.int32x4.fromFloat32x4Bits(a);
    var bInt = SIMD.int32x4.fromFloat32x4Bits(b);
    return SIMD.float32x4.fromInt32x4Bits(SIMD.int32x4.or(aInt, bInt));
  }
}

if (typeof SIMD.float32x4.xor === "undefined") {
  /**
    * @param {float32x4} a An instance of float32x4.
    * @param {float32x4} b An instance of float32x4.
    * @return {float32x4} New instance of float32x4 with values of a ^ b.
    */
  SIMD.float32x4.xor = function(a, b) {
    a = SIMD.float32x4(a);
    b = SIMD.float32x4(b);
    var aInt = SIMD.int32x4.fromFloat32x4Bits(a);
    var bInt = SIMD.int32x4.fromFloat32x4Bits(b);
    return SIMD.float32x4.fromInt32x4Bits(SIMD.int32x4.xor(aInt, bInt));
  }
}

if (typeof SIMD.float32x4.not === "undefined") {
  /**
    * @param {float32x4} a An instance of float32x4.
    * @return {float32x4} New instance of float32x4 with values of ~a.
    */
  SIMD.float32x4.not = function(a) {
    a = SIMD.float32x4(a);
    var aInt = SIMD.int32x4.fromFloat32x4Bits(a);
    return SIMD.float32x4.fromInt32x4Bits(SIMD.int32x4.not(aInt));
  }
}

if (typeof SIMD.float32x4.load === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {float32x4} New instance of float32x4.
    */
  SIMD.float32x4.load = function(tarray, index) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var f32temp = _SIMD_PRIVATE._f32x4;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? f32temp : _SIMD_PRIVATE._i32x4) :
                _SIMD_PRIVATE._f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.float32x4(f32temp[0], f32temp[1], f32temp[2], f32temp[3]);
  }
}

if (typeof SIMD.float32x4.loadX === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {float32x4} New instance of float32x4.
    */
  SIMD.float32x4.loadX = function(tarray, index) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 4) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var f32temp = _SIMD_PRIVATE._f32x4;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? f32temp : _SIMD_PRIVATE._i32x4) :
                _SIMD_PRIVATE._f64x2;
    var n = 4 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.float32x4(f32temp[0], 0.0, 0.0, 0.0);
  }
}

if (typeof SIMD.float32x4.loadXY === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {float32x4} New instance of float32x4.
    */
  SIMD.float32x4.loadXY = function(tarray, index) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 8) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var f32temp = _SIMD_PRIVATE._f32x4;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? f32temp : _SIMD_PRIVATE._i32x4) :
                _SIMD_PRIVATE._f64x2;
    var n = 8 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.float32x4(f32temp[0], f32temp[1], 0.0, 0.0);
  }
}

if (typeof SIMD.float32x4.loadXYZ === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {float32x4} New instance of float32x4.
    */
  SIMD.float32x4.loadXYZ = function(tarray, index) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 12) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var f32temp = _SIMD_PRIVATE._f32x4;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? f32temp : _SIMD_PRIVATE._i32x4) :
                _SIMD_PRIVATE._f64x2;
    var n = 12 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.float32x4(f32temp[0], f32temp[1], f32temp[2], 0.0);
  }
}

if (typeof SIMD.float32x4.store === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {float32x4} value An instance of float32x4.
    * @return {void}
    */
  SIMD.float32x4.store = function(tarray, index, value) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.float32x4(value);
    _SIMD_PRIVATE._f32x4[0] = value.x;
    _SIMD_PRIVATE._f32x4[1] = value.y;
    _SIMD_PRIVATE._f32x4[2] = value.z;
    _SIMD_PRIVATE._f32x4[3] = value.w;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4) :
                _SIMD_PRIVATE._f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      tarray[index + i] = array[i];
  }
}

if (typeof SIMD.float32x4.storeX === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {float32x4} value An instance of float32x4.
    * @return {void}
    */
  SIMD.float32x4.storeX = function(tarray, index, value) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 4) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.float32x4(value);
    if (bpe == 8) {
      // tarray's elements are too wide. Just create a new view; this is rare.
      var view = new Float32Array(tarray.buffer, tarray.byteOffset + index * 8, 1);
      view[0] = value.x;
    } else {
      _SIMD_PRIVATE._f32x4[0] = value.x;
      var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                  bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                  (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4);
      var n = 4 / bpe;
      for (var i = 0; i < n; ++i)
        tarray[index + i] = array[i];
    }
  }
}

if (typeof SIMD.float32x4.storeXY === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {float32x4} value An instance of float32x4.
    * @return {void}
    */
  SIMD.float32x4.storeXY = function(tarray, index, value) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 8) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.float32x4(value);
    _SIMD_PRIVATE._f32x4[0] = value.x;
    _SIMD_PRIVATE._f32x4[1] = value.y;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4) :
                _SIMD_PRIVATE._f64x2;
    var n = 8 / bpe;
    for (var i = 0; i < n; ++i)
      tarray[index + i] = array[i];
  }
}

if (typeof SIMD.float32x4.storeXYZ === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {float32x4} value An instance of float32x4.
    * @return {void}
    */
  SIMD.float32x4.storeXYZ = function(tarray, index, value) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 12) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.float32x4(value);
    if (bpe == 8) {
      // tarray's elements are too wide. Just create a new view; this is rare.
      var view = new Float32Array(tarray.buffer, tarray.byteOffset + index * 8, 3);
      view[0] = value.x;
      view[1] = value.y;
      view[2] = value.z;
    } else {
      _SIMD_PRIVATE._f32x4[0] = value.x;
      _SIMD_PRIVATE._f32x4[1] = value.y;
      _SIMD_PRIVATE._f32x4[2] = value.z;
      var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                  bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                  (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4);
      var n = 12 / bpe;
      for (var i = 0; i < n; ++i)
        tarray[index + i] = array[i];
    }
  }
}

if (typeof SIMD.float64x2.abs === "undefined") {
  /**
  * @return {float64x2} New instance of float64x2 with absolute values of
  * t.
  */
  SIMD.float64x2.abs = function(t) {
    t = SIMD.float64x2(t);
    return SIMD.float64x2(Math.abs(t.x), Math.abs(t.y));
  }
}

if (typeof SIMD.float64x2.neg === "undefined") {
  /**
    * @return {float64x2} New instance of float64x2 with negated values of
    * t.
    */
  SIMD.float64x2.neg = function(t) {
    t = SIMD.float64x2(t);
    return SIMD.float64x2(-t.x, -t.y);
  }
}

if (typeof SIMD.float64x2.add === "undefined") {
  /**
    * @return {float64x2} New instance of float64x2 with a + b.
    */
  SIMD.float64x2.add = function(a, b) {
    a = SIMD.float64x2(a);
    b = SIMD.float64x2(b);
    return SIMD.float64x2(a.x + b.x, a.y + b.y);
  }
}

if (typeof SIMD.float64x2.sub === "undefined") {
  /**
    * @return {float64x2} New instance of float64x2 with a - b.
    */
  SIMD.float64x2.sub = function(a, b) {
    a = SIMD.float64x2(a);
    b = SIMD.float64x2(b);
    return SIMD.float64x2(a.x - b.x, a.y - b.y);
  }
}

if (typeof SIMD.float64x2.mul === "undefined") {
  /**
    * @return {float64x2} New instance of float64x2 with a * b.
    */
  SIMD.float64x2.mul = function(a, b) {
    a = SIMD.float64x2(a);
    b = SIMD.float64x2(b);
    return SIMD.float64x2(a.x * b.x, a.y * b.y);
  }
}

if (typeof SIMD.float64x2.div === "undefined") {
  /**
    * @return {float64x2} New instance of float64x2 with a / b.
    */
  SIMD.float64x2.div = function(a, b) {
    a = SIMD.float64x2(a);
    b = SIMD.float64x2(b);
    return SIMD.float64x2(a.x / b.x, a.y / b.y);
  }
}

if (typeof SIMD.float64x2.clamp === "undefined") {
  /**
    * @return {float64x2} New instance of float64x2 with t's values clamped
    * between lowerLimit and upperLimit.
    */
  SIMD.float64x2.clamp = function(t, lowerLimit, upperLimit) {
    t = SIMD.float64x2(t);
    lowerLimit = SIMD.float64x2(lowerLimit);
    upperLimit = SIMD.float64x2(upperLimit);
    var cx = t.x < lowerLimit.x ? lowerLimit.x : t.x;
    var cy = t.y < lowerLimit.y ? lowerLimit.y : t.y;
    cx = cx > upperLimit.x ? upperLimit.x : cx;
    cy = cy > upperLimit.y ? upperLimit.y : cy;
    return SIMD.float64x2(cx, cy);
  }
}

if (typeof SIMD.float64x2.min === "undefined") {
  /**
    * @return {float64x2} New instance of float64x2 with the minimum value of
    * t and other.
    */
  SIMD.float64x2.min = function(t, other) {
    t = SIMD.float64x2(t);
    other = SIMD.float64x2(other);
    var cx = Math.min(t.x, other.x);
    var cy = Math.min(t.y, other.y);
    return SIMD.float64x2(cx, cy);
  }
}

if (typeof SIMD.float64x2.max === "undefined") {
  /**
    * @return {float64x2} New instance of float64x2 with the maximum value of
    * t and other.
    */
  SIMD.float64x2.max = function(t, other) {
    t = SIMD.float64x2(t);
    other = SIMD.float64x2(other);
    var cx = Math.max(t.x, other.x);
    var cy = Math.max(t.y, other.y);
    return SIMD.float64x2(cx, cy);
  }
}

if (typeof SIMD.float64x2.minNum === "undefined") {
  /**
    * @return {float64x2} New instance of float64x2 with the minimum value of
    * t and other, preferring numbers over NaNs.
    */
  SIMD.float64x2.minNum = function(t, other) {
    t = SIMD.float64x2(t);
    other = SIMD.float64x2(other);
    var cx = _SIMD_PRIVATE.minNum(t.x, other.x);
    var cy = _SIMD_PRIVATE.minNum(t.y, other.y);
    return SIMD.float64x2(cx, cy);
  }
}

if (typeof SIMD.float64x2.maxNum === "undefined") {
  /**
    * @return {float64x2} New instance of float64x2 with the maximum value of
    * t and other, preferring numbers over NaNs.
    */
  SIMD.float64x2.maxNum = function(t, other) {
    t = SIMD.float64x2(t);
    other = SIMD.float64x2(other);
    var cx = _SIMD_PRIVATE.maxNum(t.x, other.x);
    var cy = _SIMD_PRIVATE.maxNum(t.y, other.y);
    return SIMD.float64x2(cx, cy);
  }
}

if (typeof SIMD.float64x2.reciprocal === "undefined") {
  /**
    * @return {float64x2} New instance of float64x2 with reciprocal value of
    * t.
    */
  SIMD.float64x2.reciprocal = function(t) {
    t = SIMD.float64x2(t);
    return SIMD.float64x2(1.0 / t.x, 1.0 / t.y);
  }
}

if (typeof SIMD.float64x2.reciprocalSqrt === "undefined") {
  /**
    * @return {float64x2} New instance of float64x2 with square root of the
    * reciprocal value of t.
    */
  SIMD.float64x2.reciprocalSqrt = function(t) {
    t = SIMD.float64x2(t);
    return SIMD.float64x2(Math.sqrt(1.0 / t.x), Math.sqrt(1.0 / t.y));
  }
}

if (typeof SIMD.float64x2.scale === "undefined") {
  /**
    * @return {float64x2} New instance of float64x2 with values of t
    * scaled by s.
    */
  SIMD.float64x2.scale = function(t, s) {
    t = SIMD.float64x2(t);
    return SIMD.float64x2(s * t.x, s * t.y);
  }
}

if (typeof SIMD.float64x2.sqrt === "undefined") {
  /**
    * @return {float64x2} New instance of float64x2 with square root of
    * values of t.
    */
  SIMD.float64x2.sqrt = function(t) {
    t = SIMD.float64x2(t);
    return SIMD.float64x2(Math.sqrt(t.x), Math.sqrt(t.y));
  }
}

if (typeof SIMD.float64x2.swizzle === "undefined") {
  /**
    * @param {float64x2} t An instance of float64x2 to be swizzled.
    * @param {integer} x - Index for lane x
    * @param {integer} y - Index for lane y
    * @return {float64x2} New instance of float64x2 with lanes swizzled.
    */
  SIMD.float64x2.swizzle = function(t, x, y) {
    t = SIMD.float64x2(t);
    var storage = _SIMD_PRIVATE._f64x2;
    storage[0] = t.x;
    storage[1] = t.y;
    return SIMD.float64x2(storage[x], storage[y]);
  }
}

if (typeof SIMD.float64x2.shuffle === "undefined" ||
    typeof SIMD.float64x2.shuffleMix !== "undefined") {

  _SIMD_PRIVATE._f64x4 = new Float64Array(4);

  /**
    * @param {float64x2} t1 An instance of float64x2 to be shuffled.
    * @param {float64x2} t2 An instance of float64x2 to be shuffled.
    * @param {integer} x - Index in concatenation of t1 and t2 for lane x
    * @param {integer} y - Index in concatenation of t1 and t2 for lane y
    * @return {float64x2} New instance of float64x2 with lanes shuffled.
    */
  SIMD.float64x2.shuffle = function(t1, t2, x, y) {
    t1 = SIMD.float64x2(t1);
    t2 = SIMD.float64x2(t2);
    var storage = _SIMD_PRIVATE._f64x4;
    storage[0] = t1.x;
    storage[1] = t1.y;
    storage[2] = t2.x;
    storage[3] = t2.y;
    return SIMD.float64x2(storage[x], storage[y]);
  }
}

if (typeof SIMD.float64x2.withX === "undefined") {
  /**
    * @param {double} value used for x lane.
    * @return {float64x2} New instance of float64x2 with the values in t and
    * x replaced with {x}.
    */
  SIMD.float64x2.withX = function(t, x) {
    t = SIMD.float64x2(t);
    return SIMD.float64x2(x, t.y);
  }
}

if (typeof SIMD.float64x2.withY === "undefined") {
  /**
    * @param {double} value used for y lane.
    * @return {float64x2} New instance of float64x2 with the values in t and
    * y replaced with {y}.
    */
  SIMD.float64x2.withY = function(t, y) {
    t = SIMD.float64x2(t);
    return SIMD.float64x2(t.x, y);
  }
}

if (typeof SIMD.float64x2.lessThan === "undefined") {
  /**
    * @param {float64x2} t An instance of float64x2.
    * @param {float64x2} other An instance of float64x2.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t < other.
    */
  SIMD.float64x2.lessThan = function(t, other) {
    t = SIMD.float64x2(t);
    other = SIMD.float64x2(other);
    var cx = t.x < other.x;
    var cy = t.y < other.y;
    return SIMD.int32x4.bool(cx, cx, cy, cy);
  }
}

if (typeof SIMD.float64x2.lessThanOrEqual === "undefined") {
  /**
    * @param {float64x2} t An instance of float64x2.
    * @param {float64x2} other An instance of float64x2.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t <= other.
    */
  SIMD.float64x2.lessThanOrEqual = function(t, other) {
    t = SIMD.float64x2(t);
    other = SIMD.float64x2(other);
    var cx = t.x <= other.x;
    var cy = t.y <= other.y;
    return SIMD.int32x4.bool(cx, cx, cy, cy);
  }
}

if (typeof SIMD.float64x2.equal === "undefined") {
  /**
    * @param {float64x2} t An instance of float64x2.
    * @param {float64x2} other An instance of float64x2.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t == other.
    */
  SIMD.float64x2.equal = function(t, other) {
    t = SIMD.float64x2(t);
    other = SIMD.float64x2(other);
    var cx = t.x == other.x;
    var cy = t.y == other.y;
    return SIMD.int32x4.bool(cx, cx, cy, cy);
  }
}

if (typeof SIMD.float64x2.notEqual === "undefined") {
  /**
    * @param {float64x2} t An instance of float64x2.
    * @param {float64x2} other An instance of float64x2.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t != other.
    */
  SIMD.float64x2.notEqual = function(t, other) {
    t = SIMD.float64x2(t);
    other = SIMD.float64x2(other);
    var cx = t.x != other.x;
    var cy = t.y != other.y;
    return SIMD.int32x4.bool(cx, cx, cy, cy);
  }
}

if (typeof SIMD.float64x2.greaterThanOrEqual === "undefined") {
  /**
    * @param {float64x2} t An instance of float64x2.
    * @param {float64x2} other An instance of float64x2.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t >= other.
    */
  SIMD.float64x2.greaterThanOrEqual = function(t, other) {
    t = SIMD.float64x2(t);
    other = SIMD.float64x2(other);
    var cx = t.x >= other.x;
    var cy = t.y >= other.y;
    return SIMD.int32x4.bool(cx, cx, cy, cy);
  }
}

if (typeof SIMD.float64x2.greaterThan === "undefined") {
  /**
    * @param {float64x2} t An instance of float64x2.
    * @param {float64x2} other An instance of float64x2.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t > other.
    */
  SIMD.float64x2.greaterThan = function(t, other) {
    t = SIMD.float64x2(t);
    other = SIMD.float64x2(other);
    var cx = t.x > other.x;
    var cy = t.y > other.y;
    return SIMD.int32x4.bool(cx, cx, cy, cy);
  }
}

if (typeof SIMD.float64x2.select === "undefined") {
  /**
    * @param {int32x4} t Selector mask. An instance of int32x4
    * @param {float64x2} trueValue Pick lane from here if corresponding
    * selector lanes are 0xFFFFFFFF
    * @param {float64x2} falseValue Pick lane from here if corresponding
    * selector lanes are 0x0
    * @return {float64x2} Mix of lanes from trueValue or falseValue as
    * indicated
    */
  SIMD.float64x2.select = function(t, trueValue, falseValue) {
    t = SIMD.int32x4(t);
    trueValue = SIMD.float64x2(trueValue);
    falseValue = SIMD.float64x2(falseValue);
    var tv = SIMD.int32x4.fromFloat64x2Bits(trueValue);
    var fv = SIMD.int32x4.fromFloat64x2Bits(falseValue);
    var tr = SIMD.int32x4.and(t, tv);
    var fr = SIMD.int32x4.and(SIMD.int32x4.not(t), fv);
    return SIMD.float64x2.fromInt32x4Bits(SIMD.int32x4.or(tr, fr));
  }
}

if (typeof SIMD.float64x2.load === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {float64x2} New instance of float64x2.
    */
  SIMD.float64x2.load = function(tarray, index) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var f64temp = _SIMD_PRIVATE._f64x2;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4) :
                f64temp;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.float64x2(f64temp[0], f64temp[1]);
  }
}

if (typeof SIMD.float64x2.loadX === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {float64x2} New instance of float64x2.
    */
  SIMD.float64x2.loadX = function(tarray, index) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 8) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var f64temp = _SIMD_PRIVATE._f64x2;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4) :
                f64temp;
    var n = 8 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.float64x2(f64temp[0], 0.0);
  }
}

if (typeof SIMD.float64x2.store === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {float64x2} value An instance of float64x2.
    * @return {void}
    */
  SIMD.float64x2.store = function(tarray, index, value) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.float64x2(value);
    _SIMD_PRIVATE._f64x2[0] = value.x;
    _SIMD_PRIVATE._f64x2[1] = value.y;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4) :
                _SIMD_PRIVATE._f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      tarray[index + i] = array[i];
  }
}

if (typeof SIMD.float64x2.storeX === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {float64x2} value An instance of float64x2.
    * @return {void}
    */
  SIMD.float64x2.storeX = function(tarray, index, value) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 8) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.float64x2(value);
    _SIMD_PRIVATE._f64x2[0] = value.x;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4) :
                _SIMD_PRIVATE._f64x2;
    var n = 8 / bpe;
    for (var i = 0; i < n; ++i)
      tarray[index + i] = array[i];
  }
}

if (typeof SIMD.int32x4.and === "undefined") {
  /**
    * @param {int32x4} a An instance of int32x4.
    * @param {int32x4} b An instance of int32x4.
    * @return {int32x4} New instance of int32x4 with values of a & b.
    */
  SIMD.int32x4.and = function(a, b) {
    a = SIMD.int32x4(a);
    b = SIMD.int32x4(b);
    return SIMD.int32x4(a.x & b.x, a.y & b.y, a.z & b.z, a.w & b.w);
  }
}

if (typeof SIMD.int32x4.or === "undefined") {
  /**
    * @param {int32x4} a An instance of int32x4.
    * @param {int32x4} b An instance of int32x4.
    * @return {int32x4} New instance of int32x4 with values of a | b.
    */
  SIMD.int32x4.or = function(a, b) {
    a = SIMD.int32x4(a);
    b = SIMD.int32x4(b);
    return SIMD.int32x4(a.x | b.x, a.y | b.y, a.z | b.z, a.w | b.w);
  }
}

if (typeof SIMD.int32x4.xor === "undefined") {
  /**
    * @param {int32x4} a An instance of int32x4.
    * @param {int32x4} b An instance of int32x4.
    * @return {int32x4} New instance of int32x4 with values of a ^ b.
    */
  SIMD.int32x4.xor = function(a, b) {
    a = SIMD.int32x4(a);
    b = SIMD.int32x4(b);
    return SIMD.int32x4(a.x ^ b.x, a.y ^ b.y, a.z ^ b.z, a.w ^ b.w);
  }
}

if (typeof SIMD.int32x4.not === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @return {int32x4} New instance of int32x4 with values of ~t
    */
  SIMD.int32x4.not = function(t) {
    t = SIMD.int32x4(t);
    return SIMD.int32x4(~t.x, ~t.y, ~t.z, ~t.w);
  }
}

if (typeof SIMD.int32x4.neg === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @return {int32x4} New instance of int32x4 with values of -t
    */
  SIMD.int32x4.neg = function(t) {
    t = SIMD.int32x4(t);
    return SIMD.int32x4(-t.x, -t.y, -t.z, -t.w);
  }
}

if (typeof SIMD.int32x4.add === "undefined") {
  /**
    * @param {int32x4} a An instance of int32x4.
    * @param {int32x4} b An instance of int32x4.
    * @return {int32x4} New instance of int32x4 with values of a + b.
    */
  SIMD.int32x4.add = function(a, b) {
    a = SIMD.int32x4(a);
    b = SIMD.int32x4(b);
    return SIMD.int32x4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
  }
}

if (typeof SIMD.int32x4.sub === "undefined") {
  /**
    * @param {int32x4} a An instance of int32x4.
    * @param {int32x4} b An instance of int32x4.
    * @return {int32x4} New instance of int32x4 with values of a - b.
    */
  SIMD.int32x4.sub = function(a, b) {
    a = SIMD.int32x4(a);
    b = SIMD.int32x4(b);
    return SIMD.int32x4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
  }
}

if (typeof SIMD.int32x4.mul === "undefined") {
  /**
    * @param {int32x4} a An instance of int32x4.
    * @param {int32x4} b An instance of int32x4.
    * @return {int32x4} New instance of int32x4 with values of a * b.
    */
  SIMD.int32x4.mul = function(a, b) {
    a = SIMD.int32x4(a);
    b = SIMD.int32x4(b);
    return SIMD.int32x4(Math.imul(a.x, b.x), Math.imul(a.y, b.y),
                        Math.imul(a.z, b.z), Math.imul(a.w, b.w));
  }
}

if (typeof SIMD.int32x4.swizzle === "undefined") {
  /**
    * @param {int32x4} t An instance of float32x4 to be swizzled.
    * @param {integer} x - Index for lane x
    * @param {integer} y - Index for lane y
    * @param {integer} z - Index for lane z
    * @param {integer} w - Index for lane w
    * @return {int32x4} New instance of float32x4 with lanes swizzled.
    */
  SIMD.int32x4.swizzle = function(t, x, y, z, w) {
    t = SIMD.int32x4(t);
    var storage = _SIMD_PRIVATE._i32x4;
    storage[0] = t.x;
    storage[1] = t.y;
    storage[2] = t.z;
    storage[3] = t.w;
    return SIMD.int32x4(storage[x], storage[y], storage[z], storage[w]);
  }
}

if (typeof SIMD.int32x4.shuffle === "undefined" ||
    typeof SIMD.int32x4.shuffleMix !== "undefined") {

  _SIMD_PRIVATE._i32x8 = new Int32Array(8);

  /**
    * @param {int32x4} t1 An instance of float32x4 to be shuffled.
    * @param {int32x4} t2 An instance of float32x4 to be shuffled.
    * @param {integer} x - Index in concatenation of t1 and t2 for lane x
    * @param {integer} y - Index in concatenation of t1 and t2 for lane y
    * @param {integer} z - Index in concatenation of t1 and t2 for lane z
    * @param {integer} w - Index in concatenation of t1 and t2 for lane w
    * @return {int32x4} New instance of float32x4 with lanes shuffled.
    */
  SIMD.int32x4.shuffle = function(t1, t2, x, y, z, w) {
    t1 = SIMD.int32x4(t1);
    t2 = SIMD.int32x4(t2);
    var storage = _SIMD_PRIVATE._i32x8;
    storage[0] = t1.x;
    storage[1] = t1.y;
    storage[2] = t1.z;
    storage[3] = t1.w;
    storage[4] = t2.x;
    storage[5] = t2.y;
    storage[6] = t2.z;
    storage[7] = t2.w;
    return SIMD.float32x4(storage[x], storage[y], storage[z], storage[w]);
  }
}

if (typeof SIMD.int32x4.select === "undefined") {
  /**
    * @param {int32x4} t Selector mask. An instance of int32x4
    * @param {int32x4} trueValue Pick lane from here if corresponding
    * selector lane is 0xFFFFFFFF
    * @param {int32x4} falseValue Pick lane from here if corresponding
    * selector lane is 0x0
    * @return {int32x4} Mix of lanes from trueValue or falseValue as
    * indicated
    */
  SIMD.int32x4.select = function(t, trueValue, falseValue) {
    t = SIMD.int32x4(t);
    trueValue = SIMD.int32x4(trueValue);
    falseValue = SIMD.int32x4(falseValue);
    var tr = SIMD.int32x4.and(t, trueValue);
    var fr = SIMD.int32x4.and(SIMD.int32x4.not(t), falseValue);
    return SIMD.int32x4.or(tr, fr);
  }
}

if (typeof SIMD.int32x4.withX === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {integer} 32-bit value used for x lane.
    * @return {int32x4} New instance of int32x4 with the values in t and
    * x lane replaced with {x}.
    */
  SIMD.int32x4.withX = function(t, x) {
    t = SIMD.int32x4(t);
    return SIMD.int32x4(x, t.y, t.z, t.w);
  }
}

if (typeof SIMD.int32x4.withY === "undefined") {
  /**
    * param {int32x4} t An instance of int32x4.
    * @param {integer} 32-bit value used for y lane.
    * @return {int32x4} New instance of int32x4 with the values in t and
    * y lane replaced with {y}.
    */
  SIMD.int32x4.withY = function(t, y) {
    t = SIMD.int32x4(t);
    return SIMD.int32x4(t.x, y, t.z, t.w);
  }
}

if (typeof SIMD.int32x4.withZ === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {integer} 32-bit value used for z lane.
    * @return {int32x4} New instance of int32x4 with the values in t and
    * z lane replaced with {z}.
    */
  SIMD.int32x4.withZ = function(t, z) {
    t = SIMD.int32x4(t);
    return SIMD.int32x4(t.x, t.y, z, t.w);
  }
}

if (typeof SIMD.int32x4.withW === "undefined") {
  /**
    * @param {integer} 32-bit value used for w lane.
    * @return {int32x4} New instance of int32x4 with the values in t and
    * w lane replaced with {w}.
    */
  SIMD.int32x4.withW = function(t, w) {
    t = SIMD.int32x4(t);
    return SIMD.int32x4(t.x, t.y, t.z, w);
  }
}

if (typeof SIMD.int32x4.withFlagX === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {boolean} x flag used for x lane.
    * @return {int32x4} New instance of int32x4 with the values in t and
    * x lane replaced with {x}.
    */
  SIMD.int32x4.withFlagX = function(t, flagX) {
    t = SIMD.int32x4(t);
    var x = flagX ? 0xFFFFFFFF : 0x0;
    return SIMD.int32x4(x, t.y, t.z, t.w);
  }
}

if (typeof SIMD.int32x4.withFlagY === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {boolean} y flag used for y lane.
    * @return {int32x4} New instance of int32x4 with the values in t and
    * y lane replaced with {y}.
    */
  SIMD.int32x4.withFlagY = function(t, flagY) {
    t = SIMD.int32x4(t);
    var y = flagY ? 0xFFFFFFFF : 0x0;
    return SIMD.int32x4(t.x, y, t.z, t.w);
  }
}

if (typeof SIMD.int32x4.withFlagZ === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {boolean} z flag used for z lane.
    * @return {int32x4} New instance of int32x4 with the values in t and
    * z lane replaced with {z}.
    */
  SIMD.int32x4.withFlagZ = function(t, flagZ) {
    t = SIMD.int32x4(t);
    var z = flagZ ? 0xFFFFFFFF : 0x0;
    return SIMD.int32x4(t.x, t.y, z, t.w);
  }
}

if (typeof SIMD.int32x4.withFlagW === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {boolean} w flag used for w lane.
    * @return {int32x4} New instance of int32x4 with the values in t and
    * w lane replaced with {w}.
    */
  SIMD.int32x4.withFlagW = function(t, flagW) {
    t = SIMD.int32x4(t);
    var w = flagW ? 0xFFFFFFFF : 0x0;
    return SIMD.int32x4(t.x, t.y, t.z, w);
  }
}

if (typeof SIMD.int32x4.equal === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {int32x4} other An instance of int32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t == other.
    */
  SIMD.int32x4.equal = function(t, other) {
    t = SIMD.int32x4(t);
    other = SIMD.int32x4(other);
    var cx = t.x == other.x;
    var cy = t.y == other.y;
    var cz = t.z == other.z;
    var cw = t.w == other.w;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.int32x4.notEqual === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {int32x4} other An instance of int32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t != other.
    */
  SIMD.int32x4.notEqual = function(t, other) {
    t = SIMD.int32x4(t);
    other = SIMD.int32x4(other);
    var cx = t.x != other.x;
    var cy = t.y != other.y;
    var cz = t.z != other.z;
    var cw = t.w != other.w;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.int32x4.greaterThan === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {int32x4} other An instance of int32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t > other.
    */
  SIMD.int32x4.greaterThan = function(t, other) {
    t = SIMD.int32x4(t);
    other = SIMD.int32x4(other);
    var cx = t.x > other.x;
    var cy = t.y > other.y;
    var cz = t.z > other.z;
    var cw = t.w > other.w;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.int32x4.greaterThanOrEqual === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {int32x4} other An instance of int32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t >= other.
    */
  SIMD.int32x4.greaterThanOrEqual = function(t, other) {
    t = SIMD.int32x4(t);
    other = SIMD.int32x4(other);
    var cx = t.x >= other.x;
    var cy = t.y >= other.y;
    var cz = t.z >= other.z;
    var cw = t.w >= other.w;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.int32x4.lessThan === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {int32x4} other An instance of int32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t < other.
    */
  SIMD.int32x4.lessThan = function(t, other) {
    t = SIMD.int32x4(t);
    other = SIMD.int32x4(other);
    var cx = t.x < other.x;
    var cy = t.y < other.y;
    var cz = t.z < other.z;
    var cw = t.w < other.w;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.int32x4.lessThanOrEqual === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {int32x4} other An instance of int32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t <= other.
    */
  SIMD.int32x4.lessThanOrEqual = function(t, other) {
    t = SIMD.int32x4(t);
    other = SIMD.int32x4(other);
    var cx = t.x <= other.x;
    var cy = t.y <= other.y;
    var cz = t.z <= other.z;
    var cw = t.w <= other.w;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.int32x4.unsignedGreaterThan === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {int32x4} other An instance of int32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t>>>0 > other>>>0.
    */
  SIMD.int32x4.unsignedGreaterThan = function(t, other) {
    t = SIMD.int32x4(t);
    other = SIMD.int32x4(other);
    var cx = t.x>>>0 > other.x>>>0;
    var cy = t.y>>>0 > other.y>>>0;
    var cz = t.z>>>0 > other.z>>>0;
    var cw = t.w>>>0 > other.w>>>0;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.int32x4.unsignedGreaterThanOrEqual === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {int32x4} other An instance of int32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t>>>0 >= other>>>0.
    */
  SIMD.int32x4.unsignedGreaterThanOrEqual = function(t, other) {
    t = SIMD.int32x4(t);
    other = SIMD.int32x4(other);
    var cx = t.x>>>0 >= other.x>>>0;
    var cy = t.y>>>0 >= other.y>>>0;
    var cz = t.z>>>0 >= other.z>>>0;
    var cw = t.w>>>0 >= other.w>>>0;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.int32x4.unsignedLessThan === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {int32x4} other An instance of int32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t>>>0 < other>>>0.
    */
  SIMD.int32x4.unsignedLessThan = function(t, other) {
    t = SIMD.int32x4(t);
    other = SIMD.int32x4(other);
    var cx = t.x>>>0 < other.x>>>0;
    var cy = t.y>>>0 < other.y>>>0;
    var cz = t.z>>>0 < other.z>>>0;
    var cw = t.w>>>0 < other.w>>>0;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.int32x4.unsignedLessThanOrEqual === "undefined") {
  /**
    * @param {int32x4} t An instance of int32x4.
    * @param {int32x4} other An instance of int32x4.
    * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
    * the result of t>>>0 <= other>>>0.
    */
  SIMD.int32x4.unsignedLessThanOrEqual = function(t, other) {
    t = SIMD.int32x4(t);
    other = SIMD.int32x4(other);
    var cx = t.x>>>0 <= other.x>>>0;
    var cy = t.y>>>0 <= other.y>>>0;
    var cz = t.z>>>0 <= other.z>>>0;
    var cw = t.w>>>0 <= other.w>>>0;
    return SIMD.int32x4.bool(cx, cy, cz, cw);
  }
}

if (typeof SIMD.int32x4.shiftLeftByScalar === "undefined") {
  /**
    * @param {int32x4} a An instance of int32x4.
    * @param {int} bits Bit count to shift by.
    * @return {int32x4} lanes in a shifted by bits.
    */
  SIMD.int32x4.shiftLeftByScalar = function(a, bits) {
    a = SIMD.int32x4(a);
    if (bits>>>0 >= 32)
      return SIMD.int32x4.splat(0.0);
    var x = a.x << bits;
    var y = a.y << bits;
    var z = a.z << bits;
    var w = a.w << bits;
    return SIMD.int32x4(x, y, z, w);
  }
}

if (typeof SIMD.int32x4.shiftRightLogicalByScalar === "undefined") {
  /**
    * @param {int32x4} a An instance of int32x4.
    * @param {int} bits Bit count to shift by.
    * @return {int32x4} lanes in a shifted by bits.
    */
  SIMD.int32x4.shiftRightLogicalByScalar = function(a, bits) {
    a = SIMD.int32x4(a);
    if (bits>>>0 >= 32)
      return SIMD.int32x4.splat(0.0);
    var x = a.x >>> bits;
    var y = a.y >>> bits;
    var z = a.z >>> bits;
    var w = a.w >>> bits;
    return SIMD.int32x4(x, y, z, w);
  }
}

if (typeof SIMD.int32x4.shiftRightArithmeticByScalar === "undefined") {
  /**
    * @param {int32x4} a An instance of int32x4.
    * @param {int} bits Bit count to shift by.
    * @return {int32x4} lanes in a shifted by bits.
    */
  SIMD.int32x4.shiftRightArithmeticByScalar = function(a, bits) {
    a = SIMD.int32x4(a);
    if (bits>>>0 >= 32)
      bits = 31;
    var x = a.x >> bits;
    var y = a.y >> bits;
    var z = a.z >> bits;
    var w = a.w >> bits;
    return SIMD.int32x4(x, y, z, w);
  }
}

if (typeof SIMD.int32x4.shiftLeft === "undefined") {
  /**
    * @param {int32x4} a An instance of int32x4.
    * @param {int32x4} bits An instance of int32x4.
    * @return {int32x4} lanes New instance of int32x4 with values of a shifted
    *                   left by bits.
    */
  SIMD.int32x4.shiftLeft = function(a, bits) {
    a = SIMD.int32x4(a);
    bits = SIMD.int32x4(bits);
    var x = bits.x>>>0 >= 32 ? 0 : a.x << bits.x;
    var y = bits.y>>>0 >= 32 ? 0 : a.y << bits.y;
    var z = bits.z>>>0 >= 32 ? 0 : a.z << bits.z;
    var w = bits.w>>>0 >= 32 ? 0 : a.w << bits.w;
    return SIMD.int32x4(x, y, z, w);
  }
}

if (typeof SIMD.int32x4.shiftRightLogical === "undefined") {
  /**
    * @param {int32x4} a An instance of int32x4.
    * @param {int32x4} bits An instance of int32x4.
    * @return {int32x4} lanes New instance of int32x4 with values of a
    *                   logically-shifted right by bits.
    */
  SIMD.int32x4.shiftRightLogical = function(a, bits) {
    a = SIMD.int32x4(a);
    bits = SIMD.int32x4(bits);
    var x = bits.x>>>0 >= 32 ? 0 : a.x >>> bits.x;
    var y = bits.y>>>0 >= 32 ? 0 : a.y >>> bits.y;
    var z = bits.z>>>0 >= 32 ? 0 : a.z >>> bits.z;
    var w = bits.w>>>0 >= 32 ? 0 : a.w >>> bits.w;
    return SIMD.int32x4(x, y, z, w);
  }
}

if (typeof SIMD.int32x4.shiftRightArithmetic === "undefined") {
  /**
    * @param {int32x4} a An instance of int32x4.
    * @param {int32x4} bits An instance of int32x4.
    * @return {int32x4} lanes New instance of int32x4 with values of a
    *                   arithmetic-shifted right by bits.
    */
  SIMD.int32x4.shiftRightArithmetic = function(a, bits) {
    a = SIMD.int32x4(a);
    bits = SIMD.int32x4(bits);
    var x = a.x >> (bits.x>>>0 > 31 ? 31 : bits.x);
    var y = a.y >> (bits.y>>>0 > 31 ? 31 : bits.y);
    var z = a.z >> (bits.z>>>0 > 31 ? 31 : bits.z);
    var w = a.w >> (bits.w>>>0 > 31 ? 31 : bits.w);
    return SIMD.int32x4(x, y, z, w);
  }
}

if (typeof SIMD.int32x4.load === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {int32x4} New instance of int32x4.
    */
  SIMD.int32x4.load = function(tarray, index) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var i32temp = _SIMD_PRIVATE._i32x4;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : i32temp) :
                _SIMD_PRIVATE._f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.int32x4(i32temp[0], i32temp[1], i32temp[2], i32temp[3]);
  }
}

if (typeof SIMD.int32x4.loadX === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {int32x4} New instance of int32x4.
    */
  SIMD.int32x4.loadX = function(tarray, index) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 4) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var i32temp = _SIMD_PRIVATE._i32x4;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : i32temp) :
                _SIMD_PRIVATE._f64x2;
    var n = 4 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.int32x4(i32temp[0], 0, 0, 0);
  }
}

if (typeof SIMD.int32x4.loadXY === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {int32x4} New instance of int32x4.
    */
  SIMD.int32x4.loadXY = function(tarray, index) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 8) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var i32temp = _SIMD_PRIVATE._i32x4;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : i32temp) :
                _SIMD_PRIVATE._f64x2;
    var n = 8 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.int32x4(i32temp[0], i32temp[1], 0, 0);
  }
}

if (typeof SIMD.int32x4.loadXYZ === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {int32x4} New instance of int32x4.
    */
  SIMD.int32x4.loadXYZ = function(tarray, index) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 12) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var i32temp = _SIMD_PRIVATE._i32x4;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : i32temp) :
                _SIMD_PRIVATE._f64x2;
    var n = 12 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.int32x4(i32temp[0], i32temp[1], i32temp[2], 0);
  }
}

if (typeof SIMD.int32x4.store === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {int32x4} value An instance of int32x4.
    * @return {void}
    */
  SIMD.int32x4.store = function(tarray, index, value) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.int32x4(value);
    _SIMD_PRIVATE._i32x4[0] = value.x;
    _SIMD_PRIVATE._i32x4[1] = value.y;
    _SIMD_PRIVATE._i32x4[2] = value.z;
    _SIMD_PRIVATE._i32x4[3] = value.w;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4) :
                _SIMD_PRIVATE._f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      tarray[index + i] = array[i];
  }
}

if (typeof SIMD.int32x4.storeX === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {int32x4} value An instance of int32x4.
    * @return {void}
    */
  SIMD.int32x4.storeX = function(tarray, index, value) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 4) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.int32x4(value);
    if (bpe == 8) {
      // tarray's elements are too wide. Just create a new view; this is rare.
      var view = new Int32Array(tarray.buffer, tarray.byteOffset + index * 8, 1);
      view[0] = value.x;
    } else {
      _SIMD_PRIVATE._i32x4[0] = value.x;
      var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                  bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                  (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4);
      var n = 4 / bpe;
      for (var i = 0; i < n; ++i)
        tarray[index + i] = array[i];
    }
  }
}

if (typeof SIMD.int32x4.storeXY === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {int32x4} value An instance of int32x4.
    * @return {void}
    */
  SIMD.int32x4.storeXY = function(tarray, index, value) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 8) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.int32x4(value);
    _SIMD_PRIVATE._i32x4[0] = value.x;
    _SIMD_PRIVATE._i32x4[1] = value.y;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4) :
                _SIMD_PRIVATE._f64x2;
    var n = 8 / bpe;
    for (var i = 0; i < n; ++i)
      tarray[index + i] = array[i];
  }
}

if (typeof SIMD.int32x4.storeXYZ === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {int32x4} value An instance of int32x4.
    * @return {void}
    */
  SIMD.int32x4.storeXYZ = function(tarray, index, value) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 12) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.int32x4(value);
    if (bpe == 8) {
      // tarray's elements are too wide. Just create a new view; this is rare.
      var view = new Int32Array(tarray.buffer, tarray.byteOffset + index * 8, 3);
      view[0] = value.x;
      view[1] = value.y;
      view[2] = value.z;
    } else {
      _SIMD_PRIVATE._i32x4[0] = value.x;
      _SIMD_PRIVATE._i32x4[1] = value.y;
      _SIMD_PRIVATE._i32x4[2] = value.z;
      var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                  bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                  (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4);
      var n = 12 / bpe;
      for (var i = 0; i < n; ++i)
        tarray[index + i] = array[i];
    }
  }
}

if (typeof SIMD.int16x8.and === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int16x8} b An instance of int16x8.
    * @return {int16x8} New instance of int16x8 with values of a & b.
    */
  SIMD.int16x8.and = function(a, b) {
    a = SIMD.int16x8(a);
    b = SIMD.int16x8(b);
    return SIMD.int16x8(a.s0 & b.s0, a.s1 & b.s1, a.s2 & b.s2, a.s3 & b.s3,
                        a.s4 & b.s4, a.s5 & b.s5, a.s6 & b.s6, a.s7 & b.s7);
  }
}

if (typeof SIMD.int16x8.or === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int16x8} b An instance of int16x8.
    * @return {int16x8} New instance of int16x8 with values of a | b.
    */
  SIMD.int16x8.or = function(a, b) {
    a = SIMD.int16x8(a);
    b = SIMD.int16x8(b);
    return SIMD.int16x8(a.s0 | b.s0, a.s1 | b.s1, a.s2 | b.s2, a.s3 | b.s3,
                        a.s4 | b.s4, a.s5 | b.s5, a.s6 | b.s6, a.s7 | b.s7);
  }
}

if (typeof SIMD.int16x8.xor === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int16x8} b An instance of int16x8.
    * @return {int16x8} New instance of int16x8 with values of a ^ b.
    */
  SIMD.int16x8.xor = function(a, b) {
    a = SIMD.int16x8(a);
    b = SIMD.int16x8(b);
    return SIMD.int16x8(a.s0 ^ b.s0, a.s1 ^ b.s1, a.s2 ^ b.s2, a.s3 ^ b.s3,
                        a.s4 ^ b.s4, a.s5 ^ b.s5, a.s6 ^ b.s6, a.s7 ^ b.s7);
  }
}

if (typeof SIMD.int16x8.not === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @return {int16x8} New instance of int16x8 with values of ~t
    */
  SIMD.int16x8.not = function(t) {
    t = SIMD.int16x8(t);
    return SIMD.int16x8(~t.s0, ~t.s1, ~t.s2, ~t.s3,
                        ~t.s4, ~t.s5, ~t.s6, ~t.s7);
  }
}

if (typeof SIMD.int16x8.neg === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @return {int16x8} New instance of int16x8 with values of -t
    */
  SIMD.int16x8.neg = function(t) {
    t = SIMD.int16x8(t);
    return SIMD.int16x8(-t.s0, -t.s1, -t.s2, -t.s3,
                        -t.s4, -t.s5, -t.s6, -t.s7);
  }
}

if (typeof SIMD.int16x8.add === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int16x8} b An instance of int16x8.
    * @return {int16x8} New instance of int16x8 with values of a + b.
    */
  SIMD.int16x8.add = function(a, b) {
    a = SIMD.int16x8(a);
    b = SIMD.int16x8(b);
    return SIMD.int16x8(a.s0 + b.s0, a.s1 + b.s1, a.s2 + b.s2, a.s3 + b.s3,
                        a.s4 + b.s4, a.s5 + b.s5, a.s6 + b.s6, a.s7 + b.s7);
  }
}

if (typeof SIMD.int16x8.sub === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int16x8} b An instance of int16x8.
    * @return {int16x8} New instance of int16x8 with values of a - b.
    */
  SIMD.int16x8.sub = function(a, b) {
    a = SIMD.int16x8(a);
    b = SIMD.int16x8(b);
    return SIMD.int16x8(a.s0 - b.s0, a.s1 - b.s1, a.s2 - b.s2, a.s3 - b.s3,
                        a.s4 - b.s4, a.s5 - b.s5, a.s6 - b.s6, a.s7 - b.s7);
  }
}

if (typeof SIMD.int16x8.mul === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int16x8} b An instance of int16x8.
    * @return {int16x8} New instance of int16x8 with values of a * b.
    */
  SIMD.int16x8.mul = function(a, b) {
    a = SIMD.int16x8(a);
    b = SIMD.int16x8(b);
    return SIMD.int16x8(Math.imul(a.s0, b.s0), Math.imul(a.s1, b.s1),
                        Math.imul(a.s2, b.s2), Math.imul(a.s3, b.s3),
                        Math.imul(a.s4, b.s4), Math.imul(a.s5, b.s5),
                        Math.imul(a.s6, b.s6), Math.imul(a.s7, b.s7));
  }
}

if (typeof SIMD.int16x8.select === "undefined") {
  /**
    * @param {int16x8} t Selector mask. An instance of int16x8
    * @param {int16x8} trueValue Pick lane from here if corresponding
    * selector lane is 0xFFFF
    * @param {int16x8} falseValue Pick lane from here if corresponding
    * selector lane is 0x0
    * @return {int16x8} Mix of lanes from trueValue or falseValue as
    * indicated
    */
  SIMD.int16x8.select = function(t, trueValue, falseValue) {
    t = SIMD.int16x8(t);
    trueValue = SIMD.int16x8(trueValue);
    falseValue = SIMD.int16x8(falseValue);
    var tr = SIMD.int16x8.and(t, trueValue);
    var fr = SIMD.int16x8.and(SIMD.int16x8.not(t), falseValue);
    return SIMD.int16x8.or(tr, fr);
  }
}

if (typeof SIMD.int16x8.equal === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @param {int16x8} other An instance of int16x8.
    * @return {int16x8} 0xFFFF or 0x0 in each lane depending on
    * the result of t == other.
    */
  SIMD.int16x8.equal = function(t, other) {
    t = SIMD.int16x8(t);
    other = SIMD.int16x8(other);
    var cs0 = t.s0 == other.s0;
    var cs1 = t.s1 == other.s1;
    var cs2 = t.s2 == other.s2;
    var cs3 = t.s3 == other.s3;
    var cs4 = t.s4 == other.s4;
    var cs5 = t.s5 == other.s5;
    var cs6 = t.s6 == other.s6;
    var cs7 = t.s7 == other.s7;
    return SIMD.int16x8.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.int16x8.notEqual === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @param {int16x8} other An instance of int16x8.
    * @return {int16x8} 0xFFFF or 0x0 in each lane depending on
    * the result of t != other.
    */
  SIMD.int16x8.notEqual = function(t, other) {
    t = SIMD.int16x8(t);
    other = SIMD.int16x8(other);
    var cs0 = t.s0 != other.s0;
    var cs1 = t.s1 != other.s1;
    var cs2 = t.s2 != other.s2;
    var cs3 = t.s3 != other.s3;
    var cs4 = t.s4 != other.s4;
    var cs5 = t.s5 != other.s5;
    var cs6 = t.s6 != other.s6;
    var cs7 = t.s7 != other.s7;
    return SIMD.int16x8.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.int16x8.greaterThan === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @param {int16x8} other An instance of int16x8.
    * @return {int16x8} 0xFFFF or 0x0 in each lane depending on
    * the result of t > other.
    */
  SIMD.int16x8.greaterThan = function(t, other) {
    t = SIMD.int16x8(t);
    other = SIMD.int16x8(other);
    var cs0 = t.s0 > other.s0;
    var cs1 = t.s1 > other.s1;
    var cs2 = t.s2 > other.s2;
    var cs3 = t.s3 > other.s3;
    var cs4 = t.s4 > other.s4;
    var cs5 = t.s5 > other.s5;
    var cs6 = t.s6 > other.s6;
    var cs7 = t.s7 > other.s7;
    return SIMD.int16x8.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.int16x8.greaterThanOrEqual === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @param {int16x8} other An instance of int16x8.
    * @return {int16x8} 0xFFFF or 0x0 in each lane depending on
    * the result of t >= other.
    */
  SIMD.int16x8.greaterThanOrEqual = function(t, other) {
    t = SIMD.int16x8(t);
    other = SIMD.int16x8(other);
    var cs0 = t.s0 >= other.s0;
    var cs1 = t.s1 >= other.s1;
    var cs2 = t.s2 >= other.s2;
    var cs3 = t.s3 >= other.s3;
    var cs4 = t.s4 >= other.s4;
    var cs5 = t.s5 >= other.s5;
    var cs6 = t.s6 >= other.s6;
    var cs7 = t.s7 >= other.s7;
    return SIMD.int16x8.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.int16x8.lessThan === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @param {int16x8} other An instance of int16x8.
    * @return {int16x8} 0xFFFF or 0x0 in each lane depending on
    * the result of t < other.
    */
  SIMD.int16x8.lessThan = function(t, other) {
    t = SIMD.int16x8(t);
    other = SIMD.int16x8(other);
    var cs0 = t.s0 < other.s0;
    var cs1 = t.s1 < other.s1;
    var cs2 = t.s2 < other.s2;
    var cs3 = t.s3 < other.s3;
    var cs4 = t.s4 < other.s4;
    var cs5 = t.s5 < other.s5;
    var cs6 = t.s6 < other.s6;
    var cs7 = t.s7 < other.s7;
    return SIMD.int16x8.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.int16x8.lessThanOrEqual === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @param {int16x8} other An instance of int16x8.
    * @return {int16x8} 0xFFFF or 0x0 in each lane depending on
    * the result of t <= other.
    */
  SIMD.int16x8.lessThanOrEqual = function(t, other) {
    t = SIMD.int16x8(t);
    other = SIMD.int16x8(other);
    var cs0 = t.s0 <= other.s0;
    var cs1 = t.s1 <= other.s1;
    var cs2 = t.s2 <= other.s2;
    var cs3 = t.s3 <= other.s3;
    var cs4 = t.s4 <= other.s4;
    var cs5 = t.s5 <= other.s5;
    var cs6 = t.s6 <= other.s6;
    var cs7 = t.s7 <= other.s7;
    return SIMD.int16x8.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.int16x8.unsignedGreaterThan === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @param {int16x8} other An instance of int16x8.
    * @return {int16x8} 0xFFFF or 0x0 in each lane depending on
    * the result of t>>>0 > other>>>0.
    */
  SIMD.int16x8.unsignedGreaterThan = function(t, other) {
    t = SIMD.int16x8(t);
    other = SIMD.int16x8(other);
    var cs0 = t.s0>>>0 > other.s0>>>0;
    var cs1 = t.s1>>>0 > other.s1>>>0;
    var cs2 = t.s2>>>0 > other.s2>>>0;
    var cs3 = t.s3>>>0 > other.s3>>>0;
    var cs4 = t.s4>>>0 > other.s4>>>0;
    var cs5 = t.s5>>>0 > other.s5>>>0;
    var cs6 = t.s6>>>0 > other.s6>>>0;
    var cs7 = t.s7>>>0 > other.s7>>>0;
    return SIMD.int16x8.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.int16x8.unsignedGreaterThanOrEqual === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @param {int16x8} other An instance of int16x8.
    * @return {int16x8} 0xFFFF or 0x0 in each lane depending on
    * the result of t>>>0 >= other>>>0.
    */
  SIMD.int16x8.unsignedGreaterThanOrEqual = function(t, other) {
    t = SIMD.int16x8(t);
    other = SIMD.int16x8(other);
    var cs0 = t.s0>>>0 >= other.s0>>>0;
    var cs1 = t.s1>>>0 >= other.s1>>>0;
    var cs2 = t.s2>>>0 >= other.s2>>>0;
    var cs3 = t.s3>>>0 >= other.s3>>>0;
    var cs4 = t.s4>>>0 >= other.s4>>>0;
    var cs5 = t.s5>>>0 >= other.s5>>>0;
    var cs6 = t.s6>>>0 >= other.s6>>>0;
    var cs7 = t.s7>>>0 >= other.s7>>>0;
    return SIMD.int16x8.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.int16x8.unsignedLessThan === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @param {int16x8} other An instance of int16x8.
    * @return {int16x8} 0xFFFF or 0x0 in each lane depending on
    * the result of t>>>0 < other>>>0.
    */
  SIMD.int16x8.unsignedLessThan = function(t, other) {
    t = SIMD.int16x8(t);
    other = SIMD.int16x8(other);
    var cs0 = t.s0>>>0 < other.s0>>>0;
    var cs1 = t.s1>>>0 < other.s1>>>0;
    var cs2 = t.s2>>>0 < other.s2>>>0;
    var cs3 = t.s3>>>0 < other.s3>>>0;
    var cs4 = t.s4>>>0 < other.s4>>>0;
    var cs5 = t.s5>>>0 < other.s5>>>0;
    var cs6 = t.s6>>>0 < other.s6>>>0;
    var cs7 = t.s7>>>0 < other.s7>>>0;
    return SIMD.int16x8.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.int16x8.unsignedLessThanOrEqual === "undefined") {
  /**
    * @param {int16x8} t An instance of int16x8.
    * @param {int16x8} other An instance of int16x8.
    * @return {int16x8} 0xFFFF or 0x0 in each lane depending on
    * the result of t>>>0 <= other>>>0.
    */
  SIMD.int16x8.unsignedLessThanOrEqual = function(t, other) {
    t = SIMD.int16x8(t);
    other = SIMD.int16x8(other);
    var cs0 = t.s0>>>0 <= other.s0>>>0;
    var cs1 = t.s1>>>0 <= other.s1>>>0;
    var cs2 = t.s2>>>0 <= other.s2>>>0;
    var cs3 = t.s3>>>0 <= other.s3>>>0;
    var cs4 = t.s4>>>0 <= other.s4>>>0;
    var cs5 = t.s5>>>0 <= other.s5>>>0;
    var cs6 = t.s6>>>0 <= other.s6>>>0;
    var cs7 = t.s7>>>0 <= other.s7>>>0;
    return SIMD.int16x8.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7);
  }
}

if (typeof SIMD.int16x8.shiftLeftByScalar === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int} bits Bit count to shift by.
    * @return {int16x8} lanes in a shifted by bits.
    */
  SIMD.int16x8.shiftLeftByScalar = function(a, bits) {
    a = SIMD.int16x8(a);
    if (bits>>>0 > 16)
      bits = 16;
    var s0 = a.s0 << bits;
    var s1 = a.s1 << bits;
    var s2 = a.s2 << bits;
    var s3 = a.s3 << bits;
    var s4 = a.s4 << bits;
    var s5 = a.s5 << bits;
    var s6 = a.s6 << bits;
    var s7 = a.s7 << bits;
    return SIMD.int16x8(s0, s1, s2, s3, s4, s5, s6, s7);
  }
}

if (typeof SIMD.int16x8.shiftRightLogicalByScalar === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int} bits Bit count to shift by.
    * @return {int16x8} lanes in a shifted by bits.
    */
  SIMD.int16x8.shiftRightLogicalByScalar = function(a, bits) {
    a = SIMD.int16x8(a);
    if (bits>>>0 > 16)
      bits = 16;
    var s0 = a.s0 >>> bits;
    var s1 = a.s1 >>> bits;
    var s2 = a.s2 >>> bits;
    var s3 = a.s3 >>> bits;
    var s4 = a.s4 >>> bits;
    var s5 = a.s5 >>> bits;
    var s6 = a.s6 >>> bits;
    var s7 = a.s7 >>> bits;
    return SIMD.int16x8(s0, s1, s2, s3, s4, s5, s6, s7);
  }
}

if (typeof SIMD.int16x8.shiftRightArithmeticByScalar === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int} bits Bit count to shift by.
    * @return {int16x8} lanes in a shifted by bits.
    */
  SIMD.int16x8.shiftRightArithmeticByScalar = function(a, bits) {
    a = SIMD.int16x8(a);
    if (bits>>>0 > 16)
      bits = 16;
    var s0 = a.s0 >> bits;
    var s1 = a.s1 >> bits;
    var s2 = a.s2 >> bits;
    var s3 = a.s3 >> bits;
    var s4 = a.s4 >> bits;
    var s5 = a.s5 >> bits;
    var s6 = a.s6 >> bits;
    var s7 = a.s7 >> bits;
    return SIMD.int16x8(s0, s1, s2, s3, s4, s5, s6, s7);
  }
}

if (typeof SIMD.int16x8.shiftLeft === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int16x8} bits An instance of int16x8.
    * @return {int16x8} lanes New instance of int16x8 with values of a shifted
    *                   left by bits.
    */
  SIMD.int16x8.shiftLeft = function(a, bits) {
    a = SIMD.int16x8(a);
    bits = SIMD.int16x8(bits);
    var s0 = bits.s0>>>0 >= 32 ? 0 : a.s0 << bits.s0;
    var s1 = bits.s1>>>0 >= 32 ? 0 : a.s1 << bits.s1;
    var s2 = bits.s2>>>0 >= 32 ? 0 : a.s2 << bits.s2;
    var s3 = bits.s3>>>0 >= 32 ? 0 : a.s3 << bits.s3;
    var s4 = bits.s4>>>0 >= 32 ? 0 : a.s4 << bits.s4;
    var s5 = bits.s5>>>0 >= 32 ? 0 : a.s5 << bits.s5;
    var s6 = bits.s6>>>0 >= 32 ? 0 : a.s6 << bits.s6;
    var s7 = bits.s7>>>0 >= 32 ? 0 : a.s7 << bits.s7;
    return SIMD.int16x8(s0, s1, s2, s3, s4, s5, s6, s7);
  }
}

if (typeof SIMD.int16x8.shiftRightLogical === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int16x8} bits An instance of int16x8.
    * @return {int16x8} lanes New instance of int16x8 with values of a
    *                   logically-shifted right by bits.
    */
  SIMD.int16x8.shiftRightLogical = function(a, bits) {
    a = SIMD.int16x8(a);
    bits = SIMD.int16x8(bits);
    var s0 = bits.s0>>>0 >= 32 ? 0 : a.s0 >>> bits.s0;
    var s1 = bits.s1>>>0 >= 32 ? 0 : a.s1 >>> bits.s1;
    var s2 = bits.s2>>>0 >= 32 ? 0 : a.s2 >>> bits.s2;
    var s3 = bits.s3>>>0 >= 32 ? 0 : a.s3 >>> bits.s3;
    var s4 = bits.s4>>>0 >= 32 ? 0 : a.s4 >>> bits.s4;
    var s5 = bits.s5>>>0 >= 32 ? 0 : a.s5 >>> bits.s5;
    var s5 = bits.s6>>>0 >= 32 ? 0 : a.s6 >>> bits.s6;
    var s6 = bits.s7>>>0 >= 32 ? 0 : a.s7 >>> bits.s7;
    return SIMD.int16x8(s0, s1, s2, s3, s4, s5, s6, s7);
  }
}

if (typeof SIMD.int16x8.shiftRightArithmetic === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int16x8} bits An instance of int16x8.
    * @return {int16x8} lanes New instance of int16x8 with values of a
    *                   arithmetic-shifted right by bits.
    */
  SIMD.int16x8.shiftRightArithmetic = function(a, bits) {
    a = SIMD.int16x8(a);
    bits = SIMD.int16x8(bits);
    var s0 = a.s0 >> (bits.s0>>>0 > 31 ? 31 : bits.s0);
    var s1 = a.s1 >> (bits.s1>>>0 > 31 ? 31 : bits.s1);
    var s2 = a.s2 >> (bits.s2>>>0 > 31 ? 31 : bits.s2);
    var s3 = a.s3 >> (bits.s3>>>0 > 31 ? 31 : bits.s3);
    var s4 = a.s4 >> (bits.s4>>>0 > 31 ? 31 : bits.s4);
    var s5 = a.s5 >> (bits.s5>>>0 > 31 ? 31 : bits.s5);
    var s6 = a.s6 >> (bits.s6>>>0 > 31 ? 31 : bits.s6);
    var s7 = a.s7 >> (bits.s7>>>0 > 31 ? 31 : bits.s7);
    return SIMD.int16x8(s0, s1, s2, s3, s4, s5, s6, s7);
  }
}

if (typeof SIMD.int16x8.load === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {int16x8} New instance of int16x8.
    */
  SIMD.int16x8.load = function(tarray, index) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var i16temp = _SIMD_PRIVATE._i16x8;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? i16temp :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4) :
                _SIMD_PRIVATE._f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.int16x8(i16temp[0], i16temp[1], i16temp[2], i16temp[3],
                        i16temp[4], i16temp[5], i16temp[6], i16temp[7]);
  }
}

if (typeof SIMD.int16x8.store === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {int16x8} value An instance of int16x8.
    * @return {void}
    */
  SIMD.int16x8.store = function(tarray, index, value) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.int16x8(value);
    _SIMD_PRIVATE._i16x8[0] = value.s0;
    _SIMD_PRIVATE._i16x8[1] = value.s1;
    _SIMD_PRIVATE._i16x8[2] = value.s2;
    _SIMD_PRIVATE._i16x8[3] = value.s3;
    _SIMD_PRIVATE._i16x8[4] = value.s4;
    _SIMD_PRIVATE._i16x8[5] = value.s5;
    _SIMD_PRIVATE._i16x8[6] = value.s6;
    _SIMD_PRIVATE._i16x8[7] = value.s7;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4) :
                _SIMD_PRIVATE._f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      tarray[index + i] = array[i];
  }
}

if (typeof SIMD.int16x8.saturatingAdd === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int16x8} b An instance of int16x8.
    * @return {int16x8} New instance of int16x8 with values of
    * Math.max(Math.min(a + b, 32767), -32768).
    */
  SIMD.int16x8.saturatingAdd = function(a, b) {
    a = SIMD.int16x8(a);
    b = SIMD.int16x8(b);
    return SIMD.int16x8(Math.max(Math.min(a.s0 + b.s0, 32767), -32768),
                        Math.max(Math.min(a.s1 + b.s1, 32767), -32768),
                        Math.max(Math.min(a.s2 + b.s2, 32767), -32768),
                        Math.max(Math.min(a.s3 + b.s3, 32767), -32768),
                        Math.max(Math.min(a.s4 + b.s4, 32767), -32768),
                        Math.max(Math.min(a.s5 + b.s5, 32767), -32768),
                        Math.max(Math.min(a.s6 + b.s6, 32767), -32768),
                        Math.max(Math.min(a.s7 + b.s7, 32767), -32768));
  }
}

if (typeof SIMD.int16x8.saturatingSub === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int16x8} b An instance of int16x8.
    * @return {int16x8} New instance of int16x8 with values of
    * Math.max(Math.min(a - b, 32767), -32768).
    */
  SIMD.int16x8.saturatingSub = function(a, b) {
    a = SIMD.int16x8(a);
    b = SIMD.int16x8(b);
    return SIMD.int16x8(Math.max(Math.min(a.s0 - b.s0, 32767), -32768),
                        Math.max(Math.min(a.s1 - b.s1, 32767), -32768),
                        Math.max(Math.min(a.s2 - b.s2, 32767), -32768),
                        Math.max(Math.min(a.s3 - b.s3, 32767), -32768),
                        Math.max(Math.min(a.s4 - b.s4, 32767), -32768),
                        Math.max(Math.min(a.s5 - b.s5, 32767), -32768),
                        Math.max(Math.min(a.s6 - b.s6, 32767), -32768),
                        Math.max(Math.min(a.s7 - b.s7, 32767), -32768));
  }
}

if (typeof SIMD.int16x8.unsignedSaturatingAdd === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int16x8} b An instance of int16x8.
    * @return {int16x8} New instance of int16x8 with values of
    * Math.max(Math.min(a + b, 65535), 0).
    */
  SIMD.int16x8.unsignedSaturatingAdd = function(a, b) {
    a = SIMD.int16x8(a);
    b = SIMD.int16x8(b);
    return SIMD.int16x8(Math.max(Math.min(a.s0 + b.s0, 65535), 0),
                        Math.max(Math.min(a.s1 + b.s1, 65535), 0),
                        Math.max(Math.min(a.s2 + b.s2, 65535), 0),
                        Math.max(Math.min(a.s3 + b.s3, 65535), 0),
                        Math.max(Math.min(a.s4 + b.s4, 65535), 0),
                        Math.max(Math.min(a.s5 + b.s5, 65535), 0),
                        Math.max(Math.min(a.s6 + b.s6, 65535), 0),
                        Math.max(Math.min(a.s7 + b.s7, 65535), 0));
  }
}

if (typeof SIMD.int16x8.unsignedSaturatingSub === "undefined") {
  /**
    * @param {int16x8} a An instance of int16x8.
    * @param {int16x8} b An instance of int16x8.
    * @return {int16x8} New instance of int16x8 with values of
    * Math.max(Math.min(a - b, 65535), 0).
    */
  SIMD.int16x8.unsignedSaturatingSub = function(a, b) {
    a = SIMD.int16x8(a);
    b = SIMD.int16x8(b);
    return SIMD.int16x8(Math.max(Math.min(a.s0 - b.s0, 65535), 0),
                        Math.max(Math.min(a.s1 - b.s1, 65535), 0),
                        Math.max(Math.min(a.s2 - b.s2, 65535), 0),
                        Math.max(Math.min(a.s3 - b.s3, 65535), 0),
                        Math.max(Math.min(a.s4 - b.s4, 65535), 0),
                        Math.max(Math.min(a.s5 - b.s5, 65535), 0),
                        Math.max(Math.min(a.s6 - b.s6, 65535), 0),
                        Math.max(Math.min(a.s7 - b.s7, 65535), 0));
  }
}

if (typeof SIMD.int8x16.and === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int8x16} b An instance of int8x16.
    * @return {int8x16} New instance of int8x16 with values of a & b.
    */
  SIMD.int8x16.and = function(a, b) {
    a = SIMD.int8x16(a);
    b = SIMD.int8x16(b);
    return SIMD.int8x16(a.s0 & b.s0, a.s1 & b.s1, a.s2 & b.s2, a.s3 & b.s3,
                        a.s4 & b.s4, a.s5 & b.s5, a.s6 & b.s6, a.s7 & b.s7,
                        a.s8 & b.s8, a.s9 & b.s9, a.s10 & b.s10, a.s11 & b.s11,
                        a.s12 & b.s12, a.s13 & b.s13, a.s14 & b.s14, a.s15 & b.s15);
  }
}

if (typeof SIMD.int8x16.or === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int8x16} b An instance of int8x16.
    * @return {int8x16} New instance of int8x16 with values of a | b.
    */
  SIMD.int8x16.or = function(a, b) {
    a = SIMD.int8x16(a);
    b = SIMD.int8x16(b);
    return SIMD.int8x16(a.s0 | b.s0, a.s1 | b.s1, a.s2 | b.s2, a.s3 | b.s3,
                        a.s4 | b.s4, a.s5 | b.s5, a.s6 | b.s6, a.s7 | b.s7,
                        a.s8 | b.s8, a.s9 | b.s9, a.s10 | b.s10, a.s11 | b.s11,
                        a.s12 | b.s12, a.s13 | b.s13, a.s14 | b.s14, a.s15 | b.s15);
  }
}

if (typeof SIMD.int8x16.xor === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int8x16} b An instance of int8x16.
    * @return {int8x16} New instance of int8x16 with values of a ^ b.
    */
  SIMD.int8x16.xor = function(a, b) {
    a = SIMD.int8x16(a);
    b = SIMD.int8x16(b);
    return SIMD.int8x16(a.s0 ^ b.s0, a.s1 ^ b.s1, a.s2 ^ b.s2, a.s3 ^ b.s3,
                        a.s4 ^ b.s4, a.s5 ^ b.s5, a.s6 ^ b.s6, a.s7 ^ b.s7,
                        a.s8 ^ b.s8, a.s9 ^ b.s9, a.s10 ^ b.s10, a.s11 ^ b.s11,
                        a.s12 ^ b.s12, a.s13 ^ b.s13, a.s14 ^ b.s14, a.s15 ^ b.s15);
  }
}

if (typeof SIMD.int8x16.not === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @return {int8x16} New instance of int8x16 with values of ~t
    */
  SIMD.int8x16.not = function(t) {
    t = SIMD.int8x16(t);
    return SIMD.int8x16(~t.s0, ~t.s1, ~t.s2, ~t.s3,
                        ~t.s4, ~t.s5, ~t.s6, ~t.s7,
                        ~t.s8, ~t.s9, ~t.s10, ~t.s11,
                        ~t.s12, ~t.s13, ~t.s14, ~t.s15);
  }
}

if (typeof SIMD.int8x16.neg === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @return {int8x16} New instance of int8x16 with values of -t
    */
  SIMD.int8x16.neg = function(t) {
    t = SIMD.int8x16(t);
    return SIMD.int8x16(-t.s0, -t.s1, -t.s2, -t.s3,
                        -t.s4, -t.s5, -t.s6, -t.s7,
                        -t.s8, -t.s9, -t.s10, -t.s11,
                        -t.s12, -t.s13, -t.s14, -t.s15);
  }
}

if (typeof SIMD.int8x16.add === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int8x16} b An instance of int8x16.
    * @return {int8x16} New instance of int8x16 with values of a + b.
    */
  SIMD.int8x16.add = function(a, b) {
    a = SIMD.int8x16(a);
    b = SIMD.int8x16(b);
    return SIMD.int8x16(a.s0 + b.s0, a.s1 + b.s1, a.s2 + b.s2, a.s3 + b.s3,
                        a.s4 + b.s4, a.s5 + b.s5, a.s6 + b.s6, a.s7 + b.s7,
                        a.s8 + b.s8, a.s9 + b.s9, a.s10 + b.s10, a.s11 + b.s11,
                        a.s12 + b.s12, a.s13 + b.s13, a.s14 + b.s14, a.s15 + b.s15);
  }
}

if (typeof SIMD.int8x16.sub === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int8x16} b An instance of int8x16.
    * @return {int8x16} New instance of int8x16 with values of a - b.
    */
  SIMD.int8x16.sub = function(a, b) {
    a = SIMD.int8x16(a);
    b = SIMD.int8x16(b);
    return SIMD.int8x16(a.s0 - b.s0, a.s1 - b.s1, a.s2 - b.s2, a.s3 - b.s3,
                        a.s4 - b.s4, a.s5 - b.s5, a.s6 - b.s6, a.s7 - b.s7,
                        a.s8 - b.s8, a.s9 - b.s9, a.s10 - b.s10, a.s11 - b.s11,
                        a.s12 - b.s12, a.s13 - b.s13, a.s14 - b.s14, a.s15 - b.s15);
  }
}

if (typeof SIMD.int8x16.mul === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int8x16} b An instance of int8x16.
    * @return {int8x16} New instance of int8x16 with values of a * b.
    */
  SIMD.int8x16.mul = function(a, b) {
    a = SIMD.int8x16(a);
    b = SIMD.int8x16(b);
    return SIMD.int8x16(Math.imul(a.s0, b.s0), Math.imul(a.s1, b.s1),
                        Math.imul(a.s2, b.s2), Math.imul(a.s3, b.s3),
                        Math.imul(a.s4, b.s4), Math.imul(a.s5, b.s5),
                        Math.imul(a.s6, b.s6), Math.imul(a.s7, b.s7),
                        Math.imul(a.s8, b.s8), Math.imul(a.s9, b.s9),
                        Math.imul(a.s10, b.s10), Math.imul(a.s11, b.s11),
                        Math.imul(a.s12, b.s12), Math.imul(a.s13, b.s13),
                        Math.imul(a.s14, b.s14), Math.imul(a.s15, b.s15));
  }
}

if (typeof SIMD.int8x16.select === "undefined") {
  /**
    * @param {int8x16} t Selector mask. An instance of int8x16
    * @param {int8x16} trueValue Pick lane from here if corresponding
    * selector lane is 0xFF
    * @param {int8x16} falseValue Pick lane from here if corresponding
    * selector lane is 0x0
    * @return {int8x16} Mix of lanes from trueValue or falseValue as
    * indicated
    */
  SIMD.int8x16.select = function(t, trueValue, falseValue) {
    t = SIMD.int8x16(t);
    trueValue = SIMD.int8x16(trueValue);
    falseValue = SIMD.int8x16(falseValue);
    var tr = SIMD.int8x16.and(t, trueValue);
    var fr = SIMD.int8x16.and(SIMD.int8x16.not(t), falseValue);
    return SIMD.int8x16.or(tr, fr);
  }
}

if (typeof SIMD.int8x16.equal === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @param {int8x16} other An instance of int8x16.
    * @return {int8x16} 0xFF or 0x0 in each lane depending on
    * the result of t == other.
    */
  SIMD.int8x16.equal = function(t, other) {
    t = SIMD.int8x16(t);
    other = SIMD.int8x16(other);
    var cs0 = t.s0 == other.s0;
    var cs1 = t.s1 == other.s1;
    var cs2 = t.s2 == other.s2;
    var cs3 = t.s3 == other.s3;
    var cs4 = t.s4 == other.s4;
    var cs5 = t.s5 == other.s5;
    var cs6 = t.s6 == other.s6;
    var cs7 = t.s7 == other.s7;
    var cs8 = t.s8 == other.s8;
    var cs9 = t.s9 == other.s9;
    var cs10 = t.s10 == other.s10;
    var cs11 = t.s11 == other.s11;
    var cs12 = t.s12 == other.s12;
    var cs13 = t.s13 == other.s13;
    var cs14 = t.s14 == other.s14;
    var cs15 = t.s15 == other.s15;
    return SIMD.int8x16.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                             cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.int8x16.notEqual === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @param {int8x16} other An instance of int8x16.
    * @return {int8x16} 0xFF or 0x0 in each lane depending on
    * the result of t != other.
    */
  SIMD.int8x16.notEqual = function(t, other) {
    t = SIMD.int8x16(t);
    other = SIMD.int8x16(other);
    var cs0 = t.s0 != other.s0;
    var cs1 = t.s1 != other.s1;
    var cs2 = t.s2 != other.s2;
    var cs3 = t.s3 != other.s3;
    var cs4 = t.s4 != other.s4;
    var cs5 = t.s5 != other.s5;
    var cs6 = t.s6 != other.s6;
    var cs7 = t.s7 != other.s7;
    var cs8 = t.s8 != other.s8;
    var cs9 = t.s9 != other.s9;
    var cs10 = t.s10 != other.s10;
    var cs11 = t.s11 != other.s11;
    var cs12 = t.s12 != other.s12;
    var cs13 = t.s13 != other.s13;
    var cs14 = t.s14 != other.s14;
    var cs15 = t.s15 != other.s15;
    return SIMD.int8x16.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                             cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.int8x16.greaterThan === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @param {int8x16} other An instance of int8x16.
    * @return {int8x16} 0xFF or 0x0 in each lane depending on
    * the result of t > other.
    */
  SIMD.int8x16.greaterThan = function(t, other) {
    t = SIMD.int8x16(t);
    other = SIMD.int8x16(other);
    var cs0 = t.s0 > other.s0;
    var cs1 = t.s1 > other.s1;
    var cs2 = t.s2 > other.s2;
    var cs3 = t.s3 > other.s3;
    var cs4 = t.s4 > other.s4;
    var cs5 = t.s5 > other.s5;
    var cs6 = t.s6 > other.s6;
    var cs7 = t.s7 > other.s7;
    var cs8 = t.s8 > other.s8;
    var cs9 = t.s9 > other.s9;
    var cs10 = t.s10 > other.s10;
    var cs11 = t.s11 > other.s11;
    var cs12 = t.s12 > other.s12;
    var cs13 = t.s13 > other.s13;
    var cs14 = t.s14 > other.s14;
    var cs15 = t.s15 > other.s15;
    return SIMD.int8x16.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                             cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.int8x16.greaterThanOrEqual === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @param {int8x16} other An instance of int8x16.
    * @return {int8x16} 0xFF or 0x0 in each lane depending on
    * the result of t >= other.
    */
  SIMD.int8x16.greaterThanOrEqual = function(t, other) {
    t = SIMD.int8x16(t);
    other = SIMD.int8x16(other);
    var cs0 = t.s0 >= other.s0;
    var cs1 = t.s1 >= other.s1;
    var cs2 = t.s2 >= other.s2;
    var cs3 = t.s3 >= other.s3;
    var cs4 = t.s4 >= other.s4;
    var cs5 = t.s5 >= other.s5;
    var cs6 = t.s6 >= other.s6;
    var cs7 = t.s7 >= other.s7;
    var cs8 = t.s8 >= other.s8;
    var cs9 = t.s9 >= other.s9;
    var cs10 = t.s10 >= other.s10;
    var cs11 = t.s11 >= other.s11;
    var cs12 = t.s12 >= other.s12;
    var cs13 = t.s13 >= other.s13;
    var cs14 = t.s14 >= other.s14;
    var cs15 = t.s15 >= other.s15;
    return SIMD.int8x16.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                             cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.int8x16.lessThan === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @param {int8x16} other An instance of int8x16.
    * @return {int8x16} 0xFF or 0x0 in each lane depending on
    * the result of t < other.
    */
  SIMD.int8x16.lessThan = function(t, other) {
    t = SIMD.int8x16(t);
    other = SIMD.int8x16(other);
    var cs0 = t.s0 < other.s0;
    var cs1 = t.s1 < other.s1;
    var cs2 = t.s2 < other.s2;
    var cs3 = t.s3 < other.s3;
    var cs4 = t.s4 < other.s4;
    var cs5 = t.s5 < other.s5;
    var cs6 = t.s6 < other.s6;
    var cs7 = t.s7 < other.s7;
    var cs8 = t.s8 < other.s8;
    var cs9 = t.s9 < other.s9;
    var cs10 = t.s10 < other.s10;
    var cs11 = t.s11 < other.s11;
    var cs12 = t.s12 < other.s12;
    var cs13 = t.s13 < other.s13;
    var cs14 = t.s14 < other.s14;
    var cs15 = t.s15 < other.s15;
    return SIMD.int8x16.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                             cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.int8x16.lessThanOrEqual === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @param {int8x16} other An instance of int8x16.
    * @return {int8x16} 0xFF or 0x0 in each lane depending on
    * the result of t <= other.
    */
  SIMD.int8x16.lessThanOrEqual = function(t, other) {
    t = SIMD.int8x16(t);
    other = SIMD.int8x16(other);
    var cs0 = t.s0 <= other.s0;
    var cs1 = t.s1 <= other.s1;
    var cs2 = t.s2 <= other.s2;
    var cs3 = t.s3 <= other.s3;
    var cs4 = t.s4 <= other.s4;
    var cs5 = t.s5 <= other.s5;
    var cs6 = t.s6 <= other.s6;
    var cs7 = t.s7 <= other.s7;
    var cs8 = t.s8 <= other.s8;
    var cs9 = t.s9 <= other.s9;
    var cs10 = t.s10 <= other.s10;
    var cs11 = t.s11 <= other.s11;
    var cs12 = t.s12 <= other.s12;
    var cs13 = t.s13 <= other.s13;
    var cs14 = t.s14 <= other.s14;
    var cs15 = t.s15 <= other.s15;
    return SIMD.int8x16.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                             cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.int8x16.unsignedGreaterThan === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @param {int8x16} other An instance of int8x16.
    * @return {int8x16} 0xFF or 0x0 in each lane depending on
    * the result of t>>>0 > other>>>0.
    */
  SIMD.int8x16.unsignedGreaterThan = function(t, other) {
    t = SIMD.int8x16(t);
    other = SIMD.int8x16(other);
    var cs0 = t.s0>>>0 > other.s0>>>0;
    var cs1 = t.s1>>>0 > other.s1>>>0;
    var cs2 = t.s2>>>0 > other.s2>>>0;
    var cs3 = t.s3>>>0 > other.s3>>>0;
    var cs4 = t.s4>>>0 > other.s4>>>0;
    var cs5 = t.s5>>>0 > other.s5>>>0;
    var cs6 = t.s6>>>0 > other.s6>>>0;
    var cs7 = t.s7>>>0 > other.s7>>>0;
    var cs8 = t.s8>>>0 > other.s8>>>0;
    var cs9 = t.s9>>>0 > other.s9>>>0;
    var cs10 = t.s10>>>0 > other.s10>>>0;
    var cs11 = t.s11>>>0 > other.s11>>>0;
    var cs12 = t.s12>>>0 > other.s12>>>0;
    var cs13 = t.s13>>>0 > other.s13>>>0;
    var cs14 = t.s14>>>0 > other.s14>>>0;
    var cs15 = t.s15>>>0 > other.s15>>>0;
    return SIMD.int8x16.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                             cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.int8x16.unsignedGreaterThanOrEqual === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @param {int8x16} other An instance of int8x16.
    * @return {int8x16} 0xFF or 0x0 in each lane depending on
    * the result of t>>>0 >= other>>>0.
    */
  SIMD.int8x16.unsignedGreaterThanOrEqual = function(t, other) {
    t = SIMD.int8x16(t);
    other = SIMD.int8x16(other);
    var cs0 = t.s0>>>0 >= other.s0>>>0;
    var cs1 = t.s1>>>0 >= other.s1>>>0;
    var cs2 = t.s2>>>0 >= other.s2>>>0;
    var cs3 = t.s3>>>0 >= other.s3>>>0;
    var cs4 = t.s4>>>0 >= other.s4>>>0;
    var cs5 = t.s5>>>0 >= other.s5>>>0;
    var cs6 = t.s6>>>0 >= other.s6>>>0;
    var cs7 = t.s7>>>0 >= other.s7>>>0;
    var cs8 = t.s8>>>0 >= other.s8>>>0;
    var cs9 = t.s9>>>0 >= other.s9>>>0;
    var cs10 = t.s10>>>0 >= other.s10>>>0;
    var cs11 = t.s11>>>0 >= other.s11>>>0;
    var cs12 = t.s12>>>0 >= other.s12>>>0;
    var cs13 = t.s13>>>0 >= other.s13>>>0;
    var cs14 = t.s14>>>0 >= other.s14>>>0;
    var cs15 = t.s15>>>0 >= other.s15>>>0;
    return SIMD.int8x16.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                             cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.int8x16.unsignedLessThan === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @param {int8x16} other An instance of int8x16.
    * @return {int8x16} 0xFF or 0x0 in each lane depending on
    * the result of t>>>0 < other>>>0.
    */
  SIMD.int8x16.unsignedLessThan = function(t, other) {
    t = SIMD.int8x16(t);
    other = SIMD.int8x16(other);
    var cs0 = t.s0>>>0 < other.s0>>>0;
    var cs1 = t.s1>>>0 < other.s1>>>0;
    var cs2 = t.s2>>>0 < other.s2>>>0;
    var cs3 = t.s3>>>0 < other.s3>>>0;
    var cs4 = t.s4>>>0 < other.s4>>>0;
    var cs5 = t.s5>>>0 < other.s5>>>0;
    var cs6 = t.s6>>>0 < other.s6>>>0;
    var cs7 = t.s7>>>0 < other.s7>>>0;
    var cs8 = t.s8>>>0 < other.s8>>>0;
    var cs9 = t.s9>>>0 < other.s9>>>0;
    var cs10 = t.s10>>>0 < other.s10>>>0;
    var cs11 = t.s11>>>0 < other.s11>>>0;
    var cs12 = t.s12>>>0 < other.s12>>>0;
    var cs13 = t.s13>>>0 < other.s13>>>0;
    var cs14 = t.s14>>>0 < other.s14>>>0;
    var cs15 = t.s15>>>0 < other.s15>>>0;
    return SIMD.int8x16.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                             cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.int8x16.unsignedLessThanOrEqual === "undefined") {
  /**
    * @param {int8x16} t An instance of int8x16.
    * @param {int8x16} other An instance of int8x16.
    * @return {int8x16} 0xFF or 0x0 in each lane depending on
    * the result of t>>>0 <= other>>>0.
    */
  SIMD.int8x16.unsignedLessThanOrEqual = function(t, other) {
    t = SIMD.int8x16(t);
    other = SIMD.int8x16(other);
    var cs0 = t.s0>>>0 <= other.s0>>>0;
    var cs1 = t.s1>>>0 <= other.s1>>>0;
    var cs2 = t.s2>>>0 <= other.s2>>>0;
    var cs3 = t.s3>>>0 <= other.s3>>>0;
    var cs4 = t.s4>>>0 <= other.s4>>>0;
    var cs5 = t.s5>>>0 <= other.s5>>>0;
    var cs6 = t.s6>>>0 <= other.s6>>>0;
    var cs7 = t.s7>>>0 <= other.s7>>>0;
    var cs8 = t.s8>>>0 <= other.s8>>>0;
    var cs9 = t.s9>>>0 <= other.s9>>>0;
    var cs10 = t.s10>>>0 <= other.s10>>>0;
    var cs11 = t.s11>>>0 <= other.s11>>>0;
    var cs12 = t.s12>>>0 <= other.s12>>>0;
    var cs13 = t.s13>>>0 <= other.s13>>>0;
    var cs14 = t.s14>>>0 <= other.s14>>>0;
    var cs15 = t.s15>>>0 <= other.s15>>>0;
    return SIMD.int8x16.bool(cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7,
                             cs8, cs9, cs10, cs11, cs12, cs13, cs14, cs15);
  }
}

if (typeof SIMD.int8x16.shiftLeftByScalar === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int} bits Bit count to shift by.
    * @return {int8x16} lanes in a shifted by bits.
    */
  SIMD.int8x16.shiftLeftByScalar = function(a, bits) {
    a = SIMD.int8x16(a);
    if (bits>>>0 > 8)
      bits = 8;
    var s0 = a.s0 << bits;
    var s1 = a.s1 << bits;
    var s2 = a.s2 << bits;
    var s3 = a.s3 << bits;
    var s4 = a.s4 << bits;
    var s5 = a.s5 << bits;
    var s6 = a.s6 << bits;
    var s7 = a.s7 << bits;
    var s8 = t.s8 << bits;
    var s9 = t.s9 << bits;
    var s10 = t.s10 << bits;
    var s11 = t.s11 << bits;
    var s12 = t.s12 << bits;
    var s13 = t.s13 << bits;
    var s14 = t.s14 << bits;
    var s15 = t.s15 << bits;
    return SIMD.int8x16(s0, s1, s2, s3, s4, s5, s6, s7,
                        s8, s9, s10, s11, s12, s13, s14, s15);
  }
}

if (typeof SIMD.int8x16.shiftRightLogicalByScalar === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int} bits Bit count to shift by.
    * @return {int8x16} lanes in a shifted by bits.
    */
  SIMD.int8x16.shiftRightLogicalByScalar = function(a, bits) {
    a = SIMD.int8x16(a);
    if (bits>>>0 > 8)
      bits = 8;
    var s0 = a.s0 >>> bits;
    var s1 = a.s1 >>> bits;
    var s2 = a.s2 >>> bits;
    var s3 = a.s3 >>> bits;
    var s4 = a.s4 >>> bits;
    var s5 = a.s5 >>> bits;
    var s6 = a.s6 >>> bits;
    var s7 = a.s7 >>> bits;
    var s8 = t.s8 >>> bits;
    var s9 = t.s9 >>> bits;
    var s10 = t.s10 >>> bits;
    var s11 = t.s11 >>> bits;
    var s12 = t.s12 >>> bits;
    var s13 = t.s13 >>> bits;
    var s14 = t.s14 >>> bits;
    var s15 = t.s15 >>> bits;
    return SIMD.int8x16(s0, s1, s2, s3, s4, s5, s6, s7,
                        s8, s9, s10, s11, s12, s13, s14, s15);
  }
}

if (typeof SIMD.int8x16.shiftRightArithmeticByScalar === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int} bits Bit count to shift by.
    * @return {int8x16} lanes in a shifted by bits.
    */
  SIMD.int8x16.shiftRightArithmeticByScalar = function(a, bits) {
    a = SIMD.int8x16(a);
    if (bits>>>0 > 8)
      bits = 8;
    var s0 = a.s0 >> bits;
    var s1 = a.s1 >> bits;
    var s2 = a.s2 >> bits;
    var s3 = a.s3 >> bits;
    var s4 = a.s4 >> bits;
    var s5 = a.s5 >> bits;
    var s6 = a.s6 >> bits;
    var s7 = a.s7 >> bits;
    var s8 = t.s8 >> bits;
    var s9 = t.s9 >> bits;
    var s10 = t.s10 >> bits;
    var s11 = t.s11 >> bits;
    var s12 = t.s12 >> bits;
    var s13 = t.s13 >> bits;
    var s14 = t.s14 >> bits;
    var s15 = t.s15 >> bits;
    return SIMD.int8x16(s0, s1, s2, s3, s4, s5, s6, s7,
                        s8, s9, s10, s11, s12, s13, s14, s15);
  }
}

if (typeof SIMD.int8x16.shiftLeft === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int8x16} bits An instance of int8x16.
    * @return {int8x16} lanes New instance of int8x16 with values of a shifted
    *                   left by bits.
    */
  SIMD.int8x16.shiftLeft = function(a, bits) {
    a = SIMD.int8x16(a);
    bits = SIMD.int8x16(bits);
    var s0 = bits.s0>>>0 >= 32 ? 0 : a.s0 << bits.s0;
    var s1 = bits.s1>>>0 >= 32 ? 0 : a.s1 << bits.s1;
    var s2 = bits.s2>>>0 >= 32 ? 0 : a.s2 << bits.s2;
    var s3 = bits.s3>>>0 >= 32 ? 0 : a.s3 << bits.s3;
    var s4 = bits.s4>>>0 >= 32 ? 0 : a.s4 << bits.s4;
    var s5 = bits.s5>>>0 >= 32 ? 0 : a.s5 << bits.s5;
    var s6 = bits.s6>>>0 >= 32 ? 0 : a.s6 << bits.s6;
    var s7 = bits.s7>>>0 >= 32 ? 0 : a.s7 << bits.s7;
    var s8 = bits.s8>>>0 >= 32 ? 0 : a.s8 << bits.s8;
    var s9 = bits.s9>>>0 >= 32 ? 0 : a.s9 << bits.s9;
    var s10 = bits.s10>>>0 >= 32 ? 0 : a.s10 << bits.s10;
    var s11 = bits.s11>>>0 >= 32 ? 0 : a.s11 << bits.s11;
    var s12 = bits.s12>>>0 >= 32 ? 0 : a.s12 << bits.s12;
    var s13 = bits.s13>>>0 >= 32 ? 0 : a.s13 << bits.s13;
    var s14 = bits.s14>>>0 >= 32 ? 0 : a.s14 << bits.s14;
    var s15 = bits.s15>>>0 >= 32 ? 0 : a.s15 << bits.s15;
    return SIMD.int8x16(s0, s1, s2, s3, s4, s5, s6, s7,
                        s8, s9, s10, s11, s12, s13, s14, s15);
  }
}

if (typeof SIMD.int8x16.shiftRightLogical === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int8x16} bits An instance of int8x16.
    * @return {int8x16} lanes New instance of int8x16 with values of a
    *                   logically-shifted right by bits.
    */
  SIMD.int8x16.shiftRightLogical = function(a, bits) {
    a = SIMD.int8x16(a);
    bits = SIMD.int8x16(bits);
    var s0 = bits.s0>>>0 >= 32 ? 0 : a.s0 >>> bits.s0;
    var s1 = bits.s1>>>0 >= 32 ? 0 : a.s1 >>> bits.s1;
    var s2 = bits.s2>>>0 >= 32 ? 0 : a.s2 >>> bits.s2;
    var s3 = bits.s3>>>0 >= 32 ? 0 : a.s3 >>> bits.s3;
    var s4 = bits.s4>>>0 >= 32 ? 0 : a.s4 >>> bits.s4;
    var s5 = bits.s5>>>0 >= 32 ? 0 : a.s5 >>> bits.s5;
    var s5 = bits.s6>>>0 >= 32 ? 0 : a.s6 >>> bits.s6;
    var s6 = bits.s7>>>0 >= 32 ? 0 : a.s7 >>> bits.s7;
    var s8 = bits.s8>>>0 >= 32 ? 0 : a.s8 >>> bits.s8;
    var s9 = bits.s9>>>0 >= 32 ? 0 : a.s9 >>> bits.s9;
    var s10 = bits.s10>>>0 >= 32 ? 0 : a.s10 >>> bits.s10;
    var s11 = bits.s11>>>0 >= 32 ? 0 : a.s11 >>> bits.s11;
    var s12 = bits.s12>>>0 >= 32 ? 0 : a.s12 >>> bits.s12;
    var s13 = bits.s13>>>0 >= 32 ? 0 : a.s13 >>> bits.s13;
    var s14 = bits.s14>>>0 >= 32 ? 0 : a.s14 >>> bits.s14;
    var s15 = bits.s15>>>0 >= 32 ? 0 : a.s15 >>> bits.s15;
    return SIMD.int8x16(s0, s1, s2, s3, s4, s5, s6, s7,
                        s8, s9, s10, s11, s12, s13, s14, s15);
  }
}

if (typeof SIMD.int8x16.shiftRightArithmetic === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int8x16} bits An instance of int8x16.
    * @return {int8x16} lanes New instance of int8x16 with values of a
    *                   arithmetic-shifted right by bits.
    */
  SIMD.int8x16.shiftRightArithmetic = function(a, bits) {
    a = SIMD.int8x16(a);
    bits = SIMD.int8x16(bits);
    var s0 = a.s0 >> (bits.s0>>>0 > 31 ? 31 : bits.s0);
    var s1 = a.s1 >> (bits.s1>>>0 > 31 ? 31 : bits.s1);
    var s2 = a.s2 >> (bits.s2>>>0 > 31 ? 31 : bits.s2);
    var s3 = a.s3 >> (bits.s3>>>0 > 31 ? 31 : bits.s3);
    var s4 = a.s4 >> (bits.s4>>>0 > 31 ? 31 : bits.s4);
    var s5 = a.s5 >> (bits.s5>>>0 > 31 ? 31 : bits.s5);
    var s6 = a.s6 >> (bits.s6>>>0 > 31 ? 31 : bits.s6);
    var s7 = a.s7 >> (bits.s7>>>0 > 31 ? 31 : bits.s7);
    var s8 = a.s8 >> (bits.s8>>>0 > 31 ? 31 : bits.s8);
    var s9 = a.s9 >> (bits.s9>>>0 > 31 ? 31 : bits.s9);
    var s10 = a.s10 >> (bits.s10>>>0 > 31 ? 31 : bits.s10);
    var s11 = a.s11 >> (bits.s11>>>0 > 31 ? 31 : bits.s11);
    var s12 = a.s12 >> (bits.s12>>>0 > 31 ? 31 : bits.s12);
    var s13 = a.s13 >> (bits.s13>>>0 > 31 ? 31 : bits.s13);
    var s14 = a.s14 >> (bits.s14>>>0 > 31 ? 31 : bits.s14);
    var s15 = a.s15 >> (bits.s15>>>0 > 31 ? 31 : bits.s15);
    return SIMD.int8x16(s0, s1, s2, s3, s4, s5, s6, s7,
                        s8, s9, s10, s11, s12, s13, s14, s15);
  }
}

if (typeof SIMD.int8x16.load === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @return {int8x16} New instance of int8x16.
    */
  SIMD.int8x16.load = function(tarray, index) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    var i8temp = _SIMD_PRIVATE._i16x8;
    var array = bpe == 1 ? i8temp :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4) :
                _SIMD_PRIVATE._f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      array[i] = tarray[index + i];
    return SIMD.int8x16(i8temp[0], i8temp[1], i8temp[2], i8temp[3],
                        i8temp[4], i8temp[5], i8temp[6], i8temp[7],
                        i8temp[8], i8temp[9], i8temp[10], i8temp[11],
                        i8temp[12], i8temp[13], i8temp[14], i8temp[15]);
  }
}

if (typeof SIMD.int8x16.store === "undefined") {
  /**
    * @param {Typed array} tarray An instance of a typed array.
    * @param {Number} index An instance of Number.
    * @param {int8x16} value An instance of int8x16.
    * @return {void}
    */
  SIMD.int8x16.store = function(tarray, index, value) {
    if (!_SIMD_PRIVATE.isTypedArray(tarray))
      throw new TypeError("The 1st argument must be a typed array.");
    if (!_SIMD_PRIVATE.isNumber(index))
      throw new TypeError("The 2nd argument must be a Number.");
    var bpe = tarray.BYTES_PER_ELEMENT;
    if (index < 0 || (index * bpe + 16) > tarray.byteLength)
      throw new RangeError("The value of index is invalid.");
    value = SIMD.int8x16(value);
    _SIMD_PRIVATE._i8x16[0] = value.s0;
    _SIMD_PRIVATE._i8x16[1] = value.s1;
    _SIMD_PRIVATE._i8x16[2] = value.s2;
    _SIMD_PRIVATE._i8x16[3] = value.s3;
    _SIMD_PRIVATE._i8x16[4] = value.s4;
    _SIMD_PRIVATE._i8x16[5] = value.s5;
    _SIMD_PRIVATE._i8x16[6] = value.s6;
    _SIMD_PRIVATE._i8x16[7] = value.s7;
    _SIMD_PRIVATE._i8x16[8] = value.s8;
    _SIMD_PRIVATE._i8x16[9] = value.s9;
    _SIMD_PRIVATE._i8x16[10] = value.s10;
    _SIMD_PRIVATE._i8x16[11] = value.s11;
    _SIMD_PRIVATE._i8x16[12] = value.s12;
    _SIMD_PRIVATE._i8x16[13] = value.s13;
    _SIMD_PRIVATE._i8x16[14] = value.s14;
    _SIMD_PRIVATE._i8x16[15] = value.s15;
    var array = bpe == 1 ? _SIMD_PRIVATE._i8x16 :
                bpe == 2 ? _SIMD_PRIVATE._i16x8 :
                bpe == 4 ? (tarray instanceof Float32Array ? _SIMD_PRIVATE._f32x4 : _SIMD_PRIVATE._i32x4) :
                _SIMD_PRIVATE._f64x2;
    var n = 16 / bpe;
    for (var i = 0; i < n; ++i)
      tarray[index + i] = array[i];
  }
}

if (typeof SIMD.int8x16.saturatingAdd === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int8x16} b An instance of int8x16.
    * @return {int8x16} New instance of int8x16 with values of
    * Math.max(Math.min(a + b, 127), -128).
    */
  SIMD.int8x16.saturatingAdd = function(a, b) {
    a = SIMD.int8x16(a);
    b = SIMD.int8x16(b);
    return SIMD.int8x16(Math.max(Math.min(a.s0 + b.s0, 127), -128),
                        Math.max(Math.min(a.s1 + b.s1, 127), -128),
                        Math.max(Math.min(a.s2 + b.s2, 127), -128),
                        Math.max(Math.min(a.s3 + b.s3, 127), -128),
                        Math.max(Math.min(a.s4 + b.s4, 127), -128),
                        Math.max(Math.min(a.s5 + b.s5, 127), -128),
                        Math.max(Math.min(a.s6 + b.s6, 127), -128),
                        Math.max(Math.min(a.s7 + b.s7, 127), -128),
                        Math.max(Math.min(a.s8 + b.s8, 127), -128),
                        Math.max(Math.min(a.s9 + b.s9, 127), -128),
                        Math.max(Math.min(a.s10 + b.s10, 127), -128),
                        Math.max(Math.min(a.s11 + b.s11, 127), -128),
                        Math.max(Math.min(a.s12 + b.s12, 127), -128),
                        Math.max(Math.min(a.s13 + b.s13, 127), -128),
                        Math.max(Math.min(a.s14 + b.s14, 127), -128),
                        Math.max(Math.min(a.s15 + b.s15, 127), -128));
  }
}

if (typeof SIMD.int8x16.saturatingSub === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int8x16} b An instance of int8x16.
    * @return {int8x16} New instance of int8x16 with values of
    * Math.max(Math.min(a - b, 127), -128).
    */
  SIMD.int8x16.saturatingSub = function(a, b) {
    a = SIMD.int8x16(a);
    b = SIMD.int8x16(b);
    return SIMD.int8x16(Math.max(Math.min(a.s0 - b.s0, 127), -128),
                        Math.max(Math.min(a.s1 - b.s1, 127), -128),
                        Math.max(Math.min(a.s2 - b.s2, 127), -128),
                        Math.max(Math.min(a.s3 - b.s3, 127), -128),
                        Math.max(Math.min(a.s4 - b.s4, 127), -128),
                        Math.max(Math.min(a.s5 - b.s5, 127), -128),
                        Math.max(Math.min(a.s6 - b.s6, 127), -128),
                        Math.max(Math.min(a.s7 - b.s7, 127), -128),
                        Math.max(Math.min(a.s8 - b.s8, 127), -128),
                        Math.max(Math.min(a.s9 - b.s9, 127), -128),
                        Math.max(Math.min(a.s10 - b.s10, 127), -128),
                        Math.max(Math.min(a.s11 - b.s11, 127), -128),
                        Math.max(Math.min(a.s12 - b.s12, 127), -128),
                        Math.max(Math.min(a.s13 - b.s13, 127), -128),
                        Math.max(Math.min(a.s14 - b.s14, 127), -128),
                        Math.max(Math.min(a.s15 - b.s15, 127), -128));
  }
}

if (typeof SIMD.int8x16.unsignedSaturatingAdd === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int8x16} b An instance of int8x16.
    * @return {int8x16} New instance of int8x16 with values of
    * Math.max(Math.min(a + b, 255), 0).
    */
  SIMD.int8x16.unsignedSaturatingAdd = function(a, b) {
    a = SIMD.int8x16(a);
    b = SIMD.int8x16(b);
    return SIMD.int8x16(Math.max(Math.min(a.s0 + b.s0, 255), 0),
                        Math.max(Math.min(a.s1 + b.s1, 255), 0),
                        Math.max(Math.min(a.s2 + b.s2, 255), 0),
                        Math.max(Math.min(a.s3 + b.s3, 255), 0),
                        Math.max(Math.min(a.s4 + b.s4, 255), 0),
                        Math.max(Math.min(a.s5 + b.s5, 255), 0),
                        Math.max(Math.min(a.s6 + b.s6, 255), 0),
                        Math.max(Math.min(a.s7 + b.s7, 255), 0),
                        Math.max(Math.min(a.s8 + b.s8, 255), 0),
                        Math.max(Math.min(a.s9 + b.s9, 255), 0),
                        Math.max(Math.min(a.s10 + b.s10, 255), 0),
                        Math.max(Math.min(a.s11 + b.s11, 255), 0),
                        Math.max(Math.min(a.s12 + b.s12, 255), 0),
                        Math.max(Math.min(a.s13 + b.s13, 255), 0),
                        Math.max(Math.min(a.s14 + b.s14, 255), 0),
                        Math.max(Math.min(a.s15 + b.s15, 255), 0));
  }
}

if (typeof SIMD.int8x16.unsignedSaturatingSub === "undefined") {
  /**
    * @param {int8x16} a An instance of int8x16.
    * @param {int8x16} b An instance of int8x16.
    * @return {int8x16} New instance of int8x16 with values of
    * Math.max(Math.min(a - b, 255), 0).
    */
  SIMD.int8x16.unsignedSaturatingSub = function(a, b) {
    a = SIMD.int8x16(a);
    b = SIMD.int8x16(b);
    return SIMD.int8x16(Math.max(Math.min(a.s0 - b.s0, 255), 0),
                        Math.max(Math.min(a.s1 - b.s1, 255), 0),
                        Math.max(Math.min(a.s2 - b.s2, 255), 0),
                        Math.max(Math.min(a.s3 - b.s3, 255), 0),
                        Math.max(Math.min(a.s4 - b.s4, 255), 0),
                        Math.max(Math.min(a.s5 - b.s5, 255), 0),
                        Math.max(Math.min(a.s6 - b.s6, 255), 0),
                        Math.max(Math.min(a.s7 - b.s7, 255), 0),
                        Math.max(Math.min(a.s8 - b.s8, 255), 0),
                        Math.max(Math.min(a.s9 - b.s9, 255), 0),
                        Math.max(Math.min(a.s10 - b.s10, 255), 0),
                        Math.max(Math.min(a.s11 - b.s11, 255), 0),
                        Math.max(Math.min(a.s12 - b.s12, 255), 0),
                        Math.max(Math.min(a.s13 - b.s13, 255), 0),
                        Math.max(Math.min(a.s14 - b.s14, 255), 0),
                        Math.max(Math.min(a.s15 - b.s15, 255), 0));
  }
}

if (typeof SIMD.XXXX === "undefined") {
  // Swizzle Masks.
  Object.defineProperty(SIMD, 'XXXX', { get: function() { return 0x00; } });
  Object.defineProperty(SIMD, 'XXXY', { get: function() { return 0x40; } });
  Object.defineProperty(SIMD, 'XXXZ', { get: function() { return 0x80; } });
  Object.defineProperty(SIMD, 'XXXW', { get: function() { return 0xC0; } });
  Object.defineProperty(SIMD, 'XXYX', { get: function() { return 0x10; } });
  Object.defineProperty(SIMD, 'XXYY', { get: function() { return 0x50; } });
  Object.defineProperty(SIMD, 'XXYZ', { get: function() { return 0x90; } });
  Object.defineProperty(SIMD, 'XXYW', { get: function() { return 0xD0; } });
  Object.defineProperty(SIMD, 'XXZX', { get: function() { return 0x20; } });
  Object.defineProperty(SIMD, 'XXZY', { get: function() { return 0x60; } });
  Object.defineProperty(SIMD, 'XXZZ', { get: function() { return 0xA0; } });
  Object.defineProperty(SIMD, 'XXZW', { get: function() { return 0xE0; } });
  Object.defineProperty(SIMD, 'XXWX', { get: function() { return 0x30; } });
  Object.defineProperty(SIMD, 'XXWY', { get: function() { return 0x70; } });
  Object.defineProperty(SIMD, 'XXWZ', { get: function() { return 0xB0; } });
  Object.defineProperty(SIMD, 'XXWW', { get: function() { return 0xF0; } });
  Object.defineProperty(SIMD, 'XYXX', { get: function() { return 0x04; } });
  Object.defineProperty(SIMD, 'XYXY', { get: function() { return 0x44; } });
  Object.defineProperty(SIMD, 'XYXZ', { get: function() { return 0x84; } });
  Object.defineProperty(SIMD, 'XYXW', { get: function() { return 0xC4; } });
  Object.defineProperty(SIMD, 'XYYX', { get: function() { return 0x14; } });
  Object.defineProperty(SIMD, 'XYYY', { get: function() { return 0x54; } });
  Object.defineProperty(SIMD, 'XYYZ', { get: function() { return 0x94; } });
  Object.defineProperty(SIMD, 'XYYW', { get: function() { return 0xD4; } });
  Object.defineProperty(SIMD, 'XYZX', { get: function() { return 0x24; } });
  Object.defineProperty(SIMD, 'XYZY', { get: function() { return 0x64; } });
  Object.defineProperty(SIMD, 'XYZZ', { get: function() { return 0xA4; } });
  Object.defineProperty(SIMD, 'XYZW', { get: function() { return 0xE4; } });
  Object.defineProperty(SIMD, 'XYWX', { get: function() { return 0x34; } });
  Object.defineProperty(SIMD, 'XYWY', { get: function() { return 0x74; } });
  Object.defineProperty(SIMD, 'XYWZ', { get: function() { return 0xB4; } });
  Object.defineProperty(SIMD, 'XYWW', { get: function() { return 0xF4; } });
  Object.defineProperty(SIMD, 'XZXX', { get: function() { return 0x08; } });
  Object.defineProperty(SIMD, 'XZXY', { get: function() { return 0x48; } });
  Object.defineProperty(SIMD, 'XZXZ', { get: function() { return 0x88; } });
  Object.defineProperty(SIMD, 'XZXW', { get: function() { return 0xC8; } });
  Object.defineProperty(SIMD, 'XZYX', { get: function() { return 0x18; } });
  Object.defineProperty(SIMD, 'XZYY', { get: function() { return 0x58; } });
  Object.defineProperty(SIMD, 'XZYZ', { get: function() { return 0x98; } });
  Object.defineProperty(SIMD, 'XZYW', { get: function() { return 0xD8; } });
  Object.defineProperty(SIMD, 'XZZX', { get: function() { return 0x28; } });
  Object.defineProperty(SIMD, 'XZZY', { get: function() { return 0x68; } });
  Object.defineProperty(SIMD, 'XZZZ', { get: function() { return 0xA8; } });
  Object.defineProperty(SIMD, 'XZZW', { get: function() { return 0xE8; } });
  Object.defineProperty(SIMD, 'XZWX', { get: function() { return 0x38; } });
  Object.defineProperty(SIMD, 'XZWY', { get: function() { return 0x78; } });
  Object.defineProperty(SIMD, 'XZWZ', { get: function() { return 0xB8; } });
  Object.defineProperty(SIMD, 'XZWW', { get: function() { return 0xF8; } });
  Object.defineProperty(SIMD, 'XWXX', { get: function() { return 0x0C; } });
  Object.defineProperty(SIMD, 'XWXY', { get: function() { return 0x4C; } });
  Object.defineProperty(SIMD, 'XWXZ', { get: function() { return 0x8C; } });
  Object.defineProperty(SIMD, 'XWXW', { get: function() { return 0xCC; } });
  Object.defineProperty(SIMD, 'XWYX', { get: function() { return 0x1C; } });
  Object.defineProperty(SIMD, 'XWYY', { get: function() { return 0x5C; } });
  Object.defineProperty(SIMD, 'XWYZ', { get: function() { return 0x9C; } });
  Object.defineProperty(SIMD, 'XWYW', { get: function() { return 0xDC; } });
  Object.defineProperty(SIMD, 'XWZX', { get: function() { return 0x2C; } });
  Object.defineProperty(SIMD, 'XWZY', { get: function() { return 0x6C; } });
  Object.defineProperty(SIMD, 'XWZZ', { get: function() { return 0xAC; } });
  Object.defineProperty(SIMD, 'XWZW', { get: function() { return 0xEC; } });
  Object.defineProperty(SIMD, 'XWWX', { get: function() { return 0x3C; } });
  Object.defineProperty(SIMD, 'XWWY', { get: function() { return 0x7C; } });
  Object.defineProperty(SIMD, 'XWWZ', { get: function() { return 0xBC; } });
  Object.defineProperty(SIMD, 'XWWW', { get: function() { return 0xFC; } });
  Object.defineProperty(SIMD, 'YXXX', { get: function() { return 0x01; } });
  Object.defineProperty(SIMD, 'YXXY', { get: function() { return 0x41; } });
  Object.defineProperty(SIMD, 'YXXZ', { get: function() { return 0x81; } });
  Object.defineProperty(SIMD, 'YXXW', { get: function() { return 0xC1; } });
  Object.defineProperty(SIMD, 'YXYX', { get: function() { return 0x11; } });
  Object.defineProperty(SIMD, 'YXYY', { get: function() { return 0x51; } });
  Object.defineProperty(SIMD, 'YXYZ', { get: function() { return 0x91; } });
  Object.defineProperty(SIMD, 'YXYW', { get: function() { return 0xD1; } });
  Object.defineProperty(SIMD, 'YXZX', { get: function() { return 0x21; } });
  Object.defineProperty(SIMD, 'YXZY', { get: function() { return 0x61; } });
  Object.defineProperty(SIMD, 'YXZZ', { get: function() { return 0xA1; } });
  Object.defineProperty(SIMD, 'YXZW', { get: function() { return 0xE1; } });
  Object.defineProperty(SIMD, 'YXWX', { get: function() { return 0x31; } });
  Object.defineProperty(SIMD, 'YXWY', { get: function() { return 0x71; } });
  Object.defineProperty(SIMD, 'YXWZ', { get: function() { return 0xB1; } });
  Object.defineProperty(SIMD, 'YXWW', { get: function() { return 0xF1; } });
  Object.defineProperty(SIMD, 'YYXX', { get: function() { return 0x05; } });
  Object.defineProperty(SIMD, 'YYXY', { get: function() { return 0x45; } });
  Object.defineProperty(SIMD, 'YYXZ', { get: function() { return 0x85; } });
  Object.defineProperty(SIMD, 'YYXW', { get: function() { return 0xC5; } });
  Object.defineProperty(SIMD, 'YYYX', { get: function() { return 0x15; } });
  Object.defineProperty(SIMD, 'YYYY', { get: function() { return 0x55; } });
  Object.defineProperty(SIMD, 'YYYZ', { get: function() { return 0x95; } });
  Object.defineProperty(SIMD, 'YYYW', { get: function() { return 0xD5; } });
  Object.defineProperty(SIMD, 'YYZX', { get: function() { return 0x25; } });
  Object.defineProperty(SIMD, 'YYZY', { get: function() { return 0x65; } });
  Object.defineProperty(SIMD, 'YYZZ', { get: function() { return 0xA5; } });
  Object.defineProperty(SIMD, 'YYZW', { get: function() { return 0xE5; } });
  Object.defineProperty(SIMD, 'YYWX', { get: function() { return 0x35; } });
  Object.defineProperty(SIMD, 'YYWY', { get: function() { return 0x75; } });
  Object.defineProperty(SIMD, 'YYWZ', { get: function() { return 0xB5; } });
  Object.defineProperty(SIMD, 'YYWW', { get: function() { return 0xF5; } });
  Object.defineProperty(SIMD, 'YZXX', { get: function() { return 0x09; } });
  Object.defineProperty(SIMD, 'YZXY', { get: function() { return 0x49; } });
  Object.defineProperty(SIMD, 'YZXZ', { get: function() { return 0x89; } });
  Object.defineProperty(SIMD, 'YZXW', { get: function() { return 0xC9; } });
  Object.defineProperty(SIMD, 'YZYX', { get: function() { return 0x19; } });
  Object.defineProperty(SIMD, 'YZYY', { get: function() { return 0x59; } });
  Object.defineProperty(SIMD, 'YZYZ', { get: function() { return 0x99; } });
  Object.defineProperty(SIMD, 'YZYW', { get: function() { return 0xD9; } });
  Object.defineProperty(SIMD, 'YZZX', { get: function() { return 0x29; } });
  Object.defineProperty(SIMD, 'YZZY', { get: function() { return 0x69; } });
  Object.defineProperty(SIMD, 'YZZZ', { get: function() { return 0xA9; } });
  Object.defineProperty(SIMD, 'YZZW', { get: function() { return 0xE9; } });
  Object.defineProperty(SIMD, 'YZWX', { get: function() { return 0x39; } });
  Object.defineProperty(SIMD, 'YZWY', { get: function() { return 0x79; } });
  Object.defineProperty(SIMD, 'YZWZ', { get: function() { return 0xB9; } });
  Object.defineProperty(SIMD, 'YZWW', { get: function() { return 0xF9; } });
  Object.defineProperty(SIMD, 'YWXX', { get: function() { return 0x0D; } });
  Object.defineProperty(SIMD, 'YWXY', { get: function() { return 0x4D; } });
  Object.defineProperty(SIMD, 'YWXZ', { get: function() { return 0x8D; } });
  Object.defineProperty(SIMD, 'YWXW', { get: function() { return 0xCD; } });
  Object.defineProperty(SIMD, 'YWYX', { get: function() { return 0x1D; } });
  Object.defineProperty(SIMD, 'YWYY', { get: function() { return 0x5D; } });
  Object.defineProperty(SIMD, 'YWYZ', { get: function() { return 0x9D; } });
  Object.defineProperty(SIMD, 'YWYW', { get: function() { return 0xDD; } });
  Object.defineProperty(SIMD, 'YWZX', { get: function() { return 0x2D; } });
  Object.defineProperty(SIMD, 'YWZY', { get: function() { return 0x6D; } });
  Object.defineProperty(SIMD, 'YWZZ', { get: function() { return 0xAD; } });
  Object.defineProperty(SIMD, 'YWZW', { get: function() { return 0xED; } });
  Object.defineProperty(SIMD, 'YWWX', { get: function() { return 0x3D; } });
  Object.defineProperty(SIMD, 'YWWY', { get: function() { return 0x7D; } });
  Object.defineProperty(SIMD, 'YWWZ', { get: function() { return 0xBD; } });
  Object.defineProperty(SIMD, 'YWWW', { get: function() { return 0xFD; } });
  Object.defineProperty(SIMD, 'ZXXX', { get: function() { return 0x02; } });
  Object.defineProperty(SIMD, 'ZXXY', { get: function() { return 0x42; } });
  Object.defineProperty(SIMD, 'ZXXZ', { get: function() { return 0x82; } });
  Object.defineProperty(SIMD, 'ZXXW', { get: function() { return 0xC2; } });
  Object.defineProperty(SIMD, 'ZXYX', { get: function() { return 0x12; } });
  Object.defineProperty(SIMD, 'ZXYY', { get: function() { return 0x52; } });
  Object.defineProperty(SIMD, 'ZXYZ', { get: function() { return 0x92; } });
  Object.defineProperty(SIMD, 'ZXYW', { get: function() { return 0xD2; } });
  Object.defineProperty(SIMD, 'ZXZX', { get: function() { return 0x22; } });
  Object.defineProperty(SIMD, 'ZXZY', { get: function() { return 0x62; } });
  Object.defineProperty(SIMD, 'ZXZZ', { get: function() { return 0xA2; } });
  Object.defineProperty(SIMD, 'ZXZW', { get: function() { return 0xE2; } });
  Object.defineProperty(SIMD, 'ZXWX', { get: function() { return 0x32; } });
  Object.defineProperty(SIMD, 'ZXWY', { get: function() { return 0x72; } });
  Object.defineProperty(SIMD, 'ZXWZ', { get: function() { return 0xB2; } });
  Object.defineProperty(SIMD, 'ZXWW', { get: function() { return 0xF2; } });
  Object.defineProperty(SIMD, 'ZYXX', { get: function() { return 0x06; } });
  Object.defineProperty(SIMD, 'ZYXY', { get: function() { return 0x46; } });
  Object.defineProperty(SIMD, 'ZYXZ', { get: function() { return 0x86; } });
  Object.defineProperty(SIMD, 'ZYXW', { get: function() { return 0xC6; } });
  Object.defineProperty(SIMD, 'ZYYX', { get: function() { return 0x16; } });
  Object.defineProperty(SIMD, 'ZYYY', { get: function() { return 0x56; } });
  Object.defineProperty(SIMD, 'ZYYZ', { get: function() { return 0x96; } });
  Object.defineProperty(SIMD, 'ZYYW', { get: function() { return 0xD6; } });
  Object.defineProperty(SIMD, 'ZYZX', { get: function() { return 0x26; } });
  Object.defineProperty(SIMD, 'ZYZY', { get: function() { return 0x66; } });
  Object.defineProperty(SIMD, 'ZYZZ', { get: function() { return 0xA6; } });
  Object.defineProperty(SIMD, 'ZYZW', { get: function() { return 0xE6; } });
  Object.defineProperty(SIMD, 'ZYWX', { get: function() { return 0x36; } });
  Object.defineProperty(SIMD, 'ZYWY', { get: function() { return 0x76; } });
  Object.defineProperty(SIMD, 'ZYWZ', { get: function() { return 0xB6; } });
  Object.defineProperty(SIMD, 'ZYWW', { get: function() { return 0xF6; } });
  Object.defineProperty(SIMD, 'ZZXX', { get: function() { return 0x0A; } });
  Object.defineProperty(SIMD, 'ZZXY', { get: function() { return 0x4A; } });
  Object.defineProperty(SIMD, 'ZZXZ', { get: function() { return 0x8A; } });
  Object.defineProperty(SIMD, 'ZZXW', { get: function() { return 0xCA; } });
  Object.defineProperty(SIMD, 'ZZYX', { get: function() { return 0x1A; } });
  Object.defineProperty(SIMD, 'ZZYY', { get: function() { return 0x5A; } });
  Object.defineProperty(SIMD, 'ZZYZ', { get: function() { return 0x9A; } });
  Object.defineProperty(SIMD, 'ZZYW', { get: function() { return 0xDA; } });
  Object.defineProperty(SIMD, 'ZZZX', { get: function() { return 0x2A; } });
  Object.defineProperty(SIMD, 'ZZZY', { get: function() { return 0x6A; } });
  Object.defineProperty(SIMD, 'ZZZZ', { get: function() { return 0xAA; } });
  Object.defineProperty(SIMD, 'ZZZW', { get: function() { return 0xEA; } });
  Object.defineProperty(SIMD, 'ZZWX', { get: function() { return 0x3A; } });
  Object.defineProperty(SIMD, 'ZZWY', { get: function() { return 0x7A; } });
  Object.defineProperty(SIMD, 'ZZWZ', { get: function() { return 0xBA; } });
  Object.defineProperty(SIMD, 'ZZWW', { get: function() { return 0xFA; } });
  Object.defineProperty(SIMD, 'ZWXX', { get: function() { return 0x0E; } });
  Object.defineProperty(SIMD, 'ZWXY', { get: function() { return 0x4E; } });
  Object.defineProperty(SIMD, 'ZWXZ', { get: function() { return 0x8E; } });
  Object.defineProperty(SIMD, 'ZWXW', { get: function() { return 0xCE; } });
  Object.defineProperty(SIMD, 'ZWYX', { get: function() { return 0x1E; } });
  Object.defineProperty(SIMD, 'ZWYY', { get: function() { return 0x5E; } });
  Object.defineProperty(SIMD, 'ZWYZ', { get: function() { return 0x9E; } });
  Object.defineProperty(SIMD, 'ZWYW', { get: function() { return 0xDE; } });
  Object.defineProperty(SIMD, 'ZWZX', { get: function() { return 0x2E; } });
  Object.defineProperty(SIMD, 'ZWZY', { get: function() { return 0x6E; } });
  Object.defineProperty(SIMD, 'ZWZZ', { get: function() { return 0xAE; } });
  Object.defineProperty(SIMD, 'ZWZW', { get: function() { return 0xEE; } });
  Object.defineProperty(SIMD, 'ZWWX', { get: function() { return 0x3E; } });
  Object.defineProperty(SIMD, 'ZWWY', { get: function() { return 0x7E; } });
  Object.defineProperty(SIMD, 'ZWWZ', { get: function() { return 0xBE; } });
  Object.defineProperty(SIMD, 'ZWWW', { get: function() { return 0xFE; } });
  Object.defineProperty(SIMD, 'WXXX', { get: function() { return 0x03; } });
  Object.defineProperty(SIMD, 'WXXY', { get: function() { return 0x43; } });
  Object.defineProperty(SIMD, 'WXXZ', { get: function() { return 0x83; } });
  Object.defineProperty(SIMD, 'WXXW', { get: function() { return 0xC3; } });
  Object.defineProperty(SIMD, 'WXYX', { get: function() { return 0x13; } });
  Object.defineProperty(SIMD, 'WXYY', { get: function() { return 0x53; } });
  Object.defineProperty(SIMD, 'WXYZ', { get: function() { return 0x93; } });
  Object.defineProperty(SIMD, 'WXYW', { get: function() { return 0xD3; } });
  Object.defineProperty(SIMD, 'WXZX', { get: function() { return 0x23; } });
  Object.defineProperty(SIMD, 'WXZY', { get: function() { return 0x63; } });
  Object.defineProperty(SIMD, 'WXZZ', { get: function() { return 0xA3; } });
  Object.defineProperty(SIMD, 'WXZW', { get: function() { return 0xE3; } });
  Object.defineProperty(SIMD, 'WXWX', { get: function() { return 0x33; } });
  Object.defineProperty(SIMD, 'WXWY', { get: function() { return 0x73; } });
  Object.defineProperty(SIMD, 'WXWZ', { get: function() { return 0xB3; } });
  Object.defineProperty(SIMD, 'WXWW', { get: function() { return 0xF3; } });
  Object.defineProperty(SIMD, 'WYXX', { get: function() { return 0x07; } });
  Object.defineProperty(SIMD, 'WYXY', { get: function() { return 0x47; } });
  Object.defineProperty(SIMD, 'WYXZ', { get: function() { return 0x87; } });
  Object.defineProperty(SIMD, 'WYXW', { get: function() { return 0xC7; } });
  Object.defineProperty(SIMD, 'WYYX', { get: function() { return 0x17; } });
  Object.defineProperty(SIMD, 'WYYY', { get: function() { return 0x57; } });
  Object.defineProperty(SIMD, 'WYYZ', { get: function() { return 0x97; } });
  Object.defineProperty(SIMD, 'WYYW', { get: function() { return 0xD7; } });
  Object.defineProperty(SIMD, 'WYZX', { get: function() { return 0x27; } });
  Object.defineProperty(SIMD, 'WYZY', { get: function() { return 0x67; } });
  Object.defineProperty(SIMD, 'WYZZ', { get: function() { return 0xA7; } });
  Object.defineProperty(SIMD, 'WYZW', { get: function() { return 0xE7; } });
  Object.defineProperty(SIMD, 'WYWX', { get: function() { return 0x37; } });
  Object.defineProperty(SIMD, 'WYWY', { get: function() { return 0x77; } });
  Object.defineProperty(SIMD, 'WYWZ', { get: function() { return 0xB7; } });
  Object.defineProperty(SIMD, 'WYWW', { get: function() { return 0xF7; } });
  Object.defineProperty(SIMD, 'WZXX', { get: function() { return 0x0B; } });
  Object.defineProperty(SIMD, 'WZXY', { get: function() { return 0x4B; } });
  Object.defineProperty(SIMD, 'WZXZ', { get: function() { return 0x8B; } });
  Object.defineProperty(SIMD, 'WZXW', { get: function() { return 0xCB; } });
  Object.defineProperty(SIMD, 'WZYX', { get: function() { return 0x1B; } });
  Object.defineProperty(SIMD, 'WZYY', { get: function() { return 0x5B; } });
  Object.defineProperty(SIMD, 'WZYZ', { get: function() { return 0x9B; } });
  Object.defineProperty(SIMD, 'WZYW', { get: function() { return 0xDB; } });
  Object.defineProperty(SIMD, 'WZZX', { get: function() { return 0x2B; } });
  Object.defineProperty(SIMD, 'WZZY', { get: function() { return 0x6B; } });
  Object.defineProperty(SIMD, 'WZZZ', { get: function() { return 0xAB; } });
  Object.defineProperty(SIMD, 'WZZW', { get: function() { return 0xEB; } });
  Object.defineProperty(SIMD, 'WZWX', { get: function() { return 0x3B; } });
  Object.defineProperty(SIMD, 'WZWY', { get: function() { return 0x7B; } });
  Object.defineProperty(SIMD, 'WZWZ', { get: function() { return 0xBB; } });
  Object.defineProperty(SIMD, 'WZWW', { get: function() { return 0xFB; } });
  Object.defineProperty(SIMD, 'WWXX', { get: function() { return 0x0F; } });
  Object.defineProperty(SIMD, 'WWXY', { get: function() { return 0x4F; } });
  Object.defineProperty(SIMD, 'WWXZ', { get: function() { return 0x8F; } });
  Object.defineProperty(SIMD, 'WWXW', { get: function() { return 0xCF; } });
  Object.defineProperty(SIMD, 'WWYX', { get: function() { return 0x1F; } });
  Object.defineProperty(SIMD, 'WWYY', { get: function() { return 0x5F; } });
  Object.defineProperty(SIMD, 'WWYZ', { get: function() { return 0x9F; } });
  Object.defineProperty(SIMD, 'WWYW', { get: function() { return 0xDF; } });
  Object.defineProperty(SIMD, 'WWZX', { get: function() { return 0x2F; } });
  Object.defineProperty(SIMD, 'WWZY', { get: function() { return 0x6F; } });
  Object.defineProperty(SIMD, 'WWZZ', { get: function() { return 0xAF; } });
  Object.defineProperty(SIMD, 'WWZW', { get: function() { return 0xEF; } });
  Object.defineProperty(SIMD, 'WWWX', { get: function() { return 0x3F; } });
  Object.defineProperty(SIMD, 'WWWY', { get: function() { return 0x7F; } });
  Object.defineProperty(SIMD, 'WWWZ', { get: function() { return 0xBF; } });
  Object.defineProperty(SIMD, 'WWWW', { get: function() { return 0xFF; } });
}

if (typeof SIMD.XX === "undefined") {
  // Swizzle Masks.
  Object.defineProperty(SIMD, 'XX', { get: function() { return 0x0; } });
  Object.defineProperty(SIMD, 'XY', { get: function() { return 0x2; } });
  Object.defineProperty(SIMD, 'YX', { get: function() { return 0x1; } });
  Object.defineProperty(SIMD, 'YY', { get: function() { return 0x3; } });
}
