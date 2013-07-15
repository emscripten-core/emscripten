#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

int main() {
	setlocale(LC_TIME, "C");

	char buf[256];
	struct tm tm;
	// tm.tm_sec = 4;
      // tm.tm_min = 23;
      // tm.tm_hour = 20;
      tm.tm_mday = 2;
      tm.tm_mon = 0;
      tm.tm_year = 99;
      // tm.tm_wday = 4;
      tm.tm_yday = 2;
      tm.tm_isdst = 0;


	strftime(buf, 5, "%G", &tm);
	printf("%s\n", buf);
}
