#include "time_impl.h"

struct tm *__localtime_r(const time_t *restrict, struct tm *restrict);

struct tm *localtime(const time_t *t)
{
	static struct tm tm;
	return __localtime_r(t, &tm);
}
