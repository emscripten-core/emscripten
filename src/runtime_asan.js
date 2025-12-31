/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if !USE_ASAN
#error "should only be included in USE_ASAN mode"
#endif

// C versions of asan_js_{load|store} will be used from compiled code, which have
// ASan instrumentation on them. However, until the wasm module is ready, we
// must access things directly.

function _asan_js_check_index(arr, index, asanFn) {
#if EXIT_RUNTIME
  if (runtimeInitialized && !runtimeExited) {
#else
  if (runtimeInitialized) {
#endif
    const elemSize = arr.BYTES_PER_ELEMENT;
    asanFn(index * elemSize, elemSize);
  }
  return index;
}
