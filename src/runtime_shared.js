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
      } else if (AUDIO_WORKLET && (x == 'HEAP32' || x == 'HEAPU32')) {
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
  {{{ maybeExport('HEAP_DATA_VIEW') }}} HEAP_DATA_VIEW = new DataView(b);
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
