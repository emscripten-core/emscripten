#include <time.h>
#include <limits.h>

#ifdef __EMSCRIPTEN__
#define TIME_T_MAX INT_MAX
#else
#define TIME_T_MAX LONG_MAX
#endif

clock_t clock()
{
	struct timespec ts;

	if (__clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts))
		return -1;

	if (ts.tv_sec > TIME_T_MAX/1000000
	 || ts.tv_nsec/1000 > LONG_MAX-1000000*ts.tv_sec)
		return -1;

	return ts.tv_sec*1000000 + ts.tv_nsec/1000;
}
