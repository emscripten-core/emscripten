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
double _pthread_msecs_until(const struct timespec *restrict at);
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
	if (1 || pthread_self()->cancelasync == PTHREAD_CANCEL_ASYNCHRONOUS) {
		do {
			if (_pthread_isduecanceled(pthread_self())) {
				// Emscripten-specific return value: The wait was canceled by user calling
				// pthread_cancel() for this thread, and the caller needs to cooperatively
				// cancel execution.
				return ECANCELED;
			}
			// Must wait in slices in case this thread is cancelled in between.
			double waitMsecs = at ? _pthread_msecs_until(at) : INFINITY;
			if (waitMsecs <= 0) {
				r = ETIMEDOUT;
				break;
			}
			if (waitMsecs > 100) waitMsecs = 100;
			r = -emscripten_futex_wait((void*)addr, val, waitMsecs);
			// Assist other threads by executing proxied operations that are effectively singlethreaded.
			if (emscripten_is_main_runtime_thread()) emscripten_main_thread_process_queued_calls();
		} while(r == ETIMEDOUT);
	} else {
		// Can wait in one go.
		double waitMsecs = at ? _pthread_msecs_until(at) : INFINITY;
		r = -emscripten_futex_wait((void*)addr, val, waitMsecs);
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

	return r;
}
