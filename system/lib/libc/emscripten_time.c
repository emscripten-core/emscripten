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

weak clock_t __clock() {
  static thread_local double start = 0;
  if (!start) {
    start = emscripten_date_now();
  }
  return (emscripten_date_now() - start) * (CLOCKS_PER_SEC / 1000);
}

weak time_t __time(time_t *t) {
  double ret = emscripten_date_now() / 1000;
  if (t) {
    *t = ret;
  }
  return ret;
}

static thread_local bool checked_monotonic = false;
static thread_local bool is_monotonic = 0;

weak int __clock_gettime(clockid_t clk, struct timespec *ts) {
  if (!checked_monotonic) {
    is_monotonic = _emscripten_get_now_is_monotonic();
    checked_monotonic = true;
  }

  double now_ms;
  if (clk == CLOCK_REALTIME) {
    now_ms = emscripten_date_now();
  } else if ((clk == CLOCK_MONOTONIC || clk == CLOCK_MONOTONIC_RAW) && is_monotonic) {
    now_ms = emscripten_get_now();
  } else {
    errno = EINVAL;
    return -1;
  }

  long long now_s = now_ms / 1000;
  ts->tv_sec = now_s; // seconds
  ts->tv_nsec = (now_ms - (now_s * 1000)) * 1000 * 1000; // nanoseconds
  return 0;
}

weak int __clock_getres(clockid_t clk, struct timespec *ts) {
  if (!checked_monotonic) {
    is_monotonic = _emscripten_get_now_is_monotonic();
    checked_monotonic = true;
  }

  double nsec;
  if (clk == CLOCK_REALTIME) {
    nsec = 1000 * 1000; // educated guess that it's milliseconds
  } else if (clk == CLOCK_MONOTONIC && is_monotonic) {
    nsec = emscripten_get_now_res();
  } else {
    errno = EINVAL;
    return -1;
  }
  ts->tv_sec = (nsec / (1000 * 1000 * 1000));
  ts->tv_nsec = nsec;
  return 0;
}

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
weak_alias(__clock, clock);
weak_alias(__clock_gettime, clock_gettime);
weak_alias(__clock_getres, clock_getres);
weak_alias(__gettimeofday, gettimeofday);
