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

  https://github.com/johnmccutchan/ecmascript_simd/blob/master/src/ecmascript_simd.js
*/

"use strict";

/**
  * Construct a new instance of float32x4 number.
  * @param {double} value used for x lane.
  * @param {double} value used for y lane.
  * @param {double} value used for z lane.
  * @param {double} value used for w lane.
  * @constructor
  */
function float32x4(x, y, z, w) {
  if (!(this instanceof float32x4)) {
    return new float32x4(x, y, z, w);
  }
  this.storage_ = new Float32Array(4);
  this.storage_[0] = x;
  this.storage_[1] = y;
  this.storage_[2] = z;
  this.storage_[3] = w;
}

/**
  * Construct a new instance of float32x4 number with 0.0 in all lanes.
  * @constructor
  */
float32x4.zero = function() {
  return float32x4(0.0, 0.0, 0.0, 0.0);
}

/**
  * Construct a new instance of float32x4 number with the same value
  * in all lanes.
  * @param {double} value used for all lanes.
  * @constructor
  */
float32x4.splat = function(s) {
  return float32x4(s, s, s, s);
}

Object.defineProperty(float32x4.prototype, 'x', {
  get: function() { return this.storage_[0]; }
});

Object.defineProperty(float32x4.prototype, 'y', {
  get: function() { return this.storage_[1]; }
});

Object.defineProperty(float32x4.prototype, 'z', {
  get: function() { return this.storage_[2]; }
});

Object.defineProperty(float32x4.prototype, 'w',
  { get: function() { return this.storage_[3]; }
});

/**
  * Extract the sign bit from each lane return them in the first 4 bits.
  */
Object.defineProperty(float32x4.prototype, 'signMask', {
  get: function() {
    var mx = this.x < 0.0 ? 1 : 0;
    var my = this.y < 0.0 ? 1 : 0;
    var mz = this.z < 0.0 ? 1 : 0;
    var mw = this.w < 0.0 ? 1 : 0;
    return mx | my << 1 | mz << 2 | mw << 3;
  }
});

/**
  * Construct a new instance of int32x4 number.
  * @param {integer} 32-bit unsigned value used for x lane.
  * @param {integer} 32-bit unsigned value used for y lane.
  * @param {integer} 32-bit unsigned value used for z lane.
  * @param {integer} 32-bit unsigned value used for w lane.
  * @constructor
  */
function int32x4(x, y, z, w) {
  if (!(this instanceof int32x4)) {
    return new int32x4(x, y, z, w);
  }
  this.storage_ = new Int32Array(4);
  this.storage_[0] = x;
  this.storage_[1] = y;
  this.storage_[2] = z;
  this.storage_[3] = w;
}

/**
  * Construct a new instance of int32x4 number with 0xFFFFFFFF or 0x0 in each
  * lane, depending on the truth value in x, y, z, and w.
  * @param {boolean} flag used for x lane.
  * @param {boolean} flag used for y lane.
  * @param {boolean} flag used for z lane.
  * @param {boolean} flag used for w lane.
  * @constructor
  */
int32x4.bool = function(x, y, z, w) {
  return int32x4(x ? -1 : 0x0,
                  y ? -1 : 0x0,
                  z ? -1 : 0x0,
                  w ? -1 : 0x0);
}

/**
  * Construct a new instance of int32x4 number with the same value
  * in all lanes.
  * @param {integer} value used for all lanes.
  * @constructor
  */
int32x4.splat = function(s) {
  return int32x4(s, s, s, s);
}

Object.defineProperty(int32x4.prototype, 'x', {
  get: function() { return this.storage_[0]; }
});

Object.defineProperty(int32x4.prototype, 'y', {
  get: function() { return this.storage_[1]; }
});

Object.defineProperty(int32x4.prototype, 'z', {
  get: function() { return this.storage_[2]; }
});

Object.defineProperty(int32x4.prototype, 'w',
  { get: function() { return this.storage_[3]; }
});

Object.defineProperty(int32x4.prototype, 'flagX', {
  get: function() { return this.storage_[0] != 0x0; }
});

Object.defineProperty(int32x4.prototype, 'flagY', {
  get: function() { return this.storage_[1] != 0x0; }
});

Object.defineProperty(int32x4.prototype, 'flagZ', {
  get: function() { return this.storage_[2] != 0x0; }
});

