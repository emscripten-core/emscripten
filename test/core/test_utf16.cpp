// Copyright 2020 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <cassert>
#include <emscripten.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <wchar.h>

EM_JS_DEPS(deps, "$UTF16ToString,$stringToUTF16");

// Roundtrip a (non-)null-terminated string between C++ and JS.
EM_JS(void, roundtripString, (const char16_t* str, int strBytes, char16_t* result, int resultBytes), {
#if __wasm64__
  // TODO(sbc): Use a library helper to do this conversions
  // See: https://github.com/emscripten-core/emscripten/pull/16787
  str = Number(str);
  strBytes = Number(strBytes);
  result = Number(result);
#endif
  var jsStr = UTF16ToString(str, strBytes >= 0 ? strBytes : undefined);
  out(jsStr);
  var bytesWritten = stringToUTF16(jsStr, result, resultBytes);
  if (bytesWritten != resultBytes - 2) throw 'stringToUTF16 wrote an invalid length: ' + numBytesWritten;
});

static void testString(const char16_t* arg) {
  // Test with null-terminated string.
  std::u16string strz(arg);
  char16_t* result = new char16_t[strz.size() + 1]();
  int resultBytes = (strz.size() + 1) * sizeof(char16_t);

  roundtripString(strz.data(), -1, result, resultBytes);
  // Compare strings after taking a route through JS side.
  assert(std::equal(result, result + strz.size() + 1, strz.data()));

  // Same test with non-null-terminated string and explicit length.
  std::vector<char16_t> str(strz.begin(), strz.end());
  std::fill_n(result, str.size() + 1, 0);

  roundtripString(str.data(), str.size() * sizeof(char16_t), result, resultBytes);
  assert(std::equal(result, result + str.size(), str.data()));

  // Test again with some garbage at the end of the string.
  std::vector<char16_t> strx(str);
  strx.insert(strx.end(), 10, 'x');
  std::fill_n(result, str.size() + 1, 0);

  roundtripString(strx.data(), str.size() * sizeof(char16_t), result, resultBytes);
  assert(std::equal(result, result + str.size(), str.data()));

  delete[] result;
}

// This code tests that UTF16 strings can be marshalled between C++ and JS.
int main() {
  // For the conversion of long strings (more than 32 bytes), TextDecoder can be used.
  testString(u"abc\u2603\u20AC\U0002007C123 --- abc\u2603\u20AC\U0002007C123");

  // But for shorter strings it's never used.
  testString(u"short\u2603\u20AC\U0002007C123");
  testString(u"a");
  testString(u"");

  printf("OK.\n");
}
