/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  emmalloc_unclaimed_heap_memory: function() {
  var dynamicTop = _sbrk();
#if ALLOW_MEMORY_GROWTH
#if MAXIMUM_MEMORY != -1
  // Using MAXIMUM_MEMORY to constrain max heap size.
  return {{{ MAXIMUM_MEMORY }}} - dynamicTop;
#else
  // Not using a Wasm memory bound.
  return 2*1024*1024*1024 - 65536 - dynamicTop;
#endif
#else
  // ALLOW_MEMORY_GROWTH is disabled, the current heap size
  // is all we got.
  return HEAPU8.length - dynamicTop;
#endif
  }
});