Object.defineProperty(int32x4.prototype, 'flagW',
  { get: function() { return this.storage_[3] != 0x0; }
});

/**
  * Extract the sign bit from each lane return them in the first 4 bits.
  */
Object.defineProperty(int32x4.prototype, 'signMask', {
  get: function() {
    var mx = (this.storage_[0] & 0x80000000) >>> 31;
    var my = (this.storage_[1] & 0x80000000) >>> 31;
    var mz = (this.storage_[2] & 0x80000000) >>> 31;
    var mw = (this.storage_[3] & 0x80000000) >>> 31;
    return mx | my << 1 | mz << 2 | mw << 3;
  }
});

function isNumber(o) {
    return typeof o == "number" || (typeof o == "object" && o.constructor === Number);
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
         (o instanceof Float64Array) ||
         (o instanceof Float32x4Array);
}

function isArrayBuffer(o) {
  return (o instanceof ArrayBuffer);
}

function Float32x4Array(a, b, c) {
  if (isNumber(a)) {
    this.storage_ = new Float32Array(a*4);
    this.length_ = a;
    this.byteOffset_ = 0;
    return;
  } else if (isTypedArray(a)) {
    if (!(a instanceof Float32x4Array)) {
      throw "Copying typed array of non-Float32x4Array is unimplemented.";
    }
    this.storage_ = new Float32Array(a.length * 4);
    this.length_ = a.length;
    this.byteOffset_ = 0;
    // Copy floats.
    for (var i = 0; i < a.length*4; i++) {
      this.storage_[i] = a.storage_[i];
    }
  } else if (isArrayBuffer(a)) {
    if ((b != undefined) && (b % Float32x4Array.BYTES_PER_ELEMENT) != 0) {
      throw "byteOffset must be a multiple of 16.";
    }
    if (c != undefined) {
      c *= 4;
      this.storage_ = new Float32Array(a, b, c);
    }
    else {
      // Note: new Float32Array(a, b) is NOT equivalent to new Float32Array(a, b, undefined)
      this.storage_ = new Float32Array(a, b);
    }
    this.length_ = this.storage_.length / 4;
    this.byteOffset_ = b != undefined ? b : 0;
  } else {
    throw "Unknown type of first argument.";
  }
}

Object.defineProperty(Float32x4Array.prototype, 'length',
  { get: function() { return this.length_; }
});

Object.defineProperty(Float32x4Array.prototype, 'byteLength',
  { get: function() { return this.length_ * Float32x4Array.BYTES_PER_ELEMENT; }
});

Object.defineProperty(Float32x4Array, 'BYTES_PER_ELEMENT',
  { get: function() { return 16; }
});

Object.defineProperty(Float32x4Array.prototype, 'BYTES_PER_ELEMENT',
  { get: function() { return 16; }
});

Object.defineProperty(Float32x4Array.prototype, 'byteOffset',
  { get: function() { return this.byteOffset_; }
});

Object.defineProperty(Float32x4Array.prototype, 'buffer',
  { get: function() { return this.storage_.buffer; }
});

Float32x4Array.prototype.getAt = function(i) {
  if (i < 0) {
    throw "Index must be >= 0.";
  }
  if (i >= this.length) {
    throw "Index out of bounds.";
  }
  var x = this.storage_[i*4+0];
  var y = this.storage_[i*4+1];
  var z = this.storage_[i*4+2];
  var w = this.storage_[i*4+3];
  return float32x4(x, y, z, w);
}

Float32x4Array.prototype.setAt = function(i, v) {
  if (i < 0) {
    throw "Index must be >= 0.";
  }
  if (i >= this.length) {
    throw "Index out of bounds.";
  }
  if (!(v instanceof float32x4)) {
    throw "Value is not a float32x4.";
  }
  this.storage_[i*4+0] = v.x;
  this.storage_[i*4+1] = v.y;
  this.storage_[i*4+2] = v.z;
  this.storage_[i*4+3] = v.w;
}


