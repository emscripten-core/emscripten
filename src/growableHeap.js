/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Support for growable heap + pthreads, where the buffer may change, so JS views
// must be updated.
function GROWABLE_HEAP(arr) {
  if (wasmMemory.buffer != arr.buffer) {
    updateMemoryViews();
  }
  return arr;
}
