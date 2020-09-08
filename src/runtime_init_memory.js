/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Create the main memory. (Note: this isn't used in STANDALONE_WASM mode since the wasm
// memory is created in the wasm, not in JS.)
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
      'initial': INITIAL_INITIAL_MEMORY / WASM_PAGE_SIZE
#if ALLOW_MEMORY_GROWTH
#if MAXIMUM_MEMORY != -1
      ,
      'maximum': {{{ MAXIMUM_MEMORY }}} / WASM_PAGE_SIZE
#endif
#else
      ,
      'maximum': INITIAL_INITIAL_MEMORY / WASM_PAGE_SIZE
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
INITIAL_INITIAL_MEMORY = buffer.byteLength;
#ifdef ASSERTIONS
assert(INITIAL_INITIAL_MEMORY % WASM_PAGE_SIZE === 0);
#ifdef ALLOW_MEMORY_GROWTH && MAXIMUM_MEMORY != -1
assert({{{ WASM_PAGE_SIZE }}} % WASM_PAGE_SIZE === 0);
#endif
#endif
updateGlobalBufferAndViews(buffer);

#if USE_PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) { // Pthreads have already initialized these variables in src/worker.js, where they were passed to the thread worker at startup time
#endif
#if USE_PTHREADS
}
#endif
