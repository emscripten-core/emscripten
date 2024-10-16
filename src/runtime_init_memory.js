/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Create the wasm memory. (Note: this only applies if IMPORTED_MEMORY is defined)
#if !IMPORTED_MEMORY
{{{ throw "this file should not be be included when IMPORTED_MEMORY is set"; }}}
#endif

// check for full engine support (use string 'subarray' to avoid closure compiler confusion)

#if PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
#endif // PTHREADS

#if expectToReceiveOnModule('wasmMemory')
  if (Module['wasmMemory']) {
    wasmMemory = Module['wasmMemory'];
  } else
#endif
  {
    {{{ makeModuleReceiveWithVar('INITIAL_MEMORY', undefined, INITIAL_MEMORY) }}}

#if ASSERTIONS
    assert(INITIAL_MEMORY >= {{{STACK_SIZE}}}, 'INITIAL_MEMORY should be larger than STACK_SIZE, was ' + INITIAL_MEMORY + '! (STACK_SIZE=' + {{{STACK_SIZE}}} + ')');
#endif
#if MINIMAL_RUNTIME && WASM_WORKERS
    wasmMemory = Module['mem'] || new WebAssembly.Memory({
#else
    wasmMemory = new WebAssembly.Memory({
#endif
      'initial': {{{ toIndexType(`INITIAL_MEMORY / ${WASM_PAGE_SIZE}`) }}},
#if ALLOW_MEMORY_GROWTH
      // In theory we should not need to emit the maximum if we want "unlimited"
      // or 4GB of memory, but VMs error on that atm, see
      // https://github.com/emscripten-core/emscripten/issues/14130
      // And in the pthreads case we definitely need to emit a maximum. So
      // always emit one.
      'maximum': {{{ toIndexType(MAXIMUM_MEMORY / WASM_PAGE_SIZE) }}},
#else
      'maximum': {{{ toIndexType(`INITIAL_MEMORY / ${WASM_PAGE_SIZE}`) }}},
#endif // ALLOW_MEMORY_GROWTH
#if SHARED_MEMORY
      'shared': true,
#endif
#if MEMORY64 == 1
      'index': 'i64',
#endif
    });
  }

  updateMemoryViews();
#if PTHREADS
}
#endif

