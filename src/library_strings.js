/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#include "arrayUtils.js"

addToLibrary({
  // TextDecoder constructor defaults to UTF-8
#if TEXTDECODER == 2
  $UTF8Decoder: "new TextDecoder()",
#elif TEXTDECODER == 1
  $UTF8Decoder: "typeof TextDecoder != 'undefined' ? new TextDecoder() : undefined",
#endif

  $UTF8ArrayToString__docs: `
  /**
   * Given a pointer 'idx' to a null-terminated UTF8-encoded string in the given
   * array that contains uint8 values, returns a copy of that string as a
   * Javascript String object.
   * heapOrArray is either a regular array, or a JavaScript typed array view.
   * @param {number=} idx
   * @param {number=} maxBytesToRead
   * @return {string}
   */`,
#if TEXTDECODER
  $UTF8ArrayToString__deps: ['$UTF8Decoder'],
#endif
  $UTF8ArrayToString: (heapOrArray, idx = 0, maxBytesToRead = NaN) => {
#if CAN_ADDRESS_2GB
    idx >>>= 0;
#endif
    var endIdx = idx + maxBytesToRead;
#if TEXTDECODER
    var endPtr = idx;
    // TextDecoder needs to know the byte length in advance, it doesn't stop on
    // null terminator by itself.  Also, use the length info to avoid running tiny
    // strings through TextDecoder, since .subarray() allocates garbage.
    // (As a tiny code save trick, compare endPtr against endIdx using a negation,
    // so that undefined/NaN means Infinity)
    while (heapOrArray[endPtr] && !(endPtr >= endIdx)) ++endPtr;
#endif // TEXTDECODER

#if TEXTDECODER == 2
    return UTF8Decoder.decode(heapOrArray.buffer ? {{{ getUnsharedTextDecoderView('heapOrArray', 'idx', 'endPtr') }}} : new Uint8Array(heapOrArray.slice(idx, endPtr)));
#else // TEXTDECODER == 2
#if TEXTDECODER
    if (endPtr - idx > 16 && heapOrArray.buffer && UTF8Decoder) {
      return UTF8Decoder.decode({{{ getUnsharedTextDecoderView('heapOrArray', 'idx', 'endPtr') }}});
    }
#endif // TEXTDECODER
    var str = '';
#if TEXTDECODER
    // If building with TextDecoder, we have already computed the string length
    // above, so test loop end condition against that
    while (idx < endPtr) {
#else
    while (!(idx >= endIdx)) {
#endif
      // For UTF8 byte structure, see:
      // http://en.wikipedia.org/wiki/UTF-8#Description
      // https://www.ietf.org/rfc/rfc2279.txt
      // https://tools.ietf.org/html/rfc3629
      var u0 = heapOrArray[idx++];
#if !TEXTDECODER
      // If not building with TextDecoder enabled, we don't know the string
      // length, so scan for \0 byte.
      // If building with TextDecoder, we know exactly at what byte index the
      // string ends, so checking for nulls here would be redundant.
      if (!u0) return str;
#endif
      if (!(u0 & 0x80)) { str += String.fromCharCode(u0); continue; }
      var u1 = heapOrArray[idx++] & 63;
      if ((u0 & 0xE0) == 0xC0) { str += String.fromCharCode(((u0 & 31) << 6) | u1); continue; }
      var u2 = heapOrArray[idx++] & 63;
      if ((u0 & 0xF0) == 0xE0) {
        u0 = ((u0 & 15) << 12) | (u1 << 6) | u2;
      } else {
#if ASSERTIONS
        if ((u0 & 0xF8) != 0xF0) warnOnce('Invalid UTF-8 leading byte ' + ptrToString(u0) + ' encountered when deserializing a UTF-8 string in wasm memory to a JS string!');
#endif
        u0 = ((u0 & 7) << 18) | (u1 << 12) | (u2 << 6) | (heapOrArray[idx++] & 63);
      }

      if (u0 < 0x10000) {
        str += String.fromCharCode(u0);
      } else {
        var ch = u0 - 0x10000;
        str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
      }
    }
    return str;
#endif // TEXTDECODER == 2
  },

  $UTF8ToString__docs: `
  /**
   * Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the
   * emscripten HEAP, returns a copy of that string as a Javascript String object.
   *
   * @param {number} ptr
   * @param {number=} maxBytesToRead - An optional length that specifies the
   *   maximum number of bytes to read. You can omit this parameter to scan the
   *   string until the first 0 byte. If maxBytesToRead is passed, and the string
   *   at [ptr, ptr+maxBytesToReadr[ contains a null byte in the middle, then the
   *   string will cut short at that byte index (i.e. maxBytesToRead will not
   *   produce a string of exact length [ptr, ptr+maxBytesToRead[) N.B. mixing
   *   frequent uses of UTF8ToString() with and without maxBytesToRead may throw
   *   JS JIT optimizations off, so it is worth to consider consistently using one
   * @return {string}
   */`,
#if TEXTDECODER == 2
  $UTF8ToString__deps: ['$UTF8Decoder'],
#else
  $UTF8ToString__deps: ['$UTF8ArrayToString'],
#endif
  $UTF8ToString: (ptr, maxBytesToRead) => {
#if ASSERTIONS
    assert(typeof ptr == 'number', `UTF8ToString expects a number (got ${typeof ptr})`);
#endif
#if CAN_ADDRESS_2GB
    ptr >>>= 0;
#endif
#if TEXTDECODER == 2
    if (!ptr) return '';
    var maxPtr = ptr + maxBytesToRead;
    for (var end = ptr; !(end >= maxPtr) && HEAPU8[end];) ++end;
    return UTF8Decoder.decode({{{ getUnsharedTextDecoderView('HEAPU8', 'ptr', 'end') }}});
#else
    return ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead) : '';
#endif
  },

  /**
   * Copies the given Javascript String object 'str' to the given byte array at
   * address 'outIdx', encoded in UTF8 form and null-terminated. The copy will
   * require at most str.length*4+1 bytes of space in the HEAP.  Use the function
   * lengthBytesUTF8 to compute the exact number of bytes (excluding null
   * terminator) that this function will write.
   *
   * @param {string} str - The Javascript string to copy.
   * @param {ArrayBufferView|Array<number>} heap - The array to copy to. Each
   *                                               index in this array is assumed
   *                                               to be one 8-byte element.
   * @param {number} outIdx - The starting offset in the array to begin the copying.
   * @param {number} maxBytesToWrite - The maximum number of bytes this function
   *                                   can write to the array.  This count should
   *                                   include the null terminator, i.e. if
   *                                   maxBytesToWrite=1, only the null terminator
   *                                   will be written and nothing else.
   *                                   maxBytesToWrite=0 does not write any bytes
   *                                   to the output, not even the null
   *                                   terminator.
   * @return {number} The number of bytes written, EXCLUDING the null terminator.
   */
  $stringToUTF8Array: (str, heap, outIdx, maxBytesToWrite) => {
#if CAN_ADDRESS_2GB
    outIdx >>>= 0;
#endif
#if ASSERTIONS
    assert(typeof str === 'string', `stringToUTF8Array expects a string (got ${typeof str})`);
#endif
    // Parameter maxBytesToWrite is not optional. Negative values, 0, null,
    // undefined and false each don't write out any bytes.
    if (!(maxBytesToWrite > 0))
      return 0;

    var startIdx = outIdx;
    var endIdx = outIdx + maxBytesToWrite - 1; // -1 for string null terminator.
    for (var i = 0; i < str.length; ++i) {
      // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code
      // unit, not a Unicode code point of the character! So decode
      // UTF16->UTF32->UTF8.
      // See http://unicode.org/faq/utf_bom.html#utf16-3
      // For UTF8 byte structure, see http://en.wikipedia.org/wiki/UTF-8#Description
      // and https://www.ietf.org/rfc/rfc2279.txt
      // and https://tools.ietf.org/html/rfc3629
      var u = str.charCodeAt(i); // possibly a lead surrogate
      if (u >= 0xD800 && u <= 0xDFFF) {
        var u1 = str.charCodeAt(++i);
        u = 0x10000 + ((u & 0x3FF) << 10) | (u1 & 0x3FF);
      }
      if (u <= 0x7F) {
        if (outIdx >= endIdx) break;
        heap[outIdx++] = u;
      } else if (u <= 0x7FF) {
        if (outIdx + 1 >= endIdx) break;
        heap[outIdx++] = 0xC0 | (u >> 6);
        heap[outIdx++] = 0x80 | (u & 63);
      } else if (u <= 0xFFFF) {
        if (outIdx + 2 >= endIdx) break;
        heap[outIdx++] = 0xE0 | (u >> 12);
        heap[outIdx++] = 0x80 | ((u >> 6) & 63);
        heap[outIdx++] = 0x80 | (u & 63);
      } else {
        if (outIdx + 3 >= endIdx) break;
#if ASSERTIONS
        if (u > 0x10FFFF) warnOnce('Invalid Unicode code point ' + ptrToString(u) + ' encountered when serializing a JS string to a UTF-8 string in wasm memory! (Valid unicode code points should be in range 0-0x10FFFF).');
#endif
        heap[outIdx++] = 0xF0 | (u >> 18);
        heap[outIdx++] = 0x80 | ((u >> 12) & 63);
        heap[outIdx++] = 0x80 | ((u >> 6) & 63);
        heap[outIdx++] = 0x80 | (u & 63);
      }
    }
    // Null-terminate the pointer to the buffer.
    heap[outIdx] = 0;
    return outIdx - startIdx;
  },

  /**
   * Copies the given Javascript String object 'str' to the emscripten HEAP at
   * address 'outPtr', null-terminated and encoded in UTF8 form. The copy will
   * require at most str.length*4+1 bytes of space in the HEAP.
   * Use the function lengthBytesUTF8 to compute the exact number of bytes
   * (excluding null terminator) that this function will write.
   *
   * @return {number} The number of bytes written, EXCLUDING the null terminator.
   */
  $stringToUTF8__deps: ['$stringToUTF8Array'],
  $stringToUTF8: (str, outPtr, maxBytesToWrite) => {
#if ASSERTIONS
    assert(typeof maxBytesToWrite == 'number', 'stringToUTF8(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
#endif
    return stringToUTF8Array(str, HEAPU8, outPtr, maxBytesToWrite);
  },

  /**
   * Returns the number of bytes the given Javascript string takes if encoded as a
   * UTF8 byte array, EXCLUDING the null terminator byte.
   *
   * @param {string} str - JavaScript string to operator on
   * @return {number} Length, in bytes, of the UTF8 encoded string.
   */
  $lengthBytesUTF8: (str) => {
    var len = 0;
    for (var i = 0; i < str.length; ++i) {
      // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code
      // unit, not a Unicode code point of the character! So decode
      // UTF16->UTF32->UTF8.
      // See http://unicode.org/faq/utf_bom.html#utf16-3
      var c = str.charCodeAt(i); // possibly a lead surrogate
      if (c <= 0x7F) {
        len++;
      } else if (c <= 0x7FF) {
        len += 2;
      } else if (c >= 0xD800 && c <= 0xDFFF) {
        len += 4; ++i;
      } else {
        len += 3;
      }
    }
    return len;
  },

  $intArrayFromString__docs: '/** @type {function(string, boolean=, number=)} */',
  $intArrayFromString__deps: ['$lengthBytesUTF8', '$stringToUTF8Array'],
  $intArrayFromString: intArrayFromString,
  $intArrayToString: intArrayToString,

  // Given a pointer 'ptr' to a null-terminated ASCII-encoded string in the
  // emscripten HEAP, returns a copy of that string as a Javascript String
  // object.
  $AsciiToString: (ptr) => {
#if CAN_ADDRESS_2GB
    ptr >>>= 0;
#endif
    var str = '';
    while (1) {
      var ch = {{{ makeGetValue('ptr++', 0, 'u8') }}};
      if (!ch) return str;
      str += String.fromCharCode(ch);
    }
  },

  // Copies the given Javascript String object 'str' to the emscripten HEAP at
  // address 'outPtr', null-terminated and encoded in ASCII form. The copy will
  // require at most str.length+1 bytes of space in the HEAP.
  $stringToAscii: (str, buffer) => {
    for (var i = 0; i < str.length; ++i) {
#if ASSERTIONS
      assert(str.charCodeAt(i) === (str.charCodeAt(i) & 0xff));
#endif
      {{{ makeSetValue('buffer++', 0, 'str.charCodeAt(i)', 'i8') }}};
    }
    // Null-terminate the string
    {{{ makeSetValue('buffer', 0, 0, 'i8') }}};
  },

#if TEXTDECODER == 2
  $UTF16Decoder: "new TextDecoder('utf-16le');",
#elif TEXTDECODER == 1
  $UTF16Decoder: "typeof TextDecoder != 'undefined' ? new TextDecoder('utf-16le') : undefined;",
#endif

  // Given a pointer 'ptr' to a null-terminated UTF16LE-encoded string in the
  // emscripten HEAP, returns a copy of that string as a Javascript String
  // object.
#if TEXTDECODER
  $UTF16ToString__deps: ['$UTF16Decoder'],
#endif
  $UTF16ToString: (ptr, maxBytesToRead) => {
#if ASSERTIONS
    assert(ptr % 2 == 0, 'Pointer passed to UTF16ToString must be aligned to two bytes!');
#endif
#if TEXTDECODER
    var endPtr = ptr;
    // TextDecoder needs to know the byte length in advance, it doesn't stop on
    // null terminator by itself.
    // Also, use the length info to avoid running tiny strings through
    // TextDecoder, since .subarray() allocates garbage.
    var idx = endPtr >> 1;
    var maxIdx = idx + maxBytesToRead / 2;
    // If maxBytesToRead is not passed explicitly, it will be undefined, and this
    // will always evaluate to true. This saves on code size.
    while (!(idx >= maxIdx) && HEAPU16[idx]) ++idx;
    endPtr = idx << 1;

#if TEXTDECODER != 2
    if (endPtr - ptr > 32 && UTF16Decoder)
#endif // TEXTDECODER != 2
      return UTF16Decoder.decode({{{ getUnsharedTextDecoderView('HEAPU8', 'ptr', 'endPtr') }}});
#endif // TEXTDECODER

#if TEXTDECODER != 2
    // Fallback: decode without UTF16Decoder
    var str = '';

    // If maxBytesToRead is not passed explicitly, it will be undefined, and the
    // for-loop's condition will always evaluate to true. The loop is then
    // terminated on the first null char.
    for (var i = 0; !(i >= maxBytesToRead / 2); ++i) {
      var codeUnit = {{{ makeGetValue('ptr', 'i*2', 'i16') }}};
      if (codeUnit == 0) break;
      // fromCharCode constructs a character from a UTF-16 code unit, so we can
      // pass the UTF16 string right through.
      str += String.fromCharCode(codeUnit);
    }

    return str;
#endif // TEXTDECODER != 2
  },

  // Copies the given Javascript String object 'str' to the emscripten HEAP at
  // address 'outPtr', null-terminated and encoded in UTF16 form. The copy will
  // require at most str.length*4+2 bytes of space in the HEAP.  Use the
  // function lengthBytesUTF16() to compute the exact number of bytes (excluding
  // null terminator) that this function will write.
  // Parameters:
  //   str: the Javascript string to copy.
  //   outPtr: Byte address in Emscripten HEAP where to write the string to.
  //   maxBytesToWrite: The maximum number of bytes this function can write to
  //                    the array. This count should include the null
  //                    terminator, i.e. if maxBytesToWrite=2, only the null
  //                    terminator will be written and nothing else.
  //                    maxBytesToWrite<2 does not write any bytes to the
  //                    output, not even the null terminator.
  // Returns the number of bytes written, EXCLUDING the null terminator.
  $stringToUTF16: (str, outPtr, maxBytesToWrite) => {
#if ASSERTIONS
    assert(outPtr % 2 == 0, 'Pointer passed to stringToUTF16 must be aligned to two bytes!');
#endif
#if ASSERTIONS
    assert(typeof maxBytesToWrite == 'number', 'stringToUTF16(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
#endif
    // Backwards compatibility: if max bytes is not specified, assume unsafe unbounded write is allowed.
    maxBytesToWrite ??= 0x7FFFFFFF;
    if (maxBytesToWrite < 2) return 0;
    maxBytesToWrite -= 2; // Null terminator.
    var startPtr = outPtr;
    var numCharsToWrite = (maxBytesToWrite < str.length*2) ? (maxBytesToWrite / 2) : str.length;
    for (var i = 0; i < numCharsToWrite; ++i) {
      // charCodeAt returns a UTF-16 encoded code unit, so it can be directly written to the HEAP.
      var codeUnit = str.charCodeAt(i); // possibly a lead surrogate
      {{{ makeSetValue('outPtr', 0, 'codeUnit', 'i16') }}};
      outPtr += 2;
    }
    // Null-terminate the pointer to the HEAP.
    {{{ makeSetValue('outPtr', 0, 0, 'i16') }}};
    return outPtr - startPtr;
  },

  // Returns the number of bytes the given Javascript string takes if encoded as
  // a UTF16 byte array, EXCLUDING the null terminator byte.
  $lengthBytesUTF16: (str) => {
    return str.length*2;
  },

  $UTF32ToString: (ptr, maxBytesToRead) => {
#if ASSERTIONS
    assert(ptr % 4 == 0, 'Pointer passed to UTF32ToString must be aligned to four bytes!');
#endif
    var i = 0;

    var str = '';
    // If maxBytesToRead is not passed explicitly, it will be undefined, and this
    // will always evaluate to true. This saves on code size.
    while (!(i >= maxBytesToRead / 4)) {
      var utf32 = {{{ makeGetValue('ptr', 'i*4', 'i32') }}};
      if (utf32 == 0) break;
      ++i;
      // Gotcha: fromCharCode constructs a character from a UTF-16 encoded code (pair), not from a Unicode code point! So encode the code point to UTF-16 for constructing.
      // See http://unicode.org/faq/utf_bom.html#utf16-3
      if (utf32 >= 0x10000) {
        var ch = utf32 - 0x10000;
        str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
      } else {
        str += String.fromCharCode(utf32);
      }
    }
    return str;
  },

  // Copies the given Javascript String object 'str' to the emscripten HEAP at
  // address 'outPtr', null-terminated and encoded in UTF32 form. The copy will
  // require at most str.length*4+4 bytes of space in the HEAP.
  // Use the function lengthBytesUTF32() to compute the exact number of bytes
  // (excluding null terminator) that this function will write.
  // Parameters:
  //   str: the Javascript string to copy.
  //   outPtr: Byte address in Emscripten HEAP where to write the string to.
  //   maxBytesToWrite: The maximum number of bytes this function can write to
  //                    the array. This count should include the null
  //                    terminator, i.e. if maxBytesToWrite=4, only the null
  //                    terminator will be written and nothing else.
  //                    maxBytesToWrite<4 does not write any bytes to the
  //                    output, not even the null terminator.
  // Returns the number of bytes written, EXCLUDING the null terminator.
  $stringToUTF32: (str, outPtr, maxBytesToWrite) => {
#if CAN_ADDRESS_2GB
    outPtr >>>= 0;
#endif
#if ASSERTIONS
    assert(outPtr % 4 == 0, 'Pointer passed to stringToUTF32 must be aligned to four bytes!');
#endif
#if ASSERTIONS
    assert(typeof maxBytesToWrite == 'number', 'stringToUTF32(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
#endif
    // Backwards compatibility: if max bytes is not specified, assume unsafe unbounded write is allowed.
    maxBytesToWrite ??= 0x7FFFFFFF;
    if (maxBytesToWrite < 4) return 0;
    var startPtr = outPtr;
    var endPtr = startPtr + maxBytesToWrite - 4;
    for (var i = 0; i < str.length; ++i) {
      // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! We must decode the string to UTF-32 to the heap.
      // See http://unicode.org/faq/utf_bom.html#utf16-3
      var codeUnit = str.charCodeAt(i); // possibly a lead surrogate
      if (codeUnit >= 0xD800 && codeUnit <= 0xDFFF) {
        var trailSurrogate = str.charCodeAt(++i);
        codeUnit = 0x10000 + ((codeUnit & 0x3FF) << 10) | (trailSurrogate & 0x3FF);
      }
      {{{ makeSetValue('outPtr', 0, 'codeUnit', 'i32') }}};
      outPtr += 4;
      if (outPtr + 4 > endPtr) break;
    }
    // Null-terminate the pointer to the HEAP.
    {{{ makeSetValue('outPtr', 0, 0, 'i32') }}};
    return outPtr - startPtr;
  },

  // Returns the number of bytes the given Javascript string takes if encoded as
  // a UTF16 byte array, EXCLUDING the null terminator byte.
  $lengthBytesUTF32: (str) => {
    var len = 0;
    for (var i = 0; i < str.length; ++i) {
      // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! We must decode the string to UTF-32 to the heap.
      // See http://unicode.org/faq/utf_bom.html#utf16-3
      var codeUnit = str.charCodeAt(i);
      if (codeUnit >= 0xD800 && codeUnit <= 0xDFFF) ++i; // possibly a lead surrogate, so skip over the tail surrogate.
      len += 4;
    }

    return len;
  },

  // Allocate heap space for a JS string, and write it there.
  // It is the responsibility of the caller to free() that memory.
  $stringToNewUTF8__deps: ['$lengthBytesUTF8', '$stringToUTF8', 'malloc'],
  $stringToNewUTF8: (str) => {
    var size = lengthBytesUTF8(str) + 1;
    var ret = _malloc(size);
    if (ret) stringToUTF8(str, ret, size);
    return ret;
  },

  // Allocate stack space for a JS string, and write it there.
  $stringToUTF8OnStack__deps: ['$lengthBytesUTF8', '$stringToUTF8', '$stackAlloc'],
  $stringToUTF8OnStack: (str) => {
    var size = lengthBytesUTF8(str) + 1;
    var ret = stackAlloc(size);
    stringToUTF8(str, ret, size);
    return ret;
  },

  $writeArrayToMemory: (array, buffer) => {
#if ASSERTIONS
    assert(array.length >= 0, 'writeArrayToMemory array must have a length (should be an array or typed array)')
#endif
    HEAP8.set(array, buffer);
  },
});
