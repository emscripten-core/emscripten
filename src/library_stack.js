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
    // TODO(sbc): rename STACK_MAX -> STACK_END?
    return STACK_MAX;
  },

  $abortStackOverflow__import: true,
  $abortStackOverflow: function(allocSize) {
    abort('Stack overflow! Attempted to allocate ' + allocSize + ' bytes on the stack, but stack has only ' + (STACK_MAX - stackSave() + allocSize) + ' bytes available!');
  },
});
