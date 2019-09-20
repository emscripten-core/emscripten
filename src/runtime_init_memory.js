#if USE_PTHREADS
if (ENVIRONMENT_IS_PTHREAD) {
#if MODULARIZE && WASM
  // In pthreads mode the wasmMemory and others are received in an onmessage, and that
  // onmessage then loadScripts us, sending wasmMemory etc. on Module. Here we recapture
  // it to a local so it can be used normally.
  wasmMemory = Module['wasmMemory'];
#endif
} else {
#endif // USE_PTHREADS
#if WASM

#if expectToReceiveOnModule('wasmMemory')
  if (Module['wasmMemory']) {
    wasmMemory = Module['wasmMemory'];
  } else
#endif
  {
    wasmMemory = new WebAssembly.Memory({
      'initial': INITIAL_TOTAL_MEMORY / WASM_PAGE_SIZE
#if ALLOW_MEMORY_GROWTH
#if WASM_MEM_MAX != -1
      ,
      'maximum': {{{ WASM_MEM_MAX }}} / WASM_PAGE_SIZE
#endif
#else
      ,
      'maximum': INITIAL_TOTAL_MEMORY / WASM_PAGE_SIZE
#endif // ALLOW_MEMORY_GROWTH
#if USE_PTHREADS
      ,
      'shared': true
#endif
    });
#if USE_PTHREADS
    assert(wasmMemory.buffer instanceof SharedArrayBuffer, 'requested a shared WebAssembly.Memory but the returned buffer is not a SharedArrayBuffer, indicating that while the browser has SharedArrayBuffer it does not have WebAssembly threads support - you may need to set a flag');
#endif
  }

#else // WASM

  if (Module['buffer']) {
    buffer = Module['buffer'];
  }
#ifdef USE_PTHREADS
  else if (typeof SharedArrayBuffer !== 'undefined') {
    buffer = new SharedArrayBuffer(INITIAL_TOTAL_MEMORY);
  }
#endif
  else {
    buffer = new ArrayBuffer(INITIAL_TOTAL_MEMORY);
  }
#endif // WASM
#if USE_PTHREADS
}
#endif

#if WASM
if (wasmMemory) {
  buffer = wasmMemory.buffer;
}
#endif

// If the user provides an incorrect length, just use that length instead rather than providing the user to
// specifically provide the memory length with Module['TOTAL_MEMORY'].
INITIAL_TOTAL_MEMORY = buffer.byteLength;
#ifdef ASSERTIONS && WASM
assert(INITIAL_TOTAL_MEMORY % WASM_PAGE_SIZE === 0);
#ifdef ALLOW_MEMORY_GROWTH && WASM_MEM_MAX != -1
assert({{{ WASM_PAGE_SIZE }}} % WASM_PAGE_SIZE === 0);
#endif
#endif
updateGlobalBufferAndViews(buffer);

#if USE_PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) { // Pthreads have already initialized these variables in src/worker.js, where they were passed to the thread worker at startup time
#endif
HEAP32[DYNAMICTOP_PTR>>2] = DYNAMIC_BASE;
#if USE_PTHREADS
}
#endif

