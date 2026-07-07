/**
 * @license
 * Copyright 2024 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#include "runtime_exceptions.js"
#include "runtime_debug.js"

#if STACK_OVERFLOW_CHECK
#include "runtime_stack_check.js"
#endif

#if SAFE_HEAP
#include "runtime_safe_heap.js"
#endif

#if USE_ASAN
#include "runtime_asan.js"
#endif

#if SINGLE_FILE && SINGLE_FILE_BINARY_ENCODE && !WASM2JS
#include "binaryDecode.js"
#endif

#if SHARED_MEMORY && ALLOW_MEMORY_GROWTH && GROWABLE_ARRAYBUFFERS != 2
// Support for growable heap + pthreads, where the buffer may change, so JS views
// must be updated.
function growMemViews() {
  // `updateMemoryViews` updates all the views simultaneously, so it's enough to check any of them.
  if (typeof HEAP8 != 'undefined' && wasmMemory.buffer != HEAP8.buffer) {
    updateMemoryViews();
  }
}
#endif

#if (PTHREADS || WASM_WORKERS) && (ENVIRONMENT_MAY_BE_NODE && !WASM_ESM_INTEGRATION)
if (ENVIRONMENT_IS_NODE && {{{ ENVIRONMENT_IS_WORKER_THREAD() }}}) {
  // Create as web-worker-like an environment as we can.
  globalThis.self = globalThis;
  var parentPort = worker_threads.parentPort;
  // Deno and Bun already have `postMessage` defined on the global scope and
  // deliver messages to `globalThis.onmessage`, so we must not duplicate that
  // behavior here if `postMessage` is already present.
  if (!globalThis.postMessage) {
    parentPort.on('message', (msg) => globalThis.onmessage?.({ data: msg }));
    globalThis.postMessage = (msg) => parentPort.postMessage(msg);
  }
  // Node.js Workers do not pass postMessage()s and uncaught exception events to the parent
  // thread necessarily in the same order where they were generated in sequential program order.
  // See https://github.com/nodejs/node/issues/59617
  // To remedy this, capture all uncaughtExceptions in the Worker, and sequentialize those over
  // to the same postMessage pipe that other messages use.
  process.on("uncaughtException", (err) => {
#if PTHREADS_DEBUG
    dbg(`uncaughtException on worker thread: ${err.message}`);
#endif
    postMessage({ cmd: {{{ CMD_UNCAUGHT_EXN }}}, error: err });
    // Also shut down the Worker to match the same semantics as if this uncaughtException
    // handler was not registered.
    // (n.b. this will not shut down the whole Node.js app process, but just the Worker)
    process.exit(1);
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

// Memory management

#if SUPPORT_BIG_ENDIAN
/** @type {!DataView} */
var HEAP_DATA_VIEW;
#endif

#if !MINIMAL_RUNTIME || ASSERTIONS || SAFE_HEAP || USE_ASAN || MODULARIZE || PTHREADS
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
    } else if (EXPORTED_RUNTIME_METHODS.has(x)) {
      shouldExport = true;
    }
    return shouldExport;
  };
  const maybeExportHeap = (x) => {
    if (shouldExportHeap(x) && MODULARIZE != 'instance') {
      return `Module['${x}'] = `;
    }
    return '';
  };
  const isHeapNeeded = (x) => {
    return shouldExportHeap(x) || addedLibraryItems['$' + x];
  };
  const updateHeap = (x, type) => {
    if (isHeapNeeded(x)) {
      return `${maybeExportHeap(x)}${x} = new ${type}(b);`;
    }
    return '';
  };
}}}


#if ALLOW_MEMORY_GROWTH
// When ALLOW_MEMORY_GROWTH is enabled, the conversion from Wasm
// memory to ArrayBuffer requires some additional logic.
function getMemoryBuffer() {
#if GROWABLE_ARRAYBUFFERS == 2
  return wasmMemory.toResizableBuffer();
#else
#if GROWABLE_ARRAYBUFFERS == 1
#if SHARED_MEMORY && (MIN_FIREFOX_VERSION < 154)
  // Deserializing a growable SharedArrayBuffer was broken until Firefox 154
  // See: https://bugzilla.mozilla.org/show_bug.cgi?id=2021136
  var firefoxMatch = globalThis.navigator?.userAgent?.match(/Firefox\/(\d+)/);
  if (!firefoxMatch || Number(firefoxMatch[1]) >= 154) {
#endif
  try {
    // This method may be missing or could fail with `Memory must have a maximum`
    var b = wasmMemory.toResizableBuffer();
#if SHARED_MEMORY
    growMemViews = () => {};
#endif
    return b;
    
  } catch {}
#if SHARED_MEMORY && (MIN_FIREFOX_VERSION < 154)
  }
#endif
#endif // GROWABLE_ARRAYBUFFERS == 1
  return wasmMemory.buffer;
#endif // GROWABLE_ARRAYBUFFERS == 2
}
#endif // ALLOW_MEMORY_GROWTH

function updateMemoryViews() {
#if RUNTIME_DEBUG
  dbg(`updateMemoryViews: first=${typeof HEAP8 == 'undefined' || !HEAP8} size=${wasmMemory.buffer.byteLength}`);
#endif
#if ALLOW_MEMORY_GROWTH
  // If we already have a heap that is resizeable/growable buffer we don't
  // need to do anything in updateMemoryViews.
#if SHARED_MEMORY
  if (typeof HEAP8 != 'undefined' && HEAP8?.buffer?.growable) return;
#else
  if (typeof HEAP8 != 'undefined' && HEAP8?.buffer?.resizable) return;
#endif
  var b = getMemoryBuffer();
#else
#if ASSERTIONS
  // When memory growth is disabled this function should be called exactly once.
  assert(typeof HEAP8 == 'undefined' || !HEAP8, 'updateMemoryViews should only be called once when ALLOW_MEMORY_GROWTH=0');
#endif
  var b = wasmMemory.buffer;
#endif
  {{{ updateHeap('HEAP8',   'Int8Array')    }}}
  {{{ updateHeap('HEAP16',  'Int16Array')   }}}
  {{{ updateHeap('HEAPU8',  'Uint8Array')   }}}
  {{{ updateHeap('HEAPU16', 'Uint16Array')  }}}
  {{{ updateHeap('HEAP32',  'Int32Array')   }}}
  {{{ updateHeap('HEAPU32', 'Uint32Array')  }}}
  {{{ updateHeap('HEAPF32', 'Float32Array') }}}
  {{{ updateHeap('HEAPF64', 'Float64Array') }}}
#if WASM_BIGINT
  {{{ updateHeap('HEAP64',  'BigInt64Array') }}}
  {{{ updateHeap('HEAPU64', 'BigUint64Array') }}}
#endif
#if SUPPORT_BIG_ENDIAN
  {{{ maybeExportHeap('HEAP_DATA_VIEW') }}} HEAP_DATA_VIEW = new DataView(b);
  LE_HEAP_UPDATE();
#endif
}

#if IMPORTED_MEMORY
// In non-standalone/normal mode, we create the memory here.
#include "runtime_init_memory.js"
#endif // !IMPORTED_MEMORY && ASSERTIONS

#include "memoryprofiler.js"

#if !DECLARE_ASM_MODULE_EXPORTS
function exportAliases(wasmExports) {
{{{ makeExportAliases() }}}
}
#endif
