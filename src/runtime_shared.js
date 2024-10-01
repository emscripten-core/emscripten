/**
 * @license
 * Copyright 2024 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

{{{
  // Helper function to export a heap symbol on the module object,
  // if requested.
  globalThis.maybeExportHeap = (x) => {
    // For now, we export all heap object when not building with MINIMAL_RUNTIME
    let shouldExport = !MINIMAL_RUNTIME && !STRICT;
    if (!shouldExport) {
      if (MODULARIZE && EXPORT_ALL) {
        shouldExport = true;
      } else if (AUDIO_WORKLET && (x == 'HEAPU32' || x == 'HEAPF32')) {
        // Export to the AudioWorkletGlobalScope the needed variables to access
        // the heap. AudioWorkletGlobalScope is unable to access global JS vars
        // in the compiled main JS file.
        shouldExport = true;
      } else if (EXPORTED_RUNTIME_METHODS.includes(x)) {
        shouldExport = true;
      }
    }

    return shouldExport ? `Module['${x}'] = ` : '';
  };
  null;
}}}

function updateMemoryViews() {
  var b = wasmMemory.buffer;
#if SUPPORT_BIG_ENDIAN
  {{{ maybeExportHeap('HEAP_DATA_VIEW') }}} HEAP_DATA_VIEW = new DataView(b);
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
}

#if MEMORY64 == 1
var toIndexType = (function() {
  // Probe for support of bigint bounds with memory64.
  // TODO(sbc): Remove this once all browsers start requiring bigint here.
  // See https://github.com/WebAssembly/memory64/issues/68
  var bigintMemoryBounds = 1;
  try {
    /** @suppress {checkTypes} */
    new WebAssembly.Memory({'initial': 1n, 'index': 'i64'});
  } catch (e) {
    bigintMemoryBounds = 0;
  }
  return (i) => bigintMemoryBounds ? BigInt(i) : i;
})();
#endif

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
