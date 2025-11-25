/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if ASSERTIONS
/** @type {function(*, string=)} */
function assert(condition, text) {
  if (!condition) throw text;
}
#endif

#if ASYNCIFY == 1 // ASYNCIFY-mode requires checking ABORT variable to avoid operating if code has aborted during an unwind
var ABORT = 0;
#endif

/** @param {string|number=} what */
function abort(what) {
#if ASYNCIFY == 1
  ABORT = 1;
#endif
  throw {{{ ASSERTIONS ? 'new Error(what)' : 'what' }}};
}

#if !WASM_BIGINT
// Globals used by JS i64 conversions (see makeSetValue)
var tempDouble;
var tempI64;
#endif

#if WASM2JS && WASM != 2
// WASM == 2 includes wasm2js.js separately.
#include "wasm2js.js"
#if !WASM2JS
}
#endif
#endif

#include "runtime_common.js"
