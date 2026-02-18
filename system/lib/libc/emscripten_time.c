/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/html5.h> // For `emscripten_date_now`
#include <time.h>
#include <sys/time.h>

#include "libc.h" // For `weak`

// emscripten-specific implementations. These works out slightly smaller than
// musl's since it they avoid depending on the more general clock_gettime.
weak time_t time(time_t *t) {
  double ret = emscripten_date_now() / 1000;
  if (t) {
    *t = ret;
  }
  return ret;
}

weak int gettimeofday(struct timeval *restrict tv, void *restrict tz) {
  double now_ms = emscripten_date_now();
  long long now_s = now_ms / 1000;
  tv->tv_sec = now_s; // seconds
  tv->tv_usec = (now_ms - (now_s * 1000)) * 1000; // microseconds
  return 0;
}

// Non-standard glibc/BSD extension that is not implemented by musl
weak int dysize(int year) {
  int leap = ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)));
  return leap ? 366 : 365;
}
