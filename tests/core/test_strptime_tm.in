#include <time.h>
#include <stdio.h>
#include <string.h>

void ReadMonth(const char *month)
{
    tm value = {0};
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

  return 0;
}
