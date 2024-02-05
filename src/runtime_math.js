/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/imul
#if POLYFILL_OLD_MATH_FUNCTIONS || MIN_CHROME_VERSION < 28 || MIN_FIREFOX_VERSION < 20 || MIN_SAFARI_VERSION < 90000 // || MIN_NODE_VERSION < 0.12
// || MIN_NODE_VERSION < 0.12
// check for imul support, and also for correctness ( https://bugs.webkit.org/show_bug.cgi?id=126345 )
if (!Math.imul || Math.imul(0xffffffff, 5) !== -5) Math.imul = (a, b) => {
  var ah  = a >>> 16;
  var al = a & 0xffff;
  var bh  = b >>> 16;
  var bl = b & 0xffff;
  return (al*bl + ((ah*bl + al*bh) << 16))|0;
};
#endif

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/fround
#if POLYFILL_OLD_MATH_FUNCTIONS || MIN_CHROME_VERSION < 38 || MIN_FIREFOX_VERSION < 26 || MIN_SAFARI_VERSION < 80000 // || MIN_NODE_VERSION < 0.12
if (!Math.fround) {
  var froundBuffer = new Float32Array(1);
  Math.fround = (x) => { froundBuffer[0] = x; return froundBuffer[0] };
}
#endif

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/clz32
#if POLYFILL_OLD_MATH_FUNCTIONS || MIN_CHROME_VERSION < 38 || MIN_FIREFOX_VERSION < 31 // || MIN_NODE_VERSION < 0.12
Math.clz32 ||= (x) => {
  var n = 32;
  var y = x >> 16; if (y) { n -= 16; x = y; }
  y = x >> 8; if (y) { n -= 8; x = y; }
  y = x >> 4; if (y) { n -= 4; x = y; }
  y = x >> 2; if (y) { n -= 2; x = y; }
  y = x >> 1; if (y) return n - 2;
  return n - x;
};
#endif

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/trunc
#if POLYFILL_OLD_MATH_FUNCTIONS || MIN_CHROME_VERSION < 38 || MIN_FIREFOX_VERSION < 25 || MIN_SAFARI_VERSION < 80000 // || MIN_NODE_VERSION < 0.12
Math.trunc ||= (x) => {
  return x < 0 ? Math.ceil(x) : Math.floor(x);
};
#endif

#if ASSERTIONS
assert(Math.imul, 'This browser does not support Math.imul(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill');
assert(Math.fround, 'This browser does not support Math.fround(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill');
assert(Math.clz32, 'This browser does not support Math.clz32(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill');
assert(Math.trunc, 'This browser does not support Math.trunc(), build with LEGACY_VM_SUPPORT or POLYFILL_OLD_MATH_FUNCTIONS to add in a polyfill');
#endif
