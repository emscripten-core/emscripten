#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test(int result, const char* comment, const char* parsed = "") {
  printf("%s: %d\n", comment, result);
  if (!result) {
    printf("\nERROR: %s (\"%s\")\n", comment, parsed);
  }
}

int cmp(const char* s1, const char* s2) {
  for (; *s1 == *s2; s1++, s2++) {
    if (*s1 == '\0') break;
  }

  return (*s1 - *s2);
}

int main() {
  struct tm tm;
  char s[1000];
  size_t size;

  tm.tm_sec = 4;
  tm.tm_min = 23;
  tm.tm_hour = 20;
  tm.tm_mday = 21;
  tm.tm_mon = 1;
  tm.tm_year = 74;
  tm.tm_wday = 4;
  tm.tm_yday = 51;
  tm.tm_isdst = 0;

  size = strftime(s, 1000, "", &tm);
  test((size == 0) && (*s == '\0'), "strftime test #1", s);

  size = strftime(s, 1000, "%a", &tm);
  test((size == 3) && !cmp(s, "Thu"), "strftime test #2", s);

  size = strftime(s, 1000, "%A", &tm);
  test((size == 8) && !cmp(s, "Thursday"), "strftime test #3", s);

  size = strftime(s, 1000, "%b", &tm);
  test((size == 3) && !cmp(s, "Feb"), "strftime test #4", s);

  size = strftime(s, 1000, "%B", &tm);
  test((size == 8) && !cmp(s, "February"), "strftime test #5", s);

  size = strftime(s, 1000, "%d", &tm);
  test((size == 2) && !cmp(s, "21"), "strftime test #6", s);

  size = strftime(s, 1000, "%H", &tm);
  test((size == 2) && !cmp(s, "20"), "strftime test #7", s);

  size = strftime(s, 1000, "%I", &tm);
  test((size == 2) && !cmp(s, "08"), "strftime test #8", s);

  size = strftime(s, 1000, "%j", &tm);
  test((size == 3) && !cmp(s, "052"), "strftime test #9", s);

  size = strftime(s, 1000, "%m", &tm);
  test((size == 2) && !cmp(s, "02"), "strftime test #10", s);

  size = strftime(s, 1000, "%M", &tm);
  test((size == 2) && !cmp(s, "23"), "strftime test #11", s);

  size = strftime(s, 1000, "%p", &tm);
  test((size == 2) && !cmp(s, "PM"), "strftime test #12", s);

  size = strftime(s, 1000, "%S", &tm);
  test((size == 2) && !cmp(s, "04"), "strftime test #13", s);

  size = strftime(s, 1000, "%U", &tm);
  test((size == 2) && !cmp(s, "07"), "strftime test #14", s);

  size = strftime(s, 1000, "%w", &tm);
  test((size == 1) && !cmp(s, "4"), "strftime test #15", s);

  size = strftime(s, 1000, "%W", &tm);
  test((size == 2) && !cmp(s, "07"), "strftime test #16", s);

  size = strftime(s, 1000, "%y", &tm);
  test((size == 2) && !cmp(s, "74"), "strftime test #17", s);

  size = strftime(s, 1000, "%Y", &tm);
  test((size == 4) && !cmp(s, "1974"), "strftime test #18", s);

  size = strftime(s, 1000, "%%", &tm);
  test((size == 1) && !cmp(s, "%"), "strftime test #19", s);

  size = strftime(s, 5, "%Y", &tm);
  test((size == 4) && !cmp(s, "1974"), "strftime test #20", s);

  size = strftime(s, 4, "%Y", &tm);
  test((size == 0), "strftime test #21", s);

  tm.tm_mon = 0;
  tm.tm_mday = 1;
  size = strftime(s, 10, "%U", &tm);
  test((size == 2) && !cmp(s, "00"), "strftime test #22", s);

  size = strftime(s, 10, "%W", &tm);
  test((size == 2) && !cmp(s, "00"), "strftime test #23", s);

  // 1/1/1973 was a Sunday and is in CW 1
  tm.tm_year = 73;
  size = strftime(s, 10, "%W", &tm);
  test((size == 2) && !cmp(s, "01"), "strftime test #24", s);

  // 1/1/1978 was a Monday and is in CW 1
  tm.tm_year = 78;
  size = strftime(s, 10, "%U", &tm);
  test((size == 2) && !cmp(s, "01"), "strftime test #25", s);

  // 2/1/1999
  tm.tm_year = 99;
  tm.tm_yday = 1;
  size = strftime(s, 10, "%G (%V)", &tm);
  test((size == 9) && !cmp(s, "1998 (53)"), "strftime test #26", s);

  size = strftime(s, 10, "%g", &tm);
  test((size == 2) && !cmp(s, "98"), "strftime test #27", s);

  // 30/12/1997
  tm.tm_year = 97;
  tm.tm_yday = 363;
  size = strftime(s, 10, "%G (%V)", &tm);
  test((size == 9) && !cmp(s, "1998 (01)"), "strftime test #28", s);

  size = strftime(s, 10, "%g", &tm);
  test((size == 2) && !cmp(s, "98"), "strftime test #29", s);

  // timezones
  time_t xmas2002 = 1040786563ll;
  time_t summer2002 = 1025528525ll;
  localtime_r(&summer2002, &tm);
  int ahead = timegm(&tm) >= summer2002;
  size = strftime(s, 50, "%z", &tm);
  test((size == 5) && strchr(s, ahead ? '+' : '-'), "strftime zone test #1", s);
  size = strftime(s, 50, "%Z", &tm);
  test(strcmp(s, tzname[tm.tm_isdst]) == 0, "strftime zone test #2", s);

  localtime_r(&xmas2002, &tm);
  ahead = timegm(&tm) >= xmas2002;
  size = strftime(s, 50, "%z", &tm);
  test((size == 5) && strchr(s, ahead ? '+' : '-'), "strftime zone test #3", s);
  size = strftime(s, 50, "%Z", &tm);
  test(strcmp(s, tzname[tm.tm_isdst]) == 0, "strftime zone test #4", s);

  // AM/PM
  tm.tm_sec = 0;
  tm.tm_min = 0;

  tm.tm_hour = 0;
  size = strftime(s, 10, "%I %p", &tm);
  test(!cmp(s, "12 AM"), "strftime test #32", s);

  tm.tm_hour = 12;
  size = strftime(s, 10, "%I %p", &tm);
  test(!cmp(s, "12 PM"), "strftime test #33", s);

  tm.tm_min = 1;

  tm.tm_hour = 0;
  size = strftime(s, 10, "%I %M %p", &tm);
  test(!cmp(s, "12 01 AM"), "strftime test #34", s);

  tm.tm_hour = 12;
  size = strftime(s, 10, "%I %M %p", &tm);
  test(!cmp(s, "12 01 PM"), "strftime test #35", s);

}