function Int32x4Array(a, b, c) {

  function isNumber(o) {
      return typeof o == "number" || (typeof o == "object" && o.constructor === Number);
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
           (o instanceof Float64Array) ||
           (o instanceof Int32x4Array) ||
           (o instanceof Float32x4Array);
  }

  function isArrayBuffer(o) {
    return (o instanceof ArrayBuffer);
  }

  if (isNumber(a)) {
    this.storage_ = new Int32Array(a*4);
    this.length_ = a;
    this.byteOffset_ = 0;
    return;
  } else if (isTypedArray(a)) {
    if (!(a instanceof Int32x4Array)) {
      throw "Copying typed array of non-Int32x4Array is unimplemented.";
    }
    this.storage_ = new Int32Array(a.length * 4);
    this.length_ = a.length;
    this.byteOffset_ = 0;
    // Copy floats.
    for (var i = 0; i < a.length*4; i++) {
      this.storage_[i] = a.storage_[i];
    }
  } else if (isArrayBuffer(a)) {
    if ((b != undefined) && (b % Int32x4Array.BYTES_PER_ELEMENT) != 0) {
      throw "byteOffset must be a multiple of 16.";
    }
    if (c != undefined) {
      c *= 4;
      this.storage_ = new Int32Array(a, b, c);
    }
    else {
      // Note: new Int32Array(a, b) is NOT equivalent to new Float32Array(a, b, undefined)
      this.storage_ = new Int32Array(a, b);
    }
    this.length_ = this.storage_.length / 4;
    this.byteOffset_ = b != undefined ? b : 0;
  } else {
    throw "Unknown type of first argument.";
  }
}

Object.defineProperty(Int32x4Array.prototype, 'length',
  { get: function() { return this.length_; }
});

Object.defineProperty(Int32x4Array.prototype, 'byteLength',
  { get: function() { return this.length_ * Int32x4Array.BYTES_PER_ELEMENT; }
});

Object.defineProperty(Int32x4Array, 'BYTES_PER_ELEMENT',
  { get: function() { return 16; }
});

Object.defineProperty(Int32x4Array.prototype, 'BYTES_PER_ELEMENT',
  { get: function() { return 16; }
});

Object.defineProperty(Int32x4Array.prototype, 'byteOffset',
  { get: function() { return this.byteOffset_; }
});

Object.defineProperty(Int32x4Array.prototype, 'buffer',
  { get: function() { return this.storage_.buffer; }
});

Int32x4Array.prototype.getAt = function(i) {
  if (i < 0) {
    throw "Index must be >= 0.";
  }
  if (i >= this.length) {
    throw "Index out of bounds.";
  }
  var x = this.storage_[i*4+0];
  var y = this.storage_[i*4+1];
  var z = this.storage_[i*4+2];
  var w = this.storage_[i*4+3];
  return float32x4(x, y, z, w);
}

Int32x4Array.prototype.setAt = function(i, v) {
  if (i < 0) {
    throw "Index must be >= 0.";
  }
  if (i >= this.length) {
    throw "Index out of bounds.";
  }
  if (!(v instanceof int32x4)) {
    throw "Value is not a int32x4.";
  }
  this.storage_[i*4+0] = v.x;
  this.storage_[i*4+1] = v.y;
  this.storage_[i*4+2] = v.z;
  this.storage_[i*4+3] = v.w;
}

