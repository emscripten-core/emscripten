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

/** @param {string|number=} what */
function abort(what) {
  throw {{{ ASSERTIONS ? 'new Error(what)' : 'what' }}};
}

#if !WASM_BIGINT
// Globals used by JS i64 conversions (see makeSetValue)
var tempDouble;
var tempI64;
#endif

#if WASM != 2 && MAYBE_WASM2JS
#if !WASM2JS
if (Module['doWasm2JS']) {
#endif
#include "wasm2js.js"
#if !WASM2JS
}
#endif
#endif

var HEAP8, HEAP16, HEAP32, HEAPU8, HEAPU16, HEAPU32, HEAPF32, HEAPF64,
#if WASM_BIGINT
  HEAP64, HEAPU64,
#endif
#if SUPPORT_BIG_ENDIAN
  HEAP_DATA_VIEW,
#endif
  wasmMemory;

#if ASSERTIONS || SAFE_HEAP || USE_ASAN
var runtimeInitialized = false;
#endif

#if EXIT_RUNTIME
var __ATEXIT__    = []; // functions called during shutdown
var runtimeExited = false;
#endif

#include "runtime_shared.js"

#if IMPORTED_MEMORY
#include "runtime_init_memory.js"
#endif // IMPORTED_MEMORY

// === Body ===
