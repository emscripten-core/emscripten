/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
#if ASSERTIONS
  $writeI53ToI64__deps: ['$readI53FromI64', '$readI53FromU64'
#if MINIMAL_RUNTIME
    , '$warnOnce'
#endif
  ],
#endif
  // Writes the given JavaScript Number to the WebAssembly heap as a 64-bit integer variable.
  // If the given number is not in the range [-2^53, 2^53] (inclusive), then an unexpectedly
  // rounded or incorrect number can be written to the heap. ("garbage in, garbage out")
  // Note that unlike the most other function variants in this library, there is no separate
  // function $writeI53ToU64(): the implementation would be identical, and it is up to the
  // C/C++ side code to interpret the resulting number as signed or unsigned as is desirable.
  $writeI53ToI64: function(ptr, num) {
    HEAPU32[ptr>>2] = num;
    HEAPU32[ptr+4>>2] = (num - HEAPU32[ptr>>2])/4294967296;
#if ASSERTIONS
    var deserialized = (num >= 0) ? readI53FromU64(ptr) : readI53FromI64(ptr);
    if (deserialized != num) warnOnce('writeI53ToI64() out of range: serialized JS Number ' + num + ' to Wasm heap as bytes lo=0x' + HEAPU32[ptr>>2].toString(16) + ', hi=0x' + HEAPU32[ptr+4>>2].toString(16) + ', which deserializes back to ' + deserialized + ' instead!');
#endif
  },

  // Same as writeI53ToI64, but if the double precision number does not fit within the
  // 64-bit number, the number is clamped to range [-2^63, 2^63-1].
  $writeI53ToI64Clamped: function(ptr, num) {
    if (num > 0x7FFFFFFFFFFFFFFF) {
      HEAPU32[ptr>>2] = 0xFFFFFFFF;
      HEAPU32[ptr+4>>2] = 0x7FFFFFFF;
    } else if (num < -0x8000000000000000) {
      HEAPU32[ptr>>2] = 0;
      HEAPU32[ptr+4>>2] = 0x80000000;
    } else {
      HEAPU32[ptr>>2] = num;
      HEAPU32[ptr+4>>2] = (num - HEAPU32[ptr>>2])/4294967296;
    }
  },

  // Like writeI53ToI64, but throws if the passed number is out of range of int64.
  $writeI53ToI64Signaling: function(ptr, num) {
    if (num > 0x7FFFFFFFFFFFFFFF || num < -0x8000000000000000) {
#if ASSERTIONS
      throw 'RangeError in writeI53ToI64Signaling(): input value ' + num + ' is out of range of int64';
#else
      throw 'RangeError:' + num;
#endif
    }
    HEAPU32[ptr>>2] = num;
    HEAPU32[ptr+4>>2] = (num - HEAPU32[ptr>>2])/4294967296;
  },

  // Uint64 variant of writeI53ToI64Clamped. Writes the Number to a Uint64 variable on
  // the heap, clamping out of range values to range [0, 2^64-1].
  $writeI53ToU64Clamped: function(ptr, num) {
    if (num > 0xFFFFFFFFFFFFFFFF) HEAPU32[ptr>>2] = HEAPU32[ptr+4>>2] = 0xFFFFFFFF;
    else if (num < 0) HEAPU32[ptr>>2] = HEAPU32[ptr+4>>2] = 0;
    else {
      HEAPU32[ptr>>2] = num;
      HEAPU32[ptr+4>>2] = (num - HEAPU32[ptr>>2])/4294967296;
    }
  },

  // Like writeI53ToI64, but throws if the passed number is out of range of uint64.
  $writeI53ToU64Signaling: function(ptr, num) {
    if (num < 0 || num > 0xFFFFFFFFFFFFFFFF) {
#if ASSERTIONS
      throw 'RangeError in writeI53ToU64Signaling(): input value ' + num + ' is out of range of uint64';
#else
      throw 'RangeError:'+num;
#endif
    }
    HEAPU32[ptr>>2] = num;
    HEAPU32[ptr+4>>2] = (num - HEAPU32[ptr>>2])/4294967296;
  },

  // Reads a 64-bit signed integer from the WebAssembly heap and
  // converts it to a JavaScript Number, which can represent 53 integer bits precisely.
  // TODO: Add $readI53FromI64Signaling() variant.
  $readI53FromI64: function(ptr) {
    return HEAPU32[ptr>>2] + HEAP32[ptr+4>>2] * 4294967296;
  },

  // Reads a 64-bit unsigned integer from the WebAssembly heap and
  // converts it to a JavaScript Number, which can represent 53 integer bits precisely.
  // TODO: Add $readI53FromU64Signaling() variant.
  $readI53FromU64: function(ptr) {
    return HEAPU32[ptr>>2] + HEAPU32[ptr+4>>2] * 4294967296;
  },

  // Converts the given signed 32-bit low-high pair to a JavaScript Number that can
  // represent 53 bits of precision.
  // TODO: Add $convertI32PairToI53Signaling() variant.
  $convertI32PairToI53: function(lo, hi) {
#if ASSERTIONS
    // This function should not be getting called with too large unsigned numbers
    // in high part (if hi >= 0x7FFFFFFFF, one should have been calling
    // convertU32PairToI53())
    assert(hi === (hi|0));
#endif
    return (lo >>> 0) + hi * 4294967296;
  },

  // Converts the given unsigned 32-bit low-high pair to a JavaScript Number that can
  // represent 53 bits of precision.
  // TODO: Add $convertU32PairToI53Signaling() variant.
  $convertU32PairToI53: function(lo, hi) {
    return (lo >>> 0) + (hi >>> 0) * 4294967296;
  }
});
