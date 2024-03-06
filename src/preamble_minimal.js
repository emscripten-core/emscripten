/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if SAFE_HEAP
#include "runtime_safe_heap.js"
#endif

#if USE_ASAN
#include "runtime_asan.js"
#endif

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

#if SAFE_HEAP && !WASM_BIGINT
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

#if SINGLE_FILE && WASM == 1 && !WASM2JS

#if SINGLE_FILE_BINARY_ENCODE
// Prevent Closure from minifying the binaryDecode() function, or otherwise
// Closure may analyze through the WASM_BINARY_DATA placeholder string into this
// function, leading into incorrect results.
/** @noinline */
function binaryDecode(bin) { for(var i=0,l=bin.length,o=new Uint8Array(l);i<l;++i) o[i]=bin.charCodeAt(i)-1; return o; }
Module['wasm'] = binaryDecode('<<< WASM_BINARY_DATA >>>');
#else
#include "base64Decode.js"
Module['wasm'] = base64Decode('<<< WASM_BINARY_DATA >>>');
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

#include "runtime_shared.js"

#if IMPORTED_MEMORY
#if PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
#endif
  wasmMemory =
#if WASM_WORKERS
    Module['mem'] ||
#endif
    new WebAssembly.Memory({
      'initial': {{{ INITIAL_MEMORY / WASM_PAGE_SIZE }}},
#if SHARED_MEMORY || !ALLOW_MEMORY_GROWTH || MAXIMUM_MEMORY != FOUR_GB
      'maximum': {{{ (ALLOW_MEMORY_GROWTH && MAXIMUM_MEMORY != FOUR_GB ? MAXIMUM_MEMORY : INITIAL_MEMORY) / WASM_PAGE_SIZE }}},
#endif
#if SHARED_MEMORY
      'shared': true,
#endif
#if MEMORY64 == 1
      'index': 'i64',
#endif
    });
#if PTHREADS
}
#if MODULARIZE
else {
  wasmMemory = Module['wasmMemory'];
}
#endif // MODULARIZE
#endif // PTHREADS

#if ASSERTIONS && SHARED_MEMORY
assert(wasmMemory.buffer instanceof SharedArrayBuffer, 'requested a shared WebAssembly.Memory but the returned buffer is not a SharedArrayBuffer, indicating that while the browser has SharedArrayBuffer it does not have WebAssembly threads support - you may need to set a flag');
#endif

updateMemoryViews();
#endif // IMPORTED_MEMORY

#include "runtime_stack_check.js"
#include "runtime_assertions.js"

#if LOAD_SOURCE_MAP
var wasmSourceMap;
#include "source_map_support.js"
#endif

#if USE_OFFSET_CONVERTER
var wasmOffsetConverter;
#include "wasm_offset_converter.js"
#endif

#if EXIT_RUNTIME
var __ATEXIT__    = []; // functions called during shutdown
var runtimeExited = false;
#endif

#if ASSERTIONS || SAFE_HEAP || USE_ASAN
var runtimeInitialized = false;
#endif

#include "runtime_math.js"
#include "memoryprofiler.js"
#include "runtime_exceptions.js"
#include "runtime_debug.js"

// === Body ===
