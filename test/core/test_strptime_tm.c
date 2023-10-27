/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <time.h>
#include <stdio.h>
#include <string.h>

void ReadMonth(const char *month)
{
    struct tm value = {0};
    if(strptime(month, "%b", &value))
    {
        printf("%s: %d\n", month, value.tm_mon);
    }
}

int main() {
  struct tm tm;
  char *ptr = strptime("17410105012000", "%H%M%S%d%m%Y", &tm);

  printf(
      "%s: %s, %d/%d/%d %d:%d:%d", (ptr != NULL && *ptr == '\0') ? "OK" : "ERR",
      tm.tm_wday == 0
          ? "Sun"
          : (tm.tm_wday == 1
                 ? "Mon"
                 : (tm.tm_wday == 2
                        ? "Tue"
                        : (tm.tm_wday == 3
                               ? "Wed"
                               : (tm.tm_wday == 4
                                      ? "Thu"
                                      : (tm.tm_wday == 5
                                             ? "Fri"
                                             : (tm.tm_wday == 6 ? "Sat"
                                                                : "ERR")))))),
      tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min,
      tm.tm_sec);

  printf("\n");

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
  strptime("2020-05-01T00:01%z","%Y-%m-%dT%H:%M%%z",&tm);
  printf("%d\n",tm.tm_min);
  
  // check that %% is handled correctly even if the letter after it is 
  // in EQUIVALENT_MATCHERS
  strptime("%D2020-05-01T00:01","%%D%Y-%m-%dT%H:%M",&tm);
  printf("%d,%d\n",tm.tm_year+1900,tm.tm_min);


  // check that EQUIVALENT_MATCHERS works
  // %c == %a %b %d %H:%M:%S %Y
  strptime("Sun March 31 12:34:56 2345","%c",&tm);
  printf("%d,%d,%d,%d,%d,%d\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);

  // check that EQUIVALENT_MATCHERS works twice
  //      'T':  '%H\\:%M\\:%S',
  //      'D':  '%m\\/%d\\/%y',
  strptime("12:34:56 01/02/03","%T %D",&tm);
  printf("%d,%d,%d,%d,%d,%d\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);

  // check regex special characters don't break things
  strptime(".?12:34:56 01/02/03",".?%T %D",&tm);
  printf("%d,%d,%d,%d,%d,%d\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);


  // check timezone offsets
  strptime("2020-05-01T00:00+0100","%Y-%m-%dT%H:%M%z",&tm);
  printf("%ld\n",tm.tm_gmtoff); // 3600

  strptime("2020-05-01T00:00Z","%Y-%m-%dT%H:%M%z",&tm);
  printf("%ld\n",tm.tm_gmtoff); // 0

  strptime("2020-05-01T00:00-02:30","%Y-%m-%dT%H:%M%z",&tm);
  printf("%ld\n",tm.tm_gmtoff); // -9000

  // check that the numbers of spaces in format string are ignored
  strptime("12     34     56","%H %M %S",&tm);
  printf("%d,%d,%d\n",tm.tm_hour,tm.tm_min,tm.tm_sec);

  strptime("123456","%H %M %S",&tm);
  printf("%d,%d,%d\n",tm.tm_hour,tm.tm_min,tm.tm_sec);

  strptime("12 34    56","%H   %M %S",&tm);
  printf("%d,%d,%d\n",tm.tm_hour,tm.tm_min,tm.tm_sec);

  return 0;
}
