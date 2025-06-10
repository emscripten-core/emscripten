/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
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
  $writeI53ToI64: (ptr, num) => {
    {{{ makeSetValue('ptr', 0, 'num', 'u32') }}};
    var lower = {{{ makeGetValue('ptr', 0, 'u32') }}};
    {{{ makeSetValue('ptr', 4, '(num - lower)/4294967296', 'u32') }}};
#if ASSERTIONS
    var deserialized = (num >= 0) ? readI53FromU64(ptr) : readI53FromI64(ptr);
    var offset = {{{ getHeapOffset('ptr', 'u32') }}};
    if (deserialized != num) warnOnce(`writeI53ToI64() out of range: serialized JS Number ${num} to Wasm heap as bytes lo=${ptrToString(HEAPU32[offset])}, hi=${ptrToString(HEAPU32[offset+1])}, which deserializes back to ${deserialized} instead!`);
#endif
  },

  // Same as writeI53ToI64, but if the double precision number does not fit within the
  // 64-bit number, the number is clamped to range [-2^63, 2^63-1].
  $writeI53ToI64Clamped__deps: ['$writeI53ToI64'],
  $writeI53ToI64Clamped: (ptr, num) => {
    if (num > 0x7FFFFFFFFFFFFFFF) {
      {{{ makeSetValue('ptr', 0, 0xFFFFFFFF, 'u32') }}};
      {{{ makeSetValue('ptr', 4, 0x7FFFFFFF, 'u32') }}};
    } else if (num < -0x8000000000000000) {
      {{{ makeSetValue('ptr', 0, 0, 'u32') }}};
      {{{ makeSetValue('ptr', 4, 0x80000000, 'u32') }}};
    } else {
      writeI53ToI64(ptr, num);
    }
  },

  // Like writeI53ToI64, but throws if the passed number is out of range of int64.
  $writeI53ToI64Signaling__deps: ['$writeI53ToI64'],
  $writeI53ToI64Signaling: (ptr, num) => {
    if (num > 0x7FFFFFFFFFFFFFFF || num < -0x8000000000000000) {
#if ASSERTIONS
      throw `RangeError in writeI53ToI64Signaling(): input value ${num} is out of range of int64`;
#else
      throw `RangeError: ${num}`;
#endif
    }
    writeI53ToI64(ptr, num);
  },

  // Uint64 variant of writeI53ToI64Clamped. Writes the Number to a Uint64 variable on
  // the heap, clamping out of range values to range [0, 2^64-1].
  $writeI53ToU64Clamped__deps: ['$writeI53ToI64'],
  $writeI53ToU64Clamped: (ptr, num) => {
    if (num > 0xFFFFFFFFFFFFFFFF) {
      {{{ makeSetValue('ptr', 0, 0xFFFFFFFF, 'u32') }}};
      {{{ makeSetValue('ptr', 4, 0xFFFFFFFF, 'u32') }}};
    } else if (num < 0) {
      {{{ makeSetValue('ptr', 0, 0, 'u32') }}};
      {{{ makeSetValue('ptr', 4, 0, 'u32') }}};
    } else {
      writeI53ToI64(ptr, num);
    }
  },

  // Like writeI53ToI64, but throws if the passed number is out of range of uint64.
  $writeI53ToU64Signaling__deps: ['$writeI53ToI64'],
  $writeI53ToU64Signaling: (ptr, num) => {
    if (num < 0 || num > 0xFFFFFFFFFFFFFFFF) {
#if ASSERTIONS
      throw `RangeError in writeI53ToU64Signaling(): input value ${num} is out of range of uint64`;
#else
      throw `RangeError: ${num}`;
#endif
    }
    writeI53ToI64(ptr, num);
  },

  // Reads a 64-bit signed integer from the WebAssembly heap and
  // converts it to a JavaScript Number, which can represent 53 integer bits precisely.
  // TODO: Add $readI53FromI64Signaling() variant.
  $readI53FromI64: (ptr) => {
    return {{{ makeGetValue('ptr', 0, 'u32') }}} + {{{ makeGetValue('ptr', 4, 'i32') }}} * 4294967296;
  },

  // Reads a 64-bit unsigned integer from the WebAssembly heap and
  // converts it to a JavaScript Number, which can represent 53 integer bits precisely.
  // TODO: Add $readI53FromU64Signaling() variant.
  $readI53FromU64: (ptr) => {
    return {{{ makeGetValue('ptr', 0, 'u32') }}} + {{{ makeGetValue('ptr', 4, 'u32') }}} * 4294967296;
  },

  // Converts the given signed 32-bit low-high pair to a JavaScript Number that
  // can represent 53 bits of precision.
  $convertI32PairToI53: (lo, hi) => {
#if ASSERTIONS
    // This function should not be getting called with too large unsigned numbers
    // in high part (if hi >= 0x7FFFFFFFF, one should have been calling
    // convertU32PairToI53())
    assert(hi === (hi|0));
#endif
    return (lo >>> 0) + hi * 4294967296;
  },

  // Converts the given signed 32-bit low-high pair to a JavaScript Number that can
  // represent 53 bits of precision. Returns a NaN if the number exceeds the safe
  // integer range representable by a Number (x > 9007199254740992 || x < -9007199254740992)
  $convertI32PairToI53Checked: (lo, hi) => {
#if ASSERTIONS
    assert(lo == (lo >>> 0) || lo == (lo|0)); // lo should either be a i32 or a u32
    assert(hi === (hi|0));                    // hi should be a i32
#endif
    return ((hi + 0x200000) >>> 0 < 0x400001 - !!lo) ? (lo >>> 0) + hi * 4294967296 : NaN;
  },

  // Converts the given unsigned 32-bit low-high pair to a JavaScript Number that can
  // represent 53 bits of precision.
  // TODO: Add $convertU32PairToI53Checked() variant.
  $convertU32PairToI53: (lo, hi) => {
    return (lo >>> 0) + (hi >>> 0) * 4294967296;
  },

#if WASM_BIGINT
  $INT53_MAX: '{{{ Math.pow(2, 53) }}}',
  $INT53_MIN: '-{{{ Math.pow(2, 53) }}}',
  // Counvert a bigint value (usually coming from Wasm->JS call) into an int53
  // JS Number.  This is used when we have an incoming i64 that we know is a
  // pointer or size_t and is expected to be within the int53 range.
  // Returns NaN if the incoming bigint is outside the range.
  $bigintToI53Checked__deps: ['$INT53_MAX', '$INT53_MIN'],
  $bigintToI53Checked: (num) => (num < INT53_MIN || num > INT53_MAX) ? NaN : Number(num),
#endif
});

#if WASM_BIGINT
globalThis.i53ConversionDeps = ['$bigintToI53Checked'];
#else
globalThis.i53ConversionDeps = ['$convertI32PairToI53Checked'];
#endif
