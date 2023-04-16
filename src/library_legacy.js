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
  $allocate__deps: ['$ALLOC_NORMAL', '$ALLOC_STACK', 'malloc'],
  $allocate: function(slab, allocator) {
    var ret;
  #if ASSERTIONS
    assert(typeof allocator == 'number', 'allocate no longer takes a type argument')
    assert(typeof slab != 'number', 'allocate no longer takes a number as arg0')
  #endif

    if (allocator == ALLOC_STACK) {
      ret = stackAlloc(slab.length);
    } else {
      ret = _malloc(slab.length);
    }

    if (!slab.subarray && !slab.slice) {
      slab = new Uint8Array(slab);
    }
    HEAPU8.set(slab, ret);
    return ret;
  },

  // Deprecated: This function should not be called because it is unsafe and
  // does not provide a maximum length limit of how many bytes it is allowed to
  // write. Prefer calling the function stringToUTF8Array() instead, which takes
  // in a maximum length that can be used to be secure from out of bounds
  // writes.
  $writeStringToMemory__docs: '/** @deprecated @param {boolean=} dontAddNull */',
  $writeStringToMemory__deps: ['$lengthBytesUTF8', '$stringToUTF8'],
  $writeStringToMemory: function(string, buffer, dontAddNull) {
    warnOnce('writeStringToMemory is deprecated and should not be called! Use stringToUTF8() instead!');

    var /** @type {number} */ lastChar, /** @type {number} */ end;
    if (dontAddNull) {
      // stringToUTF8 always appends null. If we don't want to do that, remember the
      // character that existed at the location where the null will be placed, and restore
      // that after the write (below).
      end = buffer + lengthBytesUTF8(string);
      lastChar = HEAP8[end];
    }
    stringToUTF8(string, buffer, Infinity);
    if (dontAddNull) HEAP8[end] = lastChar; // Restore the value under the null character.
  },

  // Deprecated: Use stringToAscii
  $writeAsciiToMemory__docs: '/** @param {boolean=} dontAddNull */',
  $writeAsciiToMemory: function(str, buffer, dontAddNull) {
    for (var i = 0; i < str.length; ++i) {
#if ASSERTIONS
      assert(str.charCodeAt(i) === (str.charCodeAt(i) & 0xff));
#endif
      {{{ makeSetValue('buffer++', 0, 'str.charCodeAt(i)', 'i8') }}};
    }
    // Null-terminate the string
    if (!dontAddNull) {{{ makeSetValue('buffer', 0, 0, 'i8') }}};
  },

  $allocateUTF8: '$stringToNewUTF8',
  $allocateUTF8OnStack: '$stringToUTF8OnStack',
});
