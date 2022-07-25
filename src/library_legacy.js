/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $ALLOC_NORMAL: 0,  // Tries to use _malloc()
  $ALLOC_STACK: 1,  // Lives for the duration of the current function call

  /**
   * allocate(): This function is no longer used by emscripten but is kept around to avoid
   *             breaking external users.
   *             You should normally not use allocate(), and instead allocate
   *             memory using _malloc()/stackAlloc(), initialize it with
   *             setValue(), and so forth.
   * @param {(Uint8Array|Array<number>)} slab: An array of data.
   * @param {number=} allocator : How to allocate memory, see ALLOC_*
   */
  $allocate__deps: ['$ALLOC_NORMAL', '$ALLOC_STACK'],
  $allocate: function(slab, allocator) {
    var ret;
  #if ASSERTIONS
    assert(typeof allocator == 'number', 'allocate no longer takes a type argument')
    assert(typeof slab != 'number', 'allocate no longer takes a number as arg0')
  #endif

    if (allocator == ALLOC_STACK) {
      ret = stackAlloc(slab.length);
    } else {
      ret = {{{ makeMalloc('allocate', 'slab.length') }}};
    }

    if (!slab.subarray && !slab.slice) {
      slab = new Uint8Array(slab);
    }
    HEAPU8.set(slab, ret);
    return ret;
  },
});
