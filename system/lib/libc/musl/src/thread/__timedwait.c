#include <pthread.h>
#include <time.h>
#include <errno.h>
#ifdef __EMSCRIPTEN__
#include <math.h>
#include <emscripten/threading.h>
#include <emscripten/emscripten.h>
#include "pthread_impl.h"
#else
#include "futex.h"
#endif
#include "syscall.h"

#ifdef __EMSCRIPTEN__
double _pthread_nsecs_until(const struct timespec *restrict at);
int _pthread_isduecanceled(struct pthread *pthread_ptr);
#endif

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
	if (pthread_self()->cancelasync == PTHREAD_CANCEL_ASYNCHRONOUS) {
		do {
			if (_pthread_isduecanceled(pthread_self())) return EINTR;
			// Must wait in slices in case this thread is cancelled in between.
			double waitNsecs = at ? _pthread_nsecs_until(at) : INFINITY;
			if (waitNsecs <= 0) {
				r = ETIMEDOUT;
				break;
			}
			if (waitNsecs > 100 * 1000 * 1000) waitNsecs = 100 * 1000 * 1000;
			r = -emscripten_futex_wait((void*)addr, val, waitNsecs);
		} while(r == ETIMEDOUT);
	} else {
		// Can wait in one go.
		double waitNsecs = at ? _pthread_nsecs_until(at) : INFINITY;
		r = -emscripten_futex_wait((void*)addr, val, waitNsecs);
	}
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

#ifdef __EMSCRIPTEN__
	// XXX Emscripten: since we don't have signals, cooperatively test cancellation.
	pthread_testcancel();
#endif
	return r;
}
