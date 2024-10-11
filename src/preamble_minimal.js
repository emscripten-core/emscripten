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
#include "base64Decode.js"
Module['wasm'] = base64Decode('<<< WASM_BINARY_DATA >>>');
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

#if PTHREADS
#include "runtime_pthread.js"
#endif

#if IMPORTED_MEMORY
// This code is largely a duplcate of src/runtime_init_memory.js
// TODO(sbc): merge these two.
#if PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
#endif
  wasmMemory =
#if WASM_WORKERS
    Module['mem'] ||
#endif
    new WebAssembly.Memory({
      'initial': {{{ toIndexType(`${INITIAL_MEMORY} / ${WASM_PAGE_SIZE}`) }}},
#if ALLOW_MEMORY_GROWTH
      // In theory we should not need to emit the maximum if we want "unlimited"
      // or 4GB of memory, but VMs error on that atm, see
      // https://github.com/emscripten-core/emscripten/issues/14130
      // And in the pthreads case we definitely need to emit a maximum. So
      // always emit one.
      'maximum': {{{ toIndexType(MAXIMUM_MEMORY / WASM_PAGE_SIZE) }}},
#else
      'maximum': {{{ toIndexType(`${INITIAL_MEMORY} / ${WASM_PAGE_SIZE}`) }}},
#endif // ALLOW_MEMORY_GROWTH
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

#if PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
#endif

updateMemoryViews();

#if PTHREADS
}
#endif
#endif // IMPORTED_MEMORY

#include "runtime_stack_check.js"

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
