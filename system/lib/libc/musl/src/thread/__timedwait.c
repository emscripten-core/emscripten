#include <pthread.h>
#include <time.h>
#include <errno.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/threading.h>
#include <emscripten/emscripten.h>
#else
#include "futex.h"
#endif
#include "syscall.h"

static int do_wait(volatile int *addr, int val,
	clockid_t clk, const struct timespec *at, int priv)
{
	int r;
	struct timespec to, *top=0;

	if (at) {
		if (at->tv_nsec >= 1000000000UL) return EINVAL;
		if (clock_gettime(clk, &to)) return EINVAL;
		to.tv_sec = at->tv_sec - to.tv_sec;
		if ((to.tv_nsec = at->tv_nsec - to.tv_nsec) < 0) {
			to.tv_sec--;
			to.tv_nsec += 1000000000;
		}
		if (to.tv_sec < 0) return ETIMEDOUT;
		top = &to;
	}

#ifdef __EMSCRIPTEN__
	double timeout = top->tv_sec * 1000000000.0 + top->tv_nsec;
	if (timeout > 1 * 1000000000.0) timeout = 1 * 1000000000.0;
	EM_ASM_INT( { Module['printErr']('Wait ' + $0 + '.') }, timeout);
	r = emscripten_futex_wait((void*)addr, val, timeout);
#else
	r = -__syscall_cp(SYS_futex, addr, FUTEX_WAIT, val, top);
#endif
	if (r == EINTR || r == EINVAL || r == ETIMEDOUT) return r;
	return 0;
}

int __timedwait(volatile int *addr, int val,
	clockid_t clk, const struct timespec *at,
	void (*cleanup)(void *), void *arg, int priv)
{
	int r, cs;

	if (!cleanup) pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	pthread_cleanup_push(cleanup, arg);

	r = do_wait(addr, val, clk, at, priv);

	pthread_cleanup_pop(0);
	if (!cleanup) pthread_setcancelstate(cs, 0);

	return r;
}
