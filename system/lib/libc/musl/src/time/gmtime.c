#include "time_impl.h"
#include <errno.h>

struct tm *__gmtime_r(const time_t *restrict, struct tm *restrict);

struct tm *gmtime(const time_t *t)
{
	static struct tm tm;
	return __gmtime_r(t, &tm);
}
