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

static const struct {
  const char *input;
  const char *format;
} day_tests[] = {{"2000-01-01", "%Y-%m-%d"},
                 {"03/03/00", "%D"},
                 {"9/9/99", "%x"},
                 {"19990502123412", "%Y%m%d%H%M%S"},
                 {"2001 20 Mon", "%Y %U %a"},
                 {"2006 4 Fri", "%Y %U %a"},
                 {"2001 21 Mon", "%Y %W %a"},
                 {"2013 29 Wed", "%Y %W %a"},
                 {"2000-01-01 08:12:21 AM", "%Y-%m-%d %I:%M:%S %p"},
                 {"2000-01-01 08:12:21 PM", "%Y-%m-%d %I:%M:%S %p"},
                 {"2001 17 Tue", "%Y %U %a"},
                 {"2001 8 Thursday", "%Y %W %a"}, };

int main() {
  struct tm tm;

  for (int i = 0; i < sizeof(day_tests) / sizeof(day_tests[0]); ++i) {
    memset(&tm, '\0', sizeof(tm));
    char *ptr = strptime(day_tests[i].input, day_tests[i].format, &tm);
    assert(ptr);

    printf("%-23s -> %02d/%02d/%4d (%dth DoW, %3dth DoY)\n",
           day_tests[i].input,
           tm.tm_mon + 1,
           tm.tm_mday,
           1900 + tm.tm_year,
           tm.tm_wday,
           tm.tm_yday);
  }
}
