/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Create the wasm memory. (Note: this only applies if IMPORTED_MEMORY is defined)
#if !IMPORTED_MEMORY
{{{ throw "this file should not be be included when IMPORTED_MEMORY is set"; }}}
#endif

#if USE_PTHREADS
if (ENVIRONMENT_IS_PTHREAD) {
  wasmMemory = Module['wasmMemory'];
  buffer = Module['buffer'];
} else {
#endif // USE_PTHREADS

#if expectToReceiveOnModule('wasmMemory')
  if (Module['wasmMemory']) {
    wasmMemory = Module['wasmMemory'];
  } else
#endif
  {
    wasmMemory = new WebAssembly.Memory({
      'initial': INITIAL_MEMORY / {{{ WASM_PAGE_SIZE }}}
#if ALLOW_MEMORY_GROWTH
#if MAXIMUM_MEMORY != -1
      ,
      'maximum': {{{ MAXIMUM_MEMORY }}} / {{{ WASM_PAGE_SIZE }}}
#endif
#else
      ,
      'maximum': INITIAL_MEMORY / {{{ WASM_PAGE_SIZE }}}
#endif // ALLOW_MEMORY_GROWTH
#if USE_PTHREADS
      ,
      'shared': true
#endif
    });
#if USE_PTHREADS
    if (!(wasmMemory.buffer instanceof SharedArrayBuffer)) {
      err('requested a shared WebAssembly.Memory but the returned buffer is not a SharedArrayBuffer, indicating that while the browser has SharedArrayBuffer it does not have WebAssembly threads support - you may need to set a flag');
      if (ENVIRONMENT_IS_NODE) {
        console.log('(on node you may need: --experimental-wasm-threads --experimental-wasm-bulk-memory and also use a recent version)');
      }
      throw Error('bad memory');
    }
#endif
  }

#if USE_PTHREADS
}
#endif

if (wasmMemory) {
  buffer = wasmMemory.buffer;
}

// If the user provides an incorrect length, just use that length instead rather than providing the user to
// specifically provide the memory length with Module['INITIAL_MEMORY'].
INITIAL_MEMORY = buffer.byteLength;
#if ASSERTIONS
assert(INITIAL_MEMORY % {{{ WASM_PAGE_SIZE }}} === 0);
#endif
updateGlobalBufferAndViews(buffer);
