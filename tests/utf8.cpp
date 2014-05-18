#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <iostream>
#include <cassert>
#include <emscripten.h>

// This code tests that Unicode std::wstrings can be marshalled between C++ and JS.
int main() {
  const char asciiString[] = "Hello world!";
  char asciiString2[128] = {};
  EM_ASM_INT({
    var str = Module.AsciiToString($0);
    Module.print(str);
    Module.stringToAscii(str, $1);
  }, asciiString, asciiString2);
  assert(!strcmp(asciiString, asciiString2));

  char asciiString3[128] = {};
  EM_ASM_INT({
    var str = Module.UTF8ToString($0);
    Module.print(str);
    Module.stringToUTF8(str, $1);
  }, asciiString, asciiString3);
  assert(!strcmp(asciiString, asciiString3));

  const char utf8String[] = u8"Hyv\u00E4\u00E4 p\u00E4iv\u00E4\u00E4! T\u00F6\u00F6\u00F6\u00F6t! abc\u2603\u20AC\U0002007C123 --- abc\u2603\u20AC\U0002007C123."; // U+2603 is snowman, U+20AC is the Euro sign, U+2007C is a Chinese Han character that looks like three raindrops.
  char utf8String2[128] = {};
  EM_ASM_INT({
    var str = Module.UTF8ToString($0);
    Module.print(str);
    Module.stringToUTF8(str, $1);
  }, utf8String, utf8String2);
  assert(strlen(utf8String) == strlen(utf8String2));
  for(int i = 0; i < strlen(utf8String)+1; ++i)
    if (utf8String[i] != utf8String2[i])
      printf("i=%d:%u,%u\n", i, (unsigned int)(unsigned char)utf8String[i], (unsigned int)(unsigned char)utf8String2[i]);
  assert(!strcmp(utf8String, utf8String2));

  printf("OK.\n");
}
