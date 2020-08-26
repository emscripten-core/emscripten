#include <time.h>

char *__asctime_r(const struct tm *, char *);

char *asctime(const struct tm *tm)
{
	static char buf[26];
	return __asctime_r(tm, buf);
}
