// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>
#include <emscripten.h>

EM_JS_DEPS(deps, "$UTF8ToString,$stringToUTF8,$AsciiToString,$stringToAscii");

// This code tests that Unicode std::wstrings can be marshalled between C++ and JS.
int main() {
  const char latin1String[] = "\x26\xA0\xF7";
  EM_ASM({
    var str = AsciiToString($0);
    assert(str === "\\x26\\xA0\\xF7");
  }, latin1String);

  const char asciiString[] = "Hello world!";
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
    if (numBytesWritten != 12) throw 'stringToUTF8 wrote an invalid length ' + numBytesWritten;
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

  printf("OK.\n");
}
