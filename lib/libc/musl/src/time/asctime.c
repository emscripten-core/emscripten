#include <time.h>

char *__asctime(const struct tm *, char *);

char *asctime(const struct tm *tm)
{
	static char buf[26];
	return __asctime(tm, buf);
}
