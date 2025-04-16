/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <locale.h>
#include <errno.h>
#include <assert.h>

void test_vswprintf(const wchar_t *format, ...) {
  wchar_t buffer[256];
  va_list args;
  va_start(args, format);
  int n = vswprintf(buffer, 256, format, args);
  fputws(buffer, stdout);
  va_end(args);
  wprintf(L"number of characters in above string: %d. errno: %d\n", n, errno);
  assert(n == 24);
  assert(errno == 0);
}

int main() {
  setlocale(LC_ALL, "");
  /* Unicode Character 'LATIN SMALL LETTER O WITH DIAERESIS' (U+00F6):
   * http://www.fileformat.info/info/unicode/char/00f6/index.htm */
  wint_t wint = 0xF6;
  wchar_t wchar = 0xF6;
  printf("This is a wint: %lc.\n", wint);
  test_vswprintf(L"This is a character: %lc.\n", wint, wchar);
  return 0;
}
