// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>
#include <emscripten.h>

EM_JS_DEPS(deps, "$UTF8ToString,$stringToUTF8,$lengthBytesUTF8,$AsciiToString,$stringToAscii");

// This code tests that Unicode std::wstrings can be marshalled between C++ and JS.
int main() {
  const char latin1String[] = "\x26\xA0\xF7";
  EM_ASM({
    var str = AsciiToString($0);
    assert(str === "\\x26\\xA0\\xF7");
  }, latin1String);

  const char asciiString[] = "Hello, world!";
  char asciiString2[128] = {};
  EM_ASM({
    var str = AsciiToString($0);
    out("stringToAscii " + $0 + " -> " + str);
    stringToAscii(str, $1);
  }, asciiString, asciiString2);
  assert(!strcmp(asciiString, asciiString2));

  char asciiString3[128] = {};
  EM_ASM({
    var str = UTF8ToString($0);
    out(str);
    var numBytesWritten = stringToUTF8(str, $1, $2);
    if (numBytesWritten != 13) throw 'stringToUTF8 wrote an invalid length ' + numBytesWritten;
  }, asciiString, asciiString3, 128);
  assert(!strcmp(asciiString, asciiString3));

  const char utf8String[] = u8"Hyv\u00E4\u00E4 p\u00E4iv\u00E4\u00E4! T\u00F6\u00F6\u00F6\u00F6t! abc\u2603\u20AC\U0002007C123 --- abc\u2603\u20AC\U0002007C123."; // U+2603 is snowman, U+20AC is the Euro sign, U+2007C is a Chinese Han character that looks like three raindrops.
  char utf8String2[128] = {};
  EM_ASM({
    var str = UTF8ToString($0);
    out(str);
    var numBytesWritten = stringToUTF8(str, $1, $2);
    if (numBytesWritten != 69) throw 'stringToUTF8 wrote an invalid length ' + numBytesWritten;
  }, utf8String, utf8String2, 128);
  assert(strlen(utf8String) == strlen(utf8String2));
  for (int i = 0; i < strlen(utf8String)+1; ++i) {
    if (utf8String[i] != utf8String2[i]) {
      printf("i=%d:%u,%u\n", i, (unsigned int)(unsigned char)utf8String[i], (unsigned int)(unsigned char)utf8String2[i]);
    }
  }
  assert(!strcmp(utf8String, utf8String2));

  // Test that text gets properly cut off if output buffer is too small.
  EM_ASM({
    var str = UTF8ToString($0);
    out(str);
    var numBytesWritten = stringToUTF8(str, $1, $2);
    if (numBytesWritten != 9) throw 'stringToUTF8 wrote an invalid length ' + numBytesWritten;
  }, utf8String, utf8String2, 10);
  assert(strlen(utf8String2) == 9);

  // Zero-length string.
  EM_ASM({
    var str = UTF8ToString($0);
    out(str);
    var numBytesWritten = stringToUTF8(str, $1, $2);
    if (numBytesWritten != 0) throw 'stringToUTF8 wrote an invalid length ' + numBytesWritten;
  }, utf8String, utf8String2, 1);
  assert(utf8String2[0] == 0);

  // No bytes output.
  utf8String2[0] = 'X';
  EM_ASM({
    var str = UTF8ToString($0);
    out(str);
    var numBytesWritten = stringToUTF8(str, $1, $2);
    if (numBytesWritten != 0) throw 'stringToUTF8 wrote an invalid length ' + numBytesWritten;
  }, utf8String, utf8String2, 0);
  assert(utf8String2[0] == 'X');

  // Lone surrogates (https://github.com/emscripten-core/emscripten/issues/28008)
  EM_ASM({
    for (var str of ["\\uD800", "ty\\uD800", "\\uD800\\u00E9", "\\uD800\\u20AC", "\\uDC00", "x\\uD800y", "\\uD83D\\uDE00"]) {
      var expectedLen = lengthBytesUTF8(str);
      var numBytesWritten = stringToUTF8(str, $0, $1);
      if (numBytesWritten != expectedLen) {
        throw 'lengthBytesUTF8 (' + expectedLen + ') != stringToUTF8 (' + numBytesWritten + ')';
      }
    }
  }, utf8String2, 128);

  // Empty and partial-length reads on .rodata strings (including maxBytesToRead == 0).
  const char* emptyRoStr = "";
  const char* roStr = "rodata string";
  EM_ASM({
    if (UTF8ToString($0) !== "") throw 'expected empty rodata string';
    if (UTF8ToString($0, 0) !== "") throw 'expected empty string for len 0';
    if (UTF8ToString($1) !== "rodata string") throw 'expected full rodata string';
    if (UTF8ToString($1, 0) !== "") throw 'expected empty string when maxBytesToRead is 0';
    if (UTF8ToString($1, 6) !== "rodata") throw 'expected prefix for maxBytesToRead 6';
    if (UTF8ToString($1) !== "rodata string") throw 'expected full rodata string again';
  }, emptyRoStr, roStr);

  // Mutable .data and stack strings must not be cached.
  static char dataStr[] = "mutable data 1";
  char stackStr[] = "mutable stack 1";
  EM_ASM({
    if (UTF8ToString($0) !== "mutable data 1") throw 'expected mutable data 1';
    if (UTF8ToString($1) !== "mutable stack 1") throw 'expected mutable stack 1';
    if (AsciiToString($0) !== "mutable data 1") throw 'expected ascii mutable data 1';
    if (AsciiToString($1) !== "mutable stack 1") throw 'expected ascii mutable stack 1';
  }, dataStr, stackStr);
  dataStr[13] = '2';
  stackStr[14] = '2';
  EM_ASM({
    if (UTF8ToString($0) !== "mutable data 2") throw 'expected mutable data 2';
    if (UTF8ToString($1) !== "mutable stack 2") throw 'expected mutable stack 2';
    if (AsciiToString($0) !== "mutable data 2") throw 'expected ascii mutable data 2';
    if (AsciiToString($1) !== "mutable stack 2") throw 'expected ascii mutable stack 2';
  }, dataStr, stackStr);

  // UTF8ToString and AsciiToString must not collide in the rodata cache.
  const char* nonAsciiRoStr = u8"caf\u00e9";
  EM_ASM({
    if (UTF8ToString($0) !== "caf\u00e9") throw 'expected utf8 cafe';
    if (AsciiToString($0) !== "caf\u00c3\u00a9") throw 'expected latin1 cafe';
    if (UTF8ToString($0) !== "caf\u00e9") throw 'expected utf8 cafe again';
  }, nonAsciiRoStr);

#ifdef CHECK_RODATA_CACHE
  // Verify .rodata strings (including empty strings) are served from the cache
  // without re-reading memory, while explicit maxBytesToRead bypasses the cache.
  const char* cachedStr = "cached rodata";
  const char* cachedAsciiStr = "cached ascii";
  EM_ASM({
    var emptyPtr = $0;
    if (UTF8ToString(emptyPtr) !== "") throw 'expected empty rodata string';
    if (AsciiToString(emptyPtr) !== "") throw 'expected empty ascii rodata string';
    HEAPU8[emptyPtr] = 88; // 'X'
    if (UTF8ToString(emptyPtr) !== "") throw 'expected cache hit on empty rodata';
    if (AsciiToString(emptyPtr) !== "") throw 'expected ascii cache hit on empty rodata';
    HEAPU8[emptyPtr] = 0;

    var ptr = $1;
    if (UTF8ToString(ptr) !== "cached rodata") throw 'expected cached rodata';
    var orig = HEAPU8[ptr];
    HEAPU8[ptr] = 88; // 'X'
    if (UTF8ToString(ptr) !== "cached rodata") throw 'expected cache hit on rodata';
    if (UTF8ToString(ptr, 0) !== "") throw 'expected cache bypass for maxBytesToRead 0';
    if (UTF8ToString(ptr, 6) !== "Xached") throw 'expected cache bypass for maxBytesToRead 6';
    HEAPU8[ptr] = orig;

    var asciiPtr = $2;
    if (AsciiToString(asciiPtr) !== "cached ascii") throw 'expected cached ascii';
    var origAscii = HEAPU8[asciiPtr];
    HEAPU8[asciiPtr] = 88; // 'X'
    if (AsciiToString(asciiPtr) !== "cached ascii") throw 'expected ascii cache hit on rodata';
    HEAPU8[asciiPtr] = origAscii;
  }, emptyRoStr, cachedStr, cachedAsciiStr);
#endif

  printf("done\n");
}
