/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/emscripten.h>
#include <time.h>
#include <stdbool.h>
#include <sys/time.h>
#include <threads.h>
#include "libc.h"

// Replaces musl's __tz.c

__attribute__((__weak__)) long  timezone = 0;
__attribute__((__weak__)) int   daylight = 0;
__attribute__((__weak__)) char *tzname[2] = { 0, 0 };

void _tzset_js(long* timezone, int* daylight, char** tzname);
time_t _timegm_js(struct tm *tm);
time_t _mktime_js(struct tm *tm);
void _localtime_js(const time_t *restrict t, struct tm *restrict tm);
void _gmtime_js(const time_t *restrict t, struct tm *restrict tm);
double _emscripten_date_now();
double emscripten_get_now_res();

__attribute__((__weak__))
void tzset() {
  _tzset_js(&timezone, &daylight, tzname);
}

__attribute__((__weak__))
time_t timegm(struct tm *tm) {
  tzset();
  return _timegm_js(tm);
}

__attribute__((__weak__))
time_t mktime(struct tm *tm) {
  tzset();
  return _mktime_js(tm);
}

__attribute__((__weak__))
struct tm *__localtime_r(const time_t *restrict t, struct tm *restrict tm) {
  tzset();
  _localtime_js(t, tm);
  // __localtime_js sets everything but the tmzone pointer
  tm->__tm_zone = tm->tm_isdst ? tzname[1] :tzname[0];
  return tm;
}

__attribute__((__weak__))
struct tm *__gmtime_r(const time_t *restrict t, struct tm *restrict tm) {
  tzset();
  _gmtime_js(t, tm);
  tm->tm_isdst = 0;
  tm->__tm_gmtoff = 0;
  tm->__tm_zone = "GMT";
  return tm;
}

__attribute__((__weak__))
clock_t __clock() {
  static thread_local double start = 0;
  if (!start) {
    start = _emscripten_date_now();
  }
  return (_emscripten_date_now() - start) * (CLOCKS_PER_SEC / 1000);
}

__attribute__((__weak__))
time_t __time(time_t *t) {
  double ret = _emscripten_date_now() / 1000;
  if (t) {
    *t = ret;
  }
  return ret;
}

extern bool _emscripten_get_now_is_monotonic();
static thread_local bool checked_monotonic = false;
static thread_local bool is_monotonic = 0;

__attribute__((__weak__))
int __clock_gettime(clockid_t clk, struct timespec *ts) {
  if (!checked_monotonic) {
    is_monotonic = _emscripten_get_now_is_monotonic();
    checked_monotonic = true;
  }

  double now_ms;
  if (clk == CLOCK_REALTIME) {
    now_ms = _emscripten_date_now();
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

__attribute__((__weak__))
int __clock_getres(clockid_t clk, struct timespec *ts) {
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

__attribute__((__weak__))
int __gettimeofday(struct timeval *restrict tv, void *restrict tz) {
  double now_ms = _emscripten_date_now();
  long long now_s = now_ms / 1000;
  tv->tv_sec = now_s; // seconds
  tv->tv_usec = (now_ms - (now_s * 1000)) * 1000; // nicroseconds
  return 0;
}

__attribute__((__weak__))
int dysize(int year) {
  int leap = ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)));
  return leap ? 366 : 365;
}

weak_alias(__gmtime_r, gmtime_r);
weak_alias(__localtime_r, localtime_r);
weak_alias(__time, time);
weak_alias(__clock, clock);
weak_alias(__clock_gettime, clock_gettime);
weak_alias(__clock_getres, clock_getres);
weak_alias(__gettimeofday, gettimeofday);
