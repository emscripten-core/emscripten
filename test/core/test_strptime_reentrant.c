/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// glibc requires _XOPEN_SOURCE to be defined in order to get strptime.
#define _XOPEN_SOURCE
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(__GLIBC__) || (defined(__EMSCRIPTEN__) && !defined(STANDALONE))
// Not all implementations support this (for example, upstream musl)
#define HAVE_WDAY
#endif

int main() {
  int result = 0;
  struct tm tm;

  memset(&tm, 0xaa, sizeof(tm));

  /* Test we don't crash on uninitialized struct tm.
     Some fields might contain bogus values until everything
     needed is initialized, but we shouldn't crash.  */
  if (strptime("2007", "%Y", &tm) == NULL ||
      strptime("12", "%d", &tm) == NULL || strptime("Feb", "%b", &tm) == NULL ||
      strptime("13", "%M", &tm) == NULL || strptime("21", "%S", &tm) == NULL ||
      strptime("16", "%H", &tm) == NULL) {
    printf("ERR: returned NULL\n");
    exit(EXIT_FAILURE);
  }

  if (tm.tm_sec != 21 || tm.tm_min != 13 || tm.tm_hour != 16 ||
      tm.tm_mday != 12 || tm.tm_mon != 1 || tm.tm_year != 107) {
    printf("ERR: unexpected tm content (1) - %d/%d/%d %d:%d:%d\n", tm.tm_mon + 1,
           tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
    exit(EXIT_FAILURE);
  }
#ifdef HAVE_WDAY
  assert(tm.tm_wday == 1);
  assert(tm.tm_yday == 42);
#endif

  if (strptime("8", "%d", &tm) == NULL) {
    printf("ERR: strptime failed");
    exit(EXIT_FAILURE);
  }

  if (tm.tm_sec != 21 || tm.tm_min != 13 || tm.tm_hour != 16 ||
      tm.tm_mday != 8 || tm.tm_mon != 1 || tm.tm_year != 107) {
    printf("ERR: unexpected tm content (2) - %d/%d/%d %d:%d:%d\n", tm.tm_mon + 1,
           tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
    exit(EXIT_FAILURE);
  }
#ifdef HAVE_WDAY
  assert(tm.tm_wday == 4);
  assert(tm.tm_yday == 38);
#endif

  printf("OK\n");
}
