/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  emscripten_stack_get_base: function() {
    return STACK_BASE;
  },
  emscripten_stack_get_end: function() {
    return STACK_MAX;
  },

#if !WASM_BACKEND
  $abortStackOverflow__deps: ['$stackSave'],
#endif
  $abortStackOverflow__import: true,
  $abortStackOverflow: function(allocSize) {
    abort('Stack overflow! Attempted to allocate ' + allocSize + ' bytes on the stack, but stack has only ' + (STACK_MAX - stackSave() + allocSize) + ' bytes available!');
  },

#if !WASM_BACKEND
  $stackAlloc__asm: true,
  $stackAlloc__sig: 'ii',
#if ASSERTIONS || STACK_OVERFLOW_CHECK >= 2
  $stackAlloc__deps: ['$abortStackOverflow'],
#endif
  $stackAlloc: function(size) {
    size = size|0;
    var ret = 0;
    ret = STACKTOP;
    STACKTOP = (STACKTOP + size)|0;
    STACKTOP = (STACKTOP + 15)&-16;
#if ASSERTIONS || STACK_OVERFLOW_CHECK >= 2
    if ((STACKTOP|0) >= (STACK_MAX|0)) abortStackOverflow(size|0);
#endif
    return ret|0;
  },

  $stackSave__asm: true,
  $stackSave__sig: 'i',
  $stackSave: function() {
    return STACKTOP|0;
  },

  $stackRestore__asm: true,
  $stackRestore__sig: 'vi',
  $stackRestore: function(top) {
    top = top|0;
    STACKTOP = top;
  },

  // In wasm backend, these functions are implemented as native
  // functions in compiler-rt/stack_ops.s
  emscripten_stack_get_current__asm: true,
  emscripten_stack_get_current__sig: 'i',
  emscripten_stack_get_current: function() {
    return STACKTOP|0;
  },
  emscripten_stack_get_free__asm: true,
  emscripten_stack_get_free__sig: 'i',
  emscripten_stack_get_free: function() {
    return (STACK_MAX|0) - (STACKTOP|0);
  }
#endif
});
