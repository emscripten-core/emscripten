#include <time.h>

char *__asctime(const struct tm *restrict, char *restrict);

char *asctime_r(const struct tm *restrict tm, char *restrict buf)
{
	return __asctime(tm, buf);
}
