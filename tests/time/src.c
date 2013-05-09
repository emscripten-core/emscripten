#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main() {
  time_t xmas2002 = 1040786563ll;
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
  printf("off: %d\n", tm_ptr->tm_gmtoff);
  printf("zone: %s\n", tm_ptr->tm_zone);
  
  // Verify timegm() reverses gmtime.
  printf("timegm <-> gmtime: %d\n", timegm(tm_ptr) == xmas2002);
  
  // Verify gmtime_r() doesn't clobber static data.
  time_t t1 = 0;
  struct tm tm1;
  gmtime_r(&t1, &tm1);
  printf("old year: %d\n", tm_ptr->tm_year);
  printf("new year: %d\n", tm1.tm_year);
  gmtime(&xmas2002);
  printf("old year again: %d\n", tm_ptr->tm_year);
  
  // Verify localtime() picks up timezone data.
  time_t t2 = xmas2002 - 60 * 60 * 24 * 30 * 6;
  tm_ptr = localtime(&t2);
  time_t dst_diff = (tm_ptr->tm_isdst == 1) ? tm_ptr->tm_isdst * 60 * 60 : 0;
  printf("localtime timezone: %d\n", (_timezone + tm_ptr->tm_gmtoff == dst_diff)); // glibc needs
  printf("localtime daylight: %d\n", _daylight == tm_ptr->tm_isdst);               // no prefix "_"s
  printf("localtime tzname: %d\n", (!strcmp(tzname[0], tm_ptr->tm_zone) ||
                                    !strcmp(tzname[1], tm_ptr->tm_zone)));

  // Verify localtime() and mktime() reverse each other.
  printf("localtime <-> mktime: %d\n", mktime(localtime(&xmas2002)) == xmas2002);
  
  // Verify localtime_r() doesn't clobber static data.
  time_t t3 = 0;
  struct tm tm2;
  localtime_r(&t3, &tm2);
  printf("localtime_r(1): %d\n", tm2.tm_year != tm_ptr->tm_year);
  localtime(&xmas2002);
  printf("localtime_r(2): %d\n", tm2.tm_year != tm_ptr->tm_year);

  // Verify time() returns reasonable value (between 2011 and 2030).
  time_t t4 = 0;
  time(&t4);
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

  // Verify asctime_r() doesn't clobber static data.
  time_t t6 = 1309635200ll;
  tm_ptr = gmtime(&xmas2002);
  char* formatted = asctime(tm_ptr);
  char buffer[32];
  asctime_r(gmtime(&t6), buffer);
  printf("old asctime: %s", formatted);
  printf("new asctime_r: %s", buffer);
  asctime_r(tm_ptr, buffer);
  printf("old asctime again: %s", formatted);

  // Verify that clock() advances.
  time_t start_t = time(NULL);
  clock_t start = clock();
  printf("clock(start): %d\n", start >= 0);
  while (clock() - start < 2 * CLOCKS_PER_SEC); // Poor man's sleep().
  clock_t diff = time(NULL) - start_t;
  printf("clock(end): %d\n", diff >= 2 && diff < 30);

  // Verify that ctime_r(x, buf) is equivalent to asctime_r(localtime(x), buf).
  time_t t7 = time(0);
  char buffer2[30];
  char buffer3[30];
  printf("ctime: %d\n", strcmp(ctime_r(&t7, buffer2),
                               asctime_r(localtime(&t7), buffer3)));

  return 0;
}
