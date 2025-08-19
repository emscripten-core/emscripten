/**
 * @license
 * Copyright 2024 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#include "runtime_stack_check.js"
#include "runtime_exceptions.js"
#include "runtime_debug.js"

#if SAFE_HEAP
#include "runtime_safe_heap.js"
#endif

#if SHARED_MEMORY && ALLOW_MEMORY_GROWTH && !GROWABLE_ARRAYBUFFERS
// Support for growable heap + pthreads, where the buffer may change, so JS views
// must be updated.
function growMemViews() {
  // `updateMemoryViews` updates all the views simultaneously, so it's enough to check any of them.
  if (wasmMemory.buffer != HEAP8.buffer) {
    updateMemoryViews();
  }
}
#endif

#if USE_ASAN
#include "runtime_asan.js"
#endif

#if MODULARIZE
var readyPromiseResolve, readyPromiseReject;
#endif

#if (PTHREADS || WASM_WORKERS) && (ENVIRONMENT_MAY_BE_NODE && !WASM_ESM_INTEGRATION)
if (ENVIRONMENT_IS_NODE && {{{ ENVIRONMENT_IS_WORKER_THREAD() }}}) {
  // Create as web-worker-like an environment as we can.
  var parentPort = worker_threads['parentPort'];
  parentPort.on('message', (msg) => global.onmessage?.({ data: msg }));
  Object.assign(globalThis, {
    self: global,
    postMessage: (msg) => parentPort['postMessage'](msg),
  });
}
#endif // (PTHREADS || WASM_WORKERS) && (ENVIRONMENT_MAY_BE_NODE && !WASM_ESM_INTEGRATION)

#if PTHREADS
#include "runtime_pthread.js"
#endif

#if WASM_WORKERS
#include "wasm_worker.js"
#endif

#if AUDIO_WORKLET
#include "audio_worklet.js"
#endif

#if LOAD_SOURCE_MAP
var wasmSourceMap;
#include "source_map_support.js"
#endif

// Memory management

#if !WASM_ESM_INTEGRATION || IMPORTED_MEMORY
var wasmMemory;
#endif

var
/** @type {!Int8Array} */
  HEAP8,
/** @type {!Uint8Array} */
  HEAPU8,
/** @type {!Int16Array} */
  HEAP16,
/** @type {!Uint16Array} */
  HEAPU16,
/** @type {!Int32Array} */
  HEAP32,
/** @type {!Uint32Array} */
  HEAPU32,
/** @type {!Float32Array} */
  HEAPF32,
/** @type {!Float64Array} */
  HEAPF64;

#if WASM_BIGINT
// BigInt64Array type is not correctly defined in closure
var
/** not-@type {!BigInt64Array} */
  HEAP64,
/* BigUint64Array type is not correctly defined in closure
/** not-@type {!BigUint64Array} */
  HEAPU64;
#endif

#if SUPPORT_BIG_ENDIAN
/** @type {!DataView} */
var HEAP_DATA_VIEW;
#endif

#if !MINIMAL_RUNTIME || ASSERTIONS || SAFE_HEAP || USE_ASAN || MODULARIZE
var runtimeInitialized = false;
#endif

#if EXIT_RUNTIME
var runtimeExited = false;
#endif

{{{
  // Helper function to export a heap symbol on the module object,
  // if requested.
  const shouldExportHeap = (x) => {
    let shouldExport = false;
    if (MODULARIZE && EXPORT_ALL) {
      shouldExport = true;
    } else if (EXPORTED_RUNTIME_METHODS.includes(x)) {
      shouldExport = true;
    }
    return shouldExport;
  }
  const maybeExportHeap = (x) => {
    if (shouldExportHeap(x) && MODULARIZE != 'instance') {
      return `Module['${x}'] = `;
    }
    return '';
  };
}}}

function updateMemoryViews() {
#if GROWABLE_ARRAYBUFFERS
  var b = wasmMemory.toResizableBuffer();
#else
  var b = wasmMemory.buffer;
#endif
  {{{ maybeExportHeap('HEAP8')   }}}HEAP8 = new Int8Array(b);
  {{{ maybeExportHeap('HEAP16')  }}}HEAP16 = new Int16Array(b);
  {{{ maybeExportHeap('HEAPU8')  }}}HEAPU8 = new Uint8Array(b);
  {{{ maybeExportHeap('HEAPU16') }}}HEAPU16 = new Uint16Array(b);
  {{{ maybeExportHeap('HEAP32')  }}}HEAP32 = new Int32Array(b);
  {{{ maybeExportHeap('HEAPU32') }}}HEAPU32 = new Uint32Array(b);
  {{{ maybeExportHeap('HEAPF32') }}}HEAPF32 = new Float32Array(b);
  {{{ maybeExportHeap('HEAPF64') }}}HEAPF64 = new Float64Array(b);
#if WASM_BIGINT
  {{{ maybeExportHeap('HEAP64')  }}}HEAP64 = new BigInt64Array(b);
  {{{ maybeExportHeap('HEAPU64') }}}HEAPU64 = new BigUint64Array(b);
#endif
#if SUPPORT_BIG_ENDIAN
  {{{ maybeExportHeap('HEAP_DATA_VIEW') }}} HEAP_DATA_VIEW = new DataView(b);
  LE_HEAP_UPDATE();
#endif
}

#if ENVIRONMENT_MAY_BE_NODE && MIN_NODE_VERSION < 160000
// The performance global was added to node in v16.0.0:
// https://nodejs.org/api/globals.html#performance
if (ENVIRONMENT_IS_NODE) {
  // This is needed for emscripten_get_now and for pthreads support which
  // depends on it for accurate timing.
  // Use `global` rather than `globalThis` here since older versions of node
  // don't have `globalThis`.
  global.performance ??= require('perf_hooks').performance;
}
#endif

#if IMPORTED_MEMORY
// In non-standalone/normal mode, we create the memory here.
#include "runtime_init_memory.js"
#endif // !IMPORTED_MEMORY && ASSERTIONS

#include "memoryprofiler.js"
