/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// glibc requires _XOPEN_SOURCE to be defined in order to get strptime.
#define _XOPEN_SOURCE
// glibc requires _DEFAULT_SOURCE to be defined in order to get tm_gmtoff.
#define _DEFAULT_SOURCE

#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#if defined(__GLIBC__) || (defined(__EMSCRIPTEN__) && !defined(STANDALONE))
// Not all implementations support these (for example, upstream musl)
#define HAVE_WDAY
#define HAVE_TIMEZONE
#endif

#define STRPTIME(a, b, c) assert(strptime(a, b, c) != 0)

void ReadMonth(const char *month) {
  struct tm value = {0};
  STRPTIME(month, "%b", &value);
  printf("%s: %d\n", month, value.tm_mon);
}

int main() {
  struct tm tm;
  STRPTIME("17410105012000", "%H%M%S%d%m%Y", &tm);

  printf("%d/%d/%d %d:%d:%d\n",
      tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min,
      tm.tm_sec);
#ifdef HAVE_WDAY
  assert(tm.tm_wday == 3);
#endif

  ReadMonth("jan");
  ReadMonth("january");
  ReadMonth("feb");
  ReadMonth("february");
  ReadMonth("march");
  ReadMonth("mar");
  ReadMonth("april");
  ReadMonth("may");
  ReadMonth("may");
  ReadMonth("june");
  ReadMonth("jul");
  ReadMonth("august");
  ReadMonth("september");
  ReadMonth("oct");
  ReadMonth("nov");
  ReadMonth("november");
  ReadMonth("december");


  // check that %% is handled correctly for normal strings
  STRPTIME("2020-05-01T00:01%z","%Y-%m-%dT%H:%M%%z",&tm);
  printf("%d\n",tm.tm_min);

  // check that %% is handled correctly even if the letter after it is
  // in EQUIVALENT_MATCHERS
  STRPTIME("%D2020-05-01T00:01","%%D%Y-%m-%dT%H:%M",&tm);
  printf("%d,%d\n",tm.tm_year+1900,tm.tm_min);


  // check that EQUIVALENT_MATCHERS works
  // %c == %a %b %d %H:%M:%S %Y
  STRPTIME("Sun March 31 12:34:56 2345","%c",&tm);
  printf("%d,%d,%d,%d,%d,%d\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);

  // check that EQUIVALENT_MATCHERS works twice
  //      'T':  '%H\\:%M\\:%S',
  //      'D':  '%m\\/%d\\/%y',
  STRPTIME("12:34:56 01/02/03","%T %D",&tm);
  printf("%d,%d,%d,%d,%d,%d\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);

  // check regex special characters don't break things
  STRPTIME(".?12:34:56 01/02/03",".?%T %D",&tm);
  printf("%d,%d,%d,%d,%d,%d\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);


#ifdef HAVE_TIMEZONE
  // check timezone offsets
  STRPTIME("2020-05-01T00:00+0100","%Y-%m-%dT%H:%M%z",&tm);
  assert(tm.tm_gmtoff == 3600);

  STRPTIME("2020-05-01T00:00Z","%Y-%m-%dT%H:%M%z",&tm);
  assert(tm.tm_gmtoff == 0);

  STRPTIME("2020-05-01T00:00-02:30","%Y-%m-%dT%H:%M%z",&tm);
  assert(tm.tm_gmtoff == -9000);
#endif

  // check that the numbers of spaces in format string are ignored
  STRPTIME("12     34     56","%H %M %S",&tm);
  printf("%d,%d,%d\n",tm.tm_hour,tm.tm_min,tm.tm_sec);

  STRPTIME("123456","%H %M %S",&tm);
  printf("%d,%d,%d\n",tm.tm_hour,tm.tm_min,tm.tm_sec);

  STRPTIME("12 34    56","%H   %M %S",&tm);
  printf("%d,%d,%d\n",tm.tm_hour,tm.tm_min,tm.tm_sec);

  return 0;
}
