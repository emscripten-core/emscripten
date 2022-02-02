/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <time.h>
#include <stdbool.h>
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

weak_alias(__gmtime_r, gmtime_r);
weak_alias(__localtime_r, localtime_r);
