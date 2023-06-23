// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <math.h>

void check_gmtime_localtime(time_t time)
{
  char gmbuf[32], locbuf[32];
  const char fmt[] = "%Y-%m-%d %H:%M:%S";
  struct tm *gm;
  struct tm *loc;

  gm = gmtime(&time);
  assert(gm);
  assert(strftime(gmbuf, sizeof(gmbuf) - 1, fmt, gm) > 0);
  loc = localtime(&time);
  assert(loc);
  assert(strftime(locbuf, sizeof(locbuf) - 1, fmt, loc) > 0);

  // gmtime and localtime are only equal when timezone is UTC
  if ((timezone == 0) && (strcmp(gmbuf, locbuf) != 0)) {
    printf("time: %lld, gmtime: %s != localtime: %s\n", time, gmbuf, locbuf);
    puts("failed");
  } else {
    printf("time: %lld, gmtime: %s\n", time, gmbuf);
  }
}

int main() {
  time_t xmas2002 = 1040786563ll;
  time_t summer2002 = 1025528525ll;
  struct tm* tm_ptr;

  // Make sure stime() always fails.
  printf("stime: %d\n", stime(&xmas2002));

  // Verify that tzname sets *something*.
  tzset();
  printf("tzname[0] set: %d\n", strlen(tzname[0]) >= 3);
  printf("tzname[1] set: %d\n", strlen(tzname[1]) >= 3);

  // Verify gmtime() creates correct struct.
  tm_ptr = gmtime(&xmas2002);
  printf("sec: %d\n", tm_ptr->tm_sec);
  printf("min: %d\n", tm_ptr->tm_min);
  printf("hour: %d\n", tm_ptr->tm_hour);
  printf("day: %d\n", tm_ptr->tm_mday);
  printf("mon: %d\n", tm_ptr->tm_mon);
  printf("year: %d\n", tm_ptr->tm_year);
  printf("wday: %d\n", tm_ptr->tm_wday);
  printf("yday: %d\n", tm_ptr->tm_yday);
  printf("dst: %d\n", tm_ptr->tm_isdst);
  printf("off: %ld\n", (long)tm_ptr->tm_gmtoff);
  printf("zone: %s\n", tm_ptr->tm_zone);

  // Verify timegm() reverses gmtime; run through an entire year in half hours.
  int timegmOk = 1;
  for (int i = 0; i < 2*24*266; ++i) {
    struct tm tmp;
    time_t test = xmas2002 + 30*60*i;
    if (gmtime_r(&test, &tmp) != &tmp) printf("gmtime_r failed\n");
    struct tm copy = tmp;
    copy.tm_wday = -1;
    copy.tm_yday = -1;
    if (timegm(&copy) != test || copy.tm_wday != tmp.tm_wday ||
        copy.tm_yday != tmp.tm_yday)
      timegmOk = 0;
  }
  printf("timegm <-> gmtime: %d\n", timegmOk);

  // Verify gmtime_r() doesn't clobber static data.
  time_t t1 = 0;
  struct tm tm1;
  gmtime_r(&t1, &tm1);
  printf("old year still: %d\n", tm_ptr->tm_year);
  printf("new year: %d\n", tm1.tm_year);

  // Verify localtime() picks up timezone data.
  struct tm tm_winter, tm_summer;
  if (localtime_r(&xmas2002, &tm_winter) != &tm_winter) printf("localtime_r failed\n");
  if (localtime_r(&summer2002, &tm_summer) != &tm_summer) printf("localtime_r failed\n");
  printf("localtime found DST data (summer): %s\n", tm_summer.tm_isdst < 0 ? "no" : "yes");
  printf("localtime found DST data (winter): %s\n", tm_winter.tm_isdst < 0 ? "no" : "yes");
  int localeHasDst = tm_winter.tm_isdst == 1 || tm_summer.tm_isdst == 1; // DST is in December in south
  printf("localtime matches daylight: %s\n", localeHasDst == _daylight ? "yes" : "no");
  int goodGmtOff = (tm_winter.tm_gmtoff != tm_summer.tm_gmtoff) == localeHasDst;
  printf("localtime gmtoff matches DST: %s\n", goodGmtOff ? "yes" : "no");
  printf("localtime tm_zone matches tzname (winter): %s\n",
         strcmp(tzname[tm_winter.tm_isdst], tm_winter.tm_zone) ? "no" : "yes");
  printf("localtime tm_zone matches tzname (summer): %s\n",
         strcmp(tzname[tm_summer.tm_isdst], tm_summer.tm_zone) ? "no" : "yes");

  // Verify that timezone is always equal to std time
  // Need to invert these since timezone is positive in the east and negative in the west
  int inv_summer = tm_summer.tm_gmtoff * -1;
  int inv_winter = tm_winter.tm_gmtoff * -1;

  if (tm_winter.tm_isdst) {
    printf("localtime equals std: %s\n", inv_summer == timezone ? "true" : "false");
    assert(inv_winter != timezone);
    assert(inv_summer == timezone);
  } else if (tm_summer.tm_isdst) {
    printf("localtime equals std: %s\n", inv_winter == timezone ? "true" : "false");
    assert(inv_summer != timezone);
    assert(inv_winter == timezone);
  } else {
    printf("localtime equals std: %s\n", (inv_summer == timezone && inv_winter == timezone) ? "true" : "false");
    assert(inv_summer == timezone);
    assert(inv_winter == timezone);
  }

  // Verify localtime() and mktime() reverse each other; run through an entire year
  // in half hours (the two hours where the time jumps forward and back are the
  // ones to watch, but we don't where they are since the zoneinfo could be US or
  // European)
  int mktimeOk = 1;
  for (int i = 0; i < 2*24*366; ++i) {
    struct tm tmp;
    time_t test = xmas2002 + 30*60*i;
    if (localtime_r(&test, &tmp) != &tmp) printf("localtime_r failed\n");
    struct tm copy = tmp;
    copy.tm_wday = -1;
    copy.tm_yday = -1;
    if (mktime(&copy) != test || copy.tm_wday != tmp.tm_wday ||
        copy.tm_yday != tmp.tm_yday || copy.tm_isdst != tmp.tm_isdst)
      mktimeOk = 0;
  }
  printf("localtime <-> mktime: %d\n", mktimeOk);

  // Verify that mktime updates the tm struct to the correct date if its values are
  // out of range by matching against the return value of localtime.
  struct tm tm2 { 0 }, tm_local;
  tm2.tm_sec = tm2.tm_min = tm2.tm_hour = tm2.tm_mday = tm2.tm_mon = tm2.tm_wday =
    tm2.tm_yday = 1000;
  time_t t2 = mktime(&tm2); localtime_r(&t2, &tm_local);
  mktimeOk = !(
    tm2.tm_sec < 0 || tm2.tm_sec > 60 || tm2.tm_min < 0 || tm2.tm_min > 59 ||
    tm2.tm_hour < 0 || tm2.tm_hour > 23 || tm2.tm_mday < 1 || tm2.tm_mday > 31 ||
    tm2.tm_mon < 0 || tm2.tm_mon > 11 || tm2.tm_wday < 0 || tm2.tm_wday > 6 ||
    tm2.tm_yday < 0 || tm2.tm_yday > 365);
  printf("mktime updates parameter to be in range: %d\n", mktimeOk);
  mktimeOk = !(
    tm2.tm_sec != tm_local.tm_sec || tm2.tm_min != tm_local.tm_min ||
    tm2.tm_hour != tm_local.tm_hour || tm2.tm_mday != tm_local.tm_mday ||
    tm2.tm_mon != tm_local.tm_mon ||  tm2.tm_year != tm_local.tm_year ||
    tm2.tm_wday != tm_local.tm_wday || tm2.tm_yday != tm_local.tm_yday);

  printf("mktime parameter is equivalent to localtime return: %d\n", mktimeOk);

  // Verify that mktime is able to guess what the dst is. It might get it wrong
  // during the one ambiguous hour when the clock goes back -- we assume that in
  // no locale that happens on Jul 1 (summer2002) or Dec 25 (xmas2002).
  int oldDstWinter = tm_winter.tm_isdst, oldDstSummer = tm_summer.tm_isdst;
  tm_winter.tm_isdst = tm_summer.tm_isdst = -1;
  mktime(&tm_winter); mktime(&tm_summer);
  printf("mktime guesses DST (winter): %d\n", tm_winter.tm_isdst == oldDstWinter);
  printf("mktime guesses DST (summer): %d\n", tm_summer.tm_isdst == oldDstSummer);

  // Verify localtime_r() doesn't clobber static data.
  time_t t3 = 60*60*24*5; // Jan 5 1970
  struct tm tm3;
  localtime_r(&t3, &tm3);
  printf("old year still: %d\n", tm_ptr->tm_year);
  printf("new year: %d\n", tm3.tm_year);

  // Verify time() returns reasonable value (between 2011 and 2030).
  time_t t4 = 0;
  time(&t4);
  timespec ts;
  assert(clock_gettime(CLOCK_REALTIME, &ts) == 0);
  assert(abs(ts.tv_sec - t4) <= 2);
  printf("time: %d\n", t4 > 1309635200ll && t4 < 1893362400ll);

  // Verify difftime() calculates accurate time difference.
  time_t t5 = 1309635200ll;
  printf("difftime+: %lf\n", difftime(t5, xmas2002));
  printf("difftime-: %lf\n", difftime(xmas2002, t5));

  // Verify dysize() knows its leap years.
  printf("1854 days: %d\n", dysize(1854));
  printf("2000 days: %d\n", dysize(2000));
  printf("2001 days: %d\n", dysize(2001));
  printf("2004 days: %d\n", dysize(2004));

  // Verify asctime() formatting().
  printf("asctime: %s", asctime(gmtime(&xmas2002)));

  tm_ptr = gmtime(&xmas2002);
  char* formatted = asctime(tm_ptr);
  char buffer[32];
  struct tm t6;
  asctime_r(gmtime_r(&summer2002, &t6), buffer);
  printf("winter asctime: %s", formatted);
  printf("summer asctime_r: %s", buffer);
  // Verify asctime_r() and ctime_r() don't clobber static data.
  asctime_r(&t6, buffer);
  ctime_r(&summer2002, buffer);
  printf("winter asctime again: %s", formatted);
  printf("winter month again: %d\n", tm_ptr->tm_mon);

  // Verify that ctime_r(x, buf) is equivalent to asctime_r(localtime(x), buf).
  time_t t7 = time(0);
  char buffer2[30];
  char buffer3[30];
  printf("ctime matched: %d\n", !strcmp(ctime_r(&t7, buffer2),
                                        asctime_r(localtime(&t7), buffer3)));

  // Verify that clock() advances.
  time_t start_t = time(NULL);
  clock_t start = clock();
  printf("clock(start): %d\n", start >= 0);
  while (clock() - start < 2 * CLOCKS_PER_SEC); // Poor man's sleep().
  clock_t diff = time(NULL) - start_t;
  printf("clock(end): %d\n", diff >= 2 && diff < 30);

  // Verify timespec_get() will only accept a base of TIME_UTC
  //timespec ts; *already defined*
  printf("timespec_get test 0: %d\n", timespec_get(&ts, TIME_UTC) == TIME_UTC);
  printf("timespec_get test 1: %d\n", timespec_get(&ts, (TIME_UTC + 1)) == 0);

  // Verify the resultant timespec values set by timespec_get() are valid
  //timespec ts; *already defined*
  timespec_get(&ts, TIME_UTC);
  printf("timespec_get test 2: %d\n", ts.tv_sec >= 0);
  printf("timespec_get test 3: %d\n", ts.tv_sec != 0); // 0 is valid but not practical as the current time
  printf("timespec_get test 4: %d\n", ts.tv_nsec >= 0);
  printf("timespec_get test 5: %d\n", ts.tv_nsec <= 999999999);

  // Verify timespec_get() gets similar time value as clock_gettime
  timespec ts_timespec_get;
  timespec_get(&ts_timespec_get, TIME_UTC);
  timespec ts_clock_gettime;
  clock_gettime(CLOCK_REALTIME, &ts_clock_gettime);
  printf("timespec_get test 6: %d\n", abs(ts_timespec_get.tv_sec - ts_clock_gettime.tv_sec) <= 2);

  // verify gmtime() and localtime()
  check_gmtime_localtime(0);
  check_gmtime_localtime(2147483647); // int8_t max, Y2K38
  check_gmtime_localtime(2147483648);
  check_gmtime_localtime(-2147483648); // int8_t min
  check_gmtime_localtime(-2147483649);
  check_gmtime_localtime(253402300799); // end of year 9999
  check_gmtime_localtime(-62135596800); // beginning of year 1
  check_gmtime_localtime(0x83d4d9a5); // some time in 2040 (time_t > MAX_INT32)

  // check that localtime sets tm_yday correctly whenever the day rolls over (issue #17635)
  // prior to being fixed, tm_yday did not increment correctly at epoch time 1049061599 (2003-03-31 00:00:00) in CET time
  // assumes other tests already verified other aspects of localtime
  {
    struct tm prev_tm;
    time_t test = xmas2002;
    localtime_r(&test, &prev_tm);

    for (int i = 1; i < 2*24*366; ++i) {
      struct tm this_tm;
      test = xmas2002 + 30*60*i;
      localtime_r(&test, &this_tm);

      if (this_tm.tm_year != prev_tm.tm_year) {
        assert(this_tm.tm_yday == 0 && prev_tm.tm_yday == 364); //flipped over to 2003, 2002 was non-leap
      } else if(this_tm.tm_mday != prev_tm.tm_mday) {
        assert(this_tm.tm_yday == prev_tm.tm_yday + 1);
      }

      prev_tm = this_tm;
    }
  }

  puts("success");
  return 0;
}