var SIMD = (function () {
  return {
    float32x4: {
        /**
        * @return {float32x4} New instance of float32x4 with absolute values of
        * t.
        */
      abs: function(t) {
        return new float32x4(Math.abs(t.x), Math.abs(t.y), Math.abs(t.z),
                             Math.abs(t.w));
      },
      /**
        * @return {float32x4} New instance of float32x4 with negated values of
        * t.
        */
      neg: function(t) {
        return new float32x4(-t.x, -t.y, -t.z, -t.w);
      },
      /**
        * @return {float32x4} New instance of float32x4 with a + b.
        */
      add: function(a, b) {
        return new float32x4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
      },
      /**
        * @return {float32x4} New instance of float32x4 with a - b.
        */
      sub: function(a, b) {
        return new float32x4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
      },
      /**
        * @return {float32x4} New instance of float32x4 with a * b.
        */
      mul: function(a, b) {
        return new float32x4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
      },
      /**
        * @return {float32x4} New instance of float32x4 with a / b.
        */
      div: function(a, b) {
        return new float32x4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
      },
      /**
        * @return {float32x4} New instance of float32x4 with t's values clamped
        * between lowerLimit and upperLimit.
        */
      clamp: function(t, lowerLimit, upperLimit) {
        var cx = t.x < lowerLimit.x ? lowerLimit.x : t.x;
        var cy = t.y < lowerLimit.y ? lowerLimit.y : t.y;
        var cz = t.z < lowerLimit.z ? lowerLimit.z : t.z;
        var cw = t.w < lowerLimit.w ? lowerLimit.w : t.w;
        cx = cx > upperLimit.x ? upperLimit.x : cx;
        cy = cy > upperLimit.y ? upperLimit.y : cy;
        cz = cz > upperLimit.z ? upperLimit.z : cz;
        cw = cw > upperLimit.w ? upperLimit.w : cw;
        return new float32x4(cx, cy, cz, cw);
      },
      /**
        * @return {float32x4} New instance of float32x4 with the minimum value of
        * t and other.
        */
      min: function(t, other) {
        var cx = t.x > other.x ? other.x : t.x;
        var cy = t.y > other.y ? other.y : t.y;
        var cz = t.z > other.z ? other.z : t.z;
        var cw = t.w > other.w ? other.w : t.w;
        return new float32x4(cx, cy, cz, cw);
      },
      /**
        * @return {float32x4} New instance of float32x4 with the maximum value of
        * t and other.
        */
      max: function(t, other) {
        var cx = t.x < other.x ? other.x : t.x;
        var cy = t.y < other.y ? other.y : t.y;
        var cz = t.z < other.z ? other.z : t.z;
        var cw = t.w < other.w ? other.w : t.w;
        return new float32x4(cx, cy, cz, cw);
      },
      /**
        * @return {float32x4} New instance of float32x4 with reciprocal value of
        * t.
        */
      reciprocal: function(t) {
        return new float32x4(1.0 / t.x, 1.0 / t.y, 1.0 / t.z, 1.0 / t.w);
      },
      /**
        * @return {float32x4} New instance of float32x4 with square root of the
        * reciprocal value of t.
        */
      reciprocalSqrt: function(t) {
        return new float32x4(Math.sqrt(1.0 / t.x), Math.sqrt(1.0 / t.y),
                             Math.sqrt(1.0 / t.z), Math.sqrt(1.0 / t.w));
      },
      /**
        * @return {float32x4} New instance of float32x4 with values of t
        * scaled by s.
        */
      scale: function(t, s) {
        return new float32x4(s * t.x, s * t.y, s * t.z, s * t.w);
      },
      /**
        * @return {float32x4} New instance of float32x4 with square root of
        * values of t.
        */
      sqrt: function(t) {
        return new float32x4(Math.sqrt(t.x), Math.sqrt(t.y),
                             Math.sqrt(t.z), Math.sqrt(t.w));
      },
      /**
        * @param {float32x4} t An instance of float32x4 to be shuffled.
        * @param {integer} mask One of the 256 shuffle masks, for example, SIMD.XXXX.
        * @return {float32x4} New instance of float32x4 with lanes shuffled.
        */
      shuffle: function(t, mask) {
        var _x = (mask) & 0x3;
        var _y = (mask >> 2) & 0x3;
        var _z = (mask >> 4) & 0x3;
        var _w = (mask >> 6) & 0x3;
        return new float32x4(t.storage_[_x], t.storage_[_y], t.storage_[_z],
                             t.storage_[_w]);
      },
      /**
        * @param {float32x4} t1 An instance of float32x4 to be shuffled. XY lanes in result
        * @param {float32x4} t2 An instance of float32x4 to be shuffled. ZW lanes in result
        * @param {integer} mask One of the 256 shuffle masks, for example, SIMD.XXXX.
        * @return {float32x4} New instance of float32x4 with lanes shuffled.
        */
      shuffleMix: function(t1, t2, mask) {
        var _x = (mask) & 0x3;
        var _y = (mask >> 2) & 0x3;
        var _z = (mask >> 4) & 0x3;
        var _w = (mask >> 6) & 0x3;
        return new float32x4(t1.storage_[_x], t1.storage_[_y], t2.storage_[_z],
                             t2.storage_[_w]);
      },
      /**
        * @param {double} value used for x lane.
        * @return {float32x4} New instance of float32x4 with the values in t and
        * x replaced with {x}.
        */
      withX: function(t, x) {
        return new float32x4(x, t.y, t.z, t.w);
      },
      /**
        * @param {double} value used for y lane.
        * @return {float32x4} New instance of float32x4 with the values in t and
        * y replaced with {y}.
        */
      withY: function(t, y) {
        return new float32x4(t.x, y, t.z, t.w);
      },
      /**
        * @param {double} value used for z lane.
        * @return {float32x4} New instance of float32x4 with the values in t and
        * z replaced with {z}.
        */
      withZ: function(t, z) {
        return new float32x4(t.x, t.y, z, t.w);
      },
      /**
        * @param {double} value used for w lane.
        * @return {float32x4} New instance of float32x4 with the values in t and
        * w replaced with {w}.
        */
      withW: function(t, w) {
        return new float32x4(t.x, t.y, t.z, w);
      },
      /**
        * @param {float32x4} t An instance of float32x4.
        * @param {float32x4} other An instance of float32x4.
        * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
        * the result of t < other.
        */
      lessThan: function(t, other) {
        var cx = t.x < other.x;
        var cy = t.y < other.y;
        var cz = t.z < other.z;
        var cw = t.w < other.w;
        return int32x4.bool(cx, cy, cz, cw);
      },
      /**
        * @param {float32x4} t An instance of float32x4.
        * @param {float32x4} other An instance of float32x4.
        * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
        * the result of t <= other.
        */
      lessThanOrEqual: function(t, other) {
        var cx = t.x <= other.x;
        var cy = t.y <= other.y;
        var cz = t.z <= other.z;
        var cw = t.w <= other.w;
        return int32x4.bool(cx, cy, cz, cw);
      },
      /**
        * @param {float32x4} t An instance of float32x4.
        * @param {float32x4} other An instance of float32x4.
        * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
        * the result of t == other.
        */
      equal: function(t, other) {
        var cx = t.x == other.x;
        var cy = t.y == other.y;
        var cz = t.z == other.z;
        var cw = t.w == other.w;
        return int32x4.bool(cx, cy, cz, cw);
      },
      /**
        * @param {float32x4} t An instance of float32x4.
        * @param {float32x4} other An instance of float32x4.
        * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
        * the result of t != other.
        */
      notEqual: function(t, other) {
        var cx = t.x != other.x;
        var cy = t.y != other.y;
        var cz = t.z != other.z;
        var cw = t.w != other.w;
        return int32x4.bool(cx, cy, cz, cw);
      },
      /**
        * @param {float32x4} t An instance of float32x4.
        * @param {float32x4} other An instance of float32x4.
        * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
        * the result of t >= other.
        */
      greaterThanOrEqual: function(t, other) {
        var cx = t.x >= other.x;
        var cy = t.y >= other.y;
        var cz = t.z >= other.z;
        var cw = t.w >= other.w;
        return int32x4.bool(cx, cy, cz, cw);
      },
      /**
        * @param {float32x4} t An instance of float32x4.
        * @param {float32x4} other An instance of float32x4.
        * @return {int32x4} 0xFFFFFFFF or 0x0 in each lane depending on
        * the result of t > other.
        */
      greaterThan: function(t, other) {
        var cx = t.x > other.x;
        var cy = t.y > other.y;
        var cz = t.z > other.z;
        var cw = t.w > other.w;
        return int32x4.bool(cx, cy, cz, cw);
      },
      /**
        * @param {float32x4} t An instance of float32x4.
        * @return {int32x4} a bit-wise copy of t as a int32x4.
        */
      bitsToInt32x4: function(t) {
        var alias = new Int32Array(t.storage_.buffer);
        return new int32x4(alias[0], alias[1], alias[2], alias[3]);
      },
      /**
        * @param {float32x4} t An instance of float32x4.
        * @return {int32x4} with a integer to float conversion of t.
        */
      toInt32x4: function(t) {
        var a = new int32x4(t.storage_[0], t.storage_[1], t.storage_[2],
                             t.storage_[3]);
        return a;
      }
    },
    int32x4: {
      /**
        * @param {int32x4} a An instance of int32x4.
        * @param {int32x4} b An instance of int32x4.
        * @return {int32x4} New instance of int32x4 with values of a & b.
        */
      and: function(a, b) {
        return new int32x4(a.x & b.x, a.y & b.y, a.z & b.z, a.w & b.w);
      },
      /**
        * @param {int32x4} a An instance of int32x4.
        * @param {int32x4} b An instance of int32x4.
        * @return {int32x4} New instance of int32x4 with values of a | b.
        */
      or: function(a, b) {
        return new int32x4(a.x | b.x, a.y | b.y, a.z | b.z, a.w | b.w);
      },
      /**
        * @param {int32x4} a An instance of int32x4.
        * @param {int32x4} b An instance of int32x4.
        * @return {int32x4} New instance of int32x4 with values of a ^ b.
        */
      xor: function(a, b) {
        return new int32x4(a.x ^ b.x, a.y ^ b.y, a.z ^ b.z, a.w ^ b.w);
      },
      /**
        * @param {int32x4} t An instance of int32x4.
        * @return {int32x4} New instance of int32x4 with values of ~t
        */
      not: function(t) {
        return new int32x4(~t.x, ~t.y, ~t.z, ~t.w);
      },
      /**
        * @param {int32x4} t An instance of int32x4.
        * @return {int32x4} New instance of int32x4 with values of -t
        */
      neg: function(t) {
        return new int32x4(-t.x, -t.y, -t.z, -t.w);
      },
      /**
        * @param {int32x4} a An instance of int32x4.
        * @param {int32x4} b An instance of int32x4.
        * @return {int32x4} New instance of int32x4 with values of a + b.
        */
      add: function(a, b) {
        return new int32x4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
      },
      /**
        * @param {int32x4} a An instance of int32x4.
        * @param {int32x4} b An instance of int32x4.
        * @return {int32x4} New instance of int32x4 with values of a - b.
        */
      sub: function(a, b) {
        return new int32x4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
      },
      /**
        * @param {int32x4} a An instance of int32x4.
        * @param {int32x4} b An instance of int32x4.
        * @return {int32x4} New instance of int32x4 with values of a * b.
        */
      mul: function(a, b) {
        return new int32x4(Math.imul(a.x, b.x), Math.imul(a.y, b.y),
                           Math.imul(a.z, b.z), Math.imul(a.w, b.w));
      },
      /**
        * @param {int32x4} t An instance of float32x4 to be shuffled.
        * @param {integer} mask One of the 256 shuffle masks, for example, SIMD.XXXX.
        * @return {int32x4} New instance of float32x4 with lanes shuffled.
        */
      shuffle: function(t, mask) {
        var _x = (mask) & 0x3;
        var _y = (mask >> 2) & 0x3;
        var _z = (mask >> 4) & 0x3;
        var _w = (mask >> 6) & 0x3;
        return new int32x4(t.storage_[_x], t.storage_[_y], t.storage_[_z],
                             t.storage_[_w]);
      },
      /**
        * @param {int32x4} t1 An instance of float32x4 to be shuffled. XY lanes in result
        * @param {int32x4} t2 An instance of float32x4 to be shuffled. ZW lanes in result
        * @param {integer} mask One of the 256 shuffle masks, for example, SIMD.XXXX.
        * @return {int32x4} New instance of float32x4 with lanes shuffled.
        */
      shuffleMix: function(t1, t2, mask) {
        var _x = (mask) & 0x3;
        var _y = (mask >> 2) & 0x3;
        var _z = (mask >> 4) & 0x3;
        var _w = (mask >> 6) & 0x3;
        return new int32x4(t1.storage_[_x], t1.storage_[_y], t2.storage_[_z],
                             t2.storage_[_w]);
      },
      /**
        * @param {float32x4}
        */
      select: function(t, trueValue, falseValue) {
        var tv = SIMD.float32x4.bitsToInt32x4(trueValue);
        var fv = SIMD.float32x4.bitsToInt32x4(falseValue);
        var tr = SIMD.int32x4.and(t, tv);
        var fr = SIMD.int32x4.and(SIMD.int32x4.not(t), fv);
        return SIMD.int32x4.bitsToFloat32x4(SIMD.int32x4.or(tr, fr));
      },
      /**
        * @param {int32x4} t An instance of int32x4.
        * @param {integer} 32-bit value used for x lane.
        * @return {int32x4} New instance of int32x4 with the values in t and
        * x lane replaced with {x}.
        */
      withX: function(t, x) {
        return new int32x4(x, t.y, t.z, t.w);
      },
      /**
        * param {int32x4} t An instance of int32x4.
        * @param {integer} 32-bit value used for y lane.
        * @return {int32x4} New instance of int32x4 with the values in t and
        * y lane replaced with {y}.
        */
      withY: function(t, y) {
        return new int32x4(t.x, y, t.z, t.w);
      },
      /**
        * @param {int32x4} t An instance of int32x4.
        * @param {integer} 32-bit value used for z lane.
        * @return {int32x4} New instance of int32x4 with the values in t and
        * z lane replaced with {z}.
        */
      withZ: function(t, z) {
        return new int32x4(t.x, t.y, z, t.w);
      },
      /**
        * @param {integer} 32-bit value used for w lane.
        * @return {int32x4} New instance of int32x4 with the values in t and
        * w lane replaced with {w}.
        */
      withW: function(t, w) {
        return new int32x4(t.x, t.y, t.z, w);
      },
      /**
        * @param {int32x4} t An instance of int32x4.
        * @param {boolean} x flag used for x lane.
        * @return {int32x4} New instance of int32x4 with the values in t and
        * x lane replaced with {x}.
        */
      withFlagX: function(t, flagX) {
        var x = flagX ? 0xFFFFFFFF : 0x0;
        return new int32x4(x, t.y, t.z, t.w);
      },
      /**
        * @param {int32x4} t An instance of int32x4.
        * @param {boolean} y flag used for y lane.
        * @return {int32x4} New instance of int32x4 with the values in t and
        * y lane replaced with {y}.
        */
      withFlagY: function(t, flagY) {
        var y = flagY ? 0xFFFFFFFF : 0x0;
        return new int32x4(t.x, y, t.z, t.w);
      },
      /**
        * @param {int32x4} t An instance of int32x4.
        * @param {boolean} z flag used for z lane.
        * @return {int32x4} New instance of int32x4 with the values in t and
        * z lane replaced with {z}.
        */
      withFlagZ: function(t, flagZ) {
        var z = flagZ ? 0xFFFFFFFF : 0x0;
        return new int32x4(t.x, t.y, z, t.w);
      },
      /**
        * @param {int32x4} t An instance of int32x4.
        * @param {boolean} w flag used for w lane.
        * @return {int32x4} New instance of int32x4 with the values in t and
        * w lane replaced with {w}.
        */
      withFlagW: function(t, flagW) {
        var w = flagW ? 0xFFFFFFFF : 0x0;
        return new int32x4(t.x, t.y, t.z, w);
      },
      /**
        * @param {int32x4} t An instance of int32x4.
        * @return {float32x4} a bit-wise copy of t as a float32x4.
        */
      bitsToFloat32x4: function(t) {
        var temp_storage = new Int32Array([t.storage_[0], t.storage_[1], t.storage_[2], t.storage_[3]]);
        var alias = new Float32Array(temp_storage.buffer);
        var fx4 = float32x4.zero();
        fx4.storage_ = alias;
        return fx4;      
      },
      /**
        * @param {int32x4} t An instance of int32x4.
        * @return {float32x4} with a float to integer conversion copy of t.
        */
      toFloat32x4: function(t) {
        var a = float32x4.zero();
        a.storage_[0] = t.storage_[0];
        a.storage_[1] = t.storage_[1];
        a.storage_[2] = t.storage_[2];
        a.storage_[3] = t.storage_[3];
        return a;
      }
    }
  }
})();

