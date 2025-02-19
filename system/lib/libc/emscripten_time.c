/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <time.h>
#include <stdbool.h>
#include <sys/time.h>
#include <threads.h>
#include "libc.h"

#include "emscripten_internal.h"

weak time_t __time(time_t *t) {
  double ret = emscripten_date_now() / 1000;
  if (t) {
    *t = ret;
  }
  return ret;
}

static thread_local bool checked_monotonic = false;
static thread_local bool is_monotonic = 0;

weak int __gettimeofday(struct timeval *restrict tv, void *restrict tz) {
  double now_ms = emscripten_date_now();
  long long now_s = now_ms / 1000;
  tv->tv_sec = now_s; // seconds
  tv->tv_usec = (now_ms - (now_s * 1000)) * 1000; // nicroseconds
  return 0;
}

weak int dysize(int year) {
  int leap = ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)));
  return leap ? 366 : 365;
}

weak_alias(__time, time);
weak_alias(__gettimeofday, gettimeofday);
