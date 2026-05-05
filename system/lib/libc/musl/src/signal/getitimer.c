#include <sys/time.h>
#include "syscall.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
void __getitimer(int which, struct itimerval *old, double now);
#endif

int getitimer(int which, struct itimerval *old)
{
#ifdef __EMSCRIPTEN__
	if (which > ITIMER_PROF) return EINVAL;
	__getitimer(which, old, emscripten_get_now());
	return 0;
#else
	if (sizeof(time_t) > sizeof(long)) {
		long old32[4];
		int r = __syscall(SYS_getitimer, which, old32);
		if (!r) {
			old->it_interval.tv_sec = old32[0];
			old->it_interval.tv_usec = old32[1];
			old->it_value.tv_sec = old32[2];
			old->it_value.tv_usec = old32[3];
		}
		return __syscall_ret(r);
	}
	return syscall(SYS_getitimer, which, old);
#endif
}
