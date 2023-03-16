/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <time.h>

// Declare these functions `int` rather than time_t to avoid int64 at the wasm
// boundary (avoids 64-bit complexity at the boundary when WASM_BIGINT is
// missing).
// TODO(sbc): Covert back to `time_t` before 2038 ...
int _timegm_js(struct tm *tm);
int _mktime_js(struct tm *tm);
void _localtime_js(const time_t *restrict t, struct tm *restrict tm);
void _gmtime_js(const time_t *restrict t, struct tm *restrict tm);

weak time_t timegm(struct tm *tm) {
  tzset();
  return _timegm_js(tm);
}

weak time_t mktime(struct tm *tm) {
  tzset();
  return _mktime_js(tm);
}

weak struct tm *__localtime_r(const time_t *restrict t, struct tm *restrict tm) {
  tzset();
  _localtime_js(t, tm);
  // __localtime_js sets everything but the tmzone pointer
  tm->__tm_zone = tm->tm_isdst ? tzname[1] :tzname[0];
  return tm;
}

weak struct tm *__gmtime_r(const time_t *restrict t, struct tm *restrict tm) {
  tzset();
  _gmtime_js(t, tm);
  tm->tm_isdst = 0;
  tm->__tm_gmtoff = 0;
  tm->__tm_zone = "GMT";
  return tm;
}

weak_alias(__gmtime_r, gmtime_r);
weak_alias(__localtime_r, localtime_r);
