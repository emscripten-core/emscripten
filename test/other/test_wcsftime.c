// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

int main() {
  struct tm tm;
  wchar_t s[1000];

  // Ensure all fields of `tm` are initialized correctly
  // before we start messing with them.
  time_t t = 0;
  gmtime_r(&t, &tm);

  tm.tm_sec = 4;
  tm.tm_min = 23;
  tm.tm_hour = 20;
  tm.tm_mday = 21;
  tm.tm_mon = 1;
  tm.tm_year = 74;
  tm.tm_wday = 4;
  tm.tm_yday = 51;
  tm.tm_isdst = 0;

  const wchar_t *fmt = L"%m/%d/%Y %H:%M:%S %Z";
  wcsftime(s, sizeof(s), fmt, &tm);
  printf("%ls -> %ls\n", fmt, s);

  return 0;
}
