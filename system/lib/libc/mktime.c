/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <errno.h>
#include <time.h>

#include "emscripten_internal.h"

weak time_t timegm(struct tm *tm) {
  tzset();
  return _timegm_js(tm);
}

weak time_t mktime(struct tm *tm) {
  tzset();
  time_t t = _mktime_js(tm);
  if (t == -1) {
    errno = EOVERFLOW;
  }
  return t;
}

weak struct tm *__localtime_r(const time_t *restrict t, struct tm *restrict tm) {
  tzset();
  _localtime_js(*t, tm);
  // __localtime_js sets everything but the tmzone pointer
  tm->__tm_zone = tm->tm_isdst ? tzname[1] :tzname[0];
  return tm;
}

weak struct tm *__gmtime_r(const time_t *restrict t, struct tm *restrict tm) {
  tzset();
  _gmtime_js(*t, tm);
  tm->tm_isdst = 0;
  tm->__tm_gmtoff = 0;
  tm->__tm_zone = "GMT";
  return tm;
}

weak_alias(__gmtime_r, gmtime_r);
weak_alias(__localtime_r, localtime_r);
