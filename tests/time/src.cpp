#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <math.h>

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

  return 0;
}
