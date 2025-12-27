/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 *
 * Legacy library symbols that are no longer used by emscripten itself but
 * could have external users.
 *
 * Symbols in this file are not available in `-sSTRICT` mode.
 *
 * Any usage of symbols in this file will result in a `-Wdeprecated` warning.
 *
 * Symbol in this file should be removed after "enough time" has passed such
 * that all external users have been able to transition away.
 */

legacyFuncs = {
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
  $allocate__deps: ['$ALLOC_STACK', 'malloc', '$stackAlloc'],
  $allocate: (slab, allocator) => {
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
  $writeStringToMemory: (string, buffer, dontAddNull) => {
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
  $writeAsciiToMemory: (str, buffer, dontAddNull) => {
    for (var i = 0; i < str.length; ++i) {
#if ASSERTIONS
      assert(str.charCodeAt(i) === (str.charCodeAt(i) & 0xff));
#endif
      {{{ makeSetValue('buffer++', 0, 'str.charCodeAt(i)', 'i8') }}};
    }
    // Null-terminate the string
    if (!dontAddNull) {{{ makeSetValue('buffer', 0, 0, 'i8') }}};
  },

  $allocateUTF8__deps: ['$stringToNewUTF8'],
  $allocateUTF8: (...args) => stringToNewUTF8(...args),
  $allocateUTF8OnStack__deps: ['$stringToUTF8OnStack'],
  $allocateUTF8OnStack: (...args) => stringToUTF8OnStack(...args),

#if LINK_AS_CXX
  $demangle__deps: ['$withStackSave', '__cxa_demangle', 'free', '$stringToUTF8OnStack'],
  $demangle: (func) => {
    // If demangle has failed before, stop demangling any further function names
    // This avoids an infinite recursion with malloc()->abort()->stackTrace()->demangle()->malloc()->...
    demangle.recursionGuard = (demangle.recursionGuard|0)+1;
    if (demangle.recursionGuard > 1) return func;
    return withStackSave(() => {
      try {
        var s = func;
        if (s.startsWith('__Z'))
          s = s.slice(1);
        var buf = stringToUTF8OnStack(s);
        var status = stackAlloc(4);
        var ret = ___cxa_demangle(buf, 0, 0, status);
        if ({{{ makeGetValue('status', '0', 'i32') }}} === 0 && ret) {
          return UTF8ToString(ret);
        }
        // otherwise, libcxxabi failed
      } catch(e) {
      } finally {
        _free(ret);
        if (demangle.recursionGuard < 2) --demangle.recursionGuard;
      }
      // failure when using libcxxabi, don't demangle
      return func;
    });
  },
#endif

  $stackTrace__deps: ['$jsStackTrace'],
  $stackTrace: () => {
    var js = jsStackTrace();
    if (Module['extraStackTrace']) js += '\n' + Module['extraStackTrace']();
    return js;
  },

  // Legacy names for runtime `out`/`err` symbols.
  $print: '=out',
  $printErr: '=err',

  // Converts a JS string to an integer base-10. Despite _s, which
  // suggests signaling error handling, this returns NaN on error.
  // (This was a mistake in the original implementation, and kept
  // to avoid breakage.)
  $jstoi_s: 'Number',

  $getNativeTypeSize__deps: ['$POINTER_SIZE'],
  $getNativeTypeSize: {{{ getNativeTypeSize }}},
};

if (WARN_DEPRECATED && !INCLUDE_FULL_LIBRARY) {
  for (const name of Object.keys(legacyFuncs)) {
    if (!isDecorator(name)) {
      const depsKey = `${name}__deps`;
      legacyFuncs[depsKey] ??= []
      legacyFuncs[depsKey].push(() => {
        warn(`JS library symbol '${name}' is deprecated. Please open a bug if you have a continuing need for this symbol [-Wdeprecated]`);
      });
    }
  }
}

addToLibrary(legacyFuncs);
