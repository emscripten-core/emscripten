/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if !USE_ASAN
#error "should only be inclded in USE_ASAN mode"
#endif

// C versions of asan_js_{load|store} will be used from compiled code, which have
// ASan instrumentation on them. However, until the wasm module is ready, we
// must access things directly.

function _asan_js_load(arr, index) {
  if (runtimeInitialized) {
    const elemSize = arr.BYTES_PER_ELEMENT;
    ___asan_loadN(index * elemSize, elemSize);
  }
  return arr[index];
}

function _asan_js_store(arr, index, value) {
  if (runtimeInitialized) {
    const elemSize = arr.BYTES_PER_ELEMENT;
    ___asan_storeN(index * elemSize, elemSize);
  }
  return arr[index] = value;
}
