#include <sys/time.h>
#include <errno.h>
#include "syscall.h"

#define IS32BIT(x) !((x)+0x80000000ULL>>32)

#ifdef __EMSCRIPTEN__
#include <signal.h>
#include <stdio.h>
#include <emscripten/emscripten.h>

// Timeouts can either fire directly from the JS event loop (which calls
// `_emscripten_timeout`), or from `_emscripten_check_timers` (which is called
// from `_emscripten_yield`).  In order to be able to check the timers here we
// cache the current timeout and interval for each the 3 types of timer
// (ITIMER_PROF/ITIMER_VIRTUAL/ITIMER_REAL).
static double current_timeout_ms[3];
static double current_intervals_ms[3];

#define MAX(a,b) ((a)>(b)?(a):(b))

int _setitimer_js(int which, double timeout);

void __getitimer(int which, struct itimerval *old, double now)
{
	double remaining_ms = MAX(current_timeout_ms[which] - now, 0);
	old->it_value.tv_sec = remaining_ms / 1000;
	old->it_value.tv_usec = remaining_ms * 1000;
	old->it_interval.tv_sec = current_intervals_ms[which] / 1000;
	old->it_interval.tv_usec = current_intervals_ms[which] * 1000;
}

void _emscripten_timeout(int which, double now)
{
	int signum = SIGALRM;
	if (which == ITIMER_PROF)
		signum = SIGPROF;
	else if (which == ITIMER_VIRTUAL)
		signum = SIGVTALRM;
	int next_timeout = current_intervals_ms[which];
	if (next_timeout)
		current_timeout_ms[which] = now + next_timeout;
	else
		current_timeout_ms[which] = 0;
	_setitimer_js(which, next_timeout);
	raise(signum);
}

void _emscripten_check_timers(double now)
{
	for (int which = 0; which < 3; which++) {
		if (current_timeout_ms[which]) {
			// Only call out to JS to get the current time if it was not passed in
			// *and* we have one or more timers set.
			if (!now)
			 	now = emscripten_get_now();
			if (now >= current_timeout_ms[which])
				_emscripten_timeout(which, now);
		}
	}
}
#endif

int setitimer(int which, const struct itimerval *restrict new, struct itimerval *restrict old)
{
#ifdef __EMSCRIPTEN__
	if (which > ITIMER_PROF) return EINVAL;
	double now = emscripten_get_now();
	if (old) {
		__getitimer(which, old, now);
	}
	if (new->it_value.tv_sec || new->it_value.tv_usec) {
		current_timeout_ms[which] = now + new->it_value.tv_sec * 1000 + new->it_value.tv_usec / 1000;
		current_intervals_ms[which] = new->it_interval.tv_sec * 1000 + new->it_interval.tv_usec / 1000;
	} else {
		current_timeout_ms[which] = 0;
		current_intervals_ms[which] = 0;
	}
	return _setitimer_js(which, new->it_value.tv_sec * 1000 + new->it_value.tv_usec / 1000);
#else
	if (sizeof(time_t) > sizeof(long)) {
		time_t is = new->it_interval.tv_sec, vs = new->it_value.tv_sec;
		long ius = new->it_interval.tv_usec, vus = new->it_value.tv_usec;
		if (!IS32BIT(is) || !IS32BIT(vs))
			return __syscall_ret(-ENOTSUP);
		long old32[4];
		int r = __syscall(SYS_setitimer, which,
			((long[]){is, ius, vs, vus}), old32);
		if (!r && old) {
			old->it_interval.tv_sec = old32[0];
			old->it_interval.tv_usec = old32[1];
			old->it_value.tv_sec = old32[2];
			old->it_value.tv_usec = old32[3];
		}
		return __syscall_ret(r);
	}
	return syscall(SYS_setitimer, which, new, old);
#endif
}
