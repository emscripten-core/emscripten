/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

{{{
  // Helper function to export a symbol on the module object
  // if requested.
  global.maybeExport = (x) => {
    return MODULARIZE && EXPORT_ALL ? `Module['${x}'] = ` : '';
  };
  // Export to the AudioWorkletGlobalScope the needed variables to access
  // the heap. AudioWorkletGlobalScope is unable to access global JS vars
  // in the compiled main JS file.
  global.maybeExportIfAudioWorklet = (x) => {
    return (MODULARIZE && EXPORT_ALL) || AUDIO_WORKLET ? `Module['${x}'] = ` : '';
  };
  null;
}}}

#if SAFE_HEAP
#include "runtime_safe_heap.js"
#endif

#if USE_ASAN
#include "runtime_asan.js"
#endif

#if ASSERTIONS || SAFE_HEAP
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
  wasmMemory, wasmTable;

function updateMemoryViews() {
  var b = wasmMemory.buffer;
#if ASSERTIONS && SHARED_MEMORY
  assert(b instanceof SharedArrayBuffer, 'requested a shared WebAssembly.Memory but the returned buffer is not a SharedArrayBuffer, indicating that while the browser has SharedArrayBuffer it does not have WebAssembly threads support - you may need to set a flag');
#endif
#if SUPPORT_BIG_ENDIAN
  {{{ maybeExport('HEAP_DATA_VIEW') }}} HEAP_DATA_VIEW = new DataView(b);
#endif
  {{{ maybeExport('HEAP8') }}} HEAP8 = new Int8Array(b);
  {{{ maybeExport('HEAP16') }}} HEAP16 = new Int16Array(b);
  {{{ maybeExport('HEAP32') }}} HEAP32 = new Int32Array(b);
  {{{ maybeExport('HEAPU8') }}} HEAPU8 = new Uint8Array(b);
  {{{ maybeExport('HEAPU16') }}} HEAPU16 = new Uint16Array(b);
  {{{ maybeExportIfAudioWorklet('HEAPU32') }}} HEAPU32 = new Uint32Array(b);
  {{{ maybeExportIfAudioWorklet('HEAPF32') }}} HEAPF32 = new Float32Array(b);
  {{{ maybeExport('HEAPF64') }}} HEAPF64 = new Float64Array(b);
#if WASM_BIGINT
  {{{ maybeExport('HEAP64') }}} HEAP64 = new BigInt64Array(b);
  {{{ maybeExport('HEAPU64') }}} HEAPU64 = new BigUint64Array(b);
#endif
}

#if IMPORTED_MEMORY
#if PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
#endif
  wasmMemory =
#if WASM_WORKERS
    Module['mem'] ||
#endif
    new WebAssembly.Memory({
    'initial': {{{ INITIAL_MEMORY >>> 16 }}}
#if SHARED_MEMORY || !ALLOW_MEMORY_GROWTH || MAXIMUM_MEMORY != FOUR_GB
    , 'maximum': {{{ (ALLOW_MEMORY_GROWTH && MAXIMUM_MEMORY != FOUR_GB ? MAXIMUM_MEMORY : INITIAL_MEMORY) >>> 16 }}}
#endif
#if SHARED_MEMORY
    , 'shared': true
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