Object.defineProperty(SIMD, 'XXXX', { get: function() { return 0x0; } });
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
Object.defineProperty(SIMD, 'XYXX', { get: function() { return 0x4; } });
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
Object.defineProperty(SIMD, 'XZXX', { get: function() { return 0x8; } });
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
Object.defineProperty(SIMD, 'XWXX', { get: function() { return 0xC; } });
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
Object.defineProperty(SIMD, 'YXXX', { get: function() { return 0x1; } });
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
Object.defineProperty(SIMD, 'YYXX', { get: function() { return 0x5; } });
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
Object.defineProperty(SIMD, 'YZXX', { get: function() { return 0x9; } });
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
Object.defineProperty(SIMD, 'YWXX', { get: function() { return 0xD; } });
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
Object.defineProperty(SIMD, 'ZXXX', { get: function() { return 0x2; } });
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
Object.defineProperty(SIMD, 'ZYXX', { get: function() { return 0x6; } });
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
Object.defineProperty(SIMD, 'ZZXX', { get: function() { return 0xA; } });
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
Object.defineProperty(SIMD, 'ZWXX', { get: function() { return 0xE; } });
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
Object.defineProperty(SIMD, 'WXXX', { get: function() { return 0x3; } });
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
Object.defineProperty(SIMD, 'WYXX', { get: function() { return 0x7; } });
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
Object.defineProperty(SIMD, 'WZXX', { get: function() { return 0xB; } });
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
Object.defineProperty(SIMD, 'WWXX', { get: function() { return 0xF; } });
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
