// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>

int main() {
  const char *hello = "hello world";
  int result = 1;
  putc('g', stdout);
  putc('o', stdout);
  putc('!', stdout);
  if (strchr(hello, 'l') != hello + 2) result = 0;
  if (strchr(hello, 'w') != hello + 6) result = 0;
  putc('d', stdout);
  putc('o', stdout);
  putc('n', stdout);
  putc('e', stdout);
  REPORT_RESULT(result);
  return 0;
}

