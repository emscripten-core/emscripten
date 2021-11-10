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
#include "pthread_impl.h"

#ifndef __EMSCRIPTEN__
#define IS32BIT(x) !((x)+0x80000000ULL>>32)
#define CLAMP(x) (int)(IS32BIT(x) ? (x) : 0x7fffffffU+((0ULL+(x))>>63))

static int __futex4_cp(volatile void *addr, int op, int val, const struct timespec *to)
{
	int r;
#ifdef SYS_futex_time64
	time_t s = to ? to->tv_sec : 0;
	long ns = to ? to->tv_nsec : 0;
	r = -ENOSYS;
	if (SYS_futex == SYS_futex_time64 || !IS32BIT(s))
		r = __syscall_cp(SYS_futex_time64, addr, op, val,
			to ? ((long long[]){s, ns}) : 0);
	if (SYS_futex == SYS_futex_time64 || r!=-ENOSYS) return r;
	to = to ? (void *)(long[]){CLAMP(s), ns} : 0;
#endif
	r = __syscall_cp(SYS_futex, addr, op, val, to);
	if (r != -ENOSYS) return r;
	return __syscall_cp(SYS_futex, addr, op & ~FUTEX_PRIVATE, val, to);
}
#endif

static volatile int dummy = 0;
weak_alias(dummy, __eintr_valid_flag);

#ifdef __EMSCRIPTEN__
int _pthread_isduecanceled(struct pthread *pthread_ptr);
#endif

int __timedwait_cp(volatile int *addr, int val,
	clockid_t clk, const struct timespec *at, int priv)
{
	int r;
	struct timespec to, *top=0;

	if (priv) priv = FUTEX_PRIVATE;

	if (at) {
		if (at->tv_nsec >= 1000000000UL) return EINVAL;
		if (__clock_gettime(clk, &to)) return EINVAL;
		to.tv_sec = at->tv_sec - to.tv_sec;
		if ((to.tv_nsec = at->tv_nsec - to.tv_nsec) < 0) {
			to.tv_sec--;
			to.tv_nsec += 1000000000;
		}
		if (to.tv_sec < 0) return ETIMEDOUT;
		top = &to;
	}

#ifdef __EMSCRIPTEN__
	double msecsToSleep = top ? (top->tv_sec * 1000 + top->tv_nsec / 1000000.0) : INFINITY;
	int is_main_thread = emscripten_is_main_browser_thread();
	// cp suffix in the function name means "cancellation point", so this wait can be cancelled
	// by the users unless current threads cancelability is set to PTHREAD_CANCEL_DISABLE
	// which may be either done by the user of __timedwait() function.
	if (is_main_thread ||
	    pthread_self()->canceldisable != PTHREAD_CANCEL_DISABLE ||
	    pthread_self()->cancelasync == PTHREAD_CANCEL_ASYNCHRONOUS) {
		double sleepUntilTime = emscripten_get_now() + msecsToSleep;
		do {
			if (_pthread_isduecanceled(pthread_self())) {
				// Emscripten-specific return value: The wait was canceled by user calling
				// pthread_cancel() for this thread, and the caller needs to cooperatively
				// cancel execution.
				return ECANCELED;
			}
			// Assist other threads by executing proxied operations that are effectively singlethreaded.
			if (is_main_thread) emscripten_main_thread_process_queued_calls();
			// Must wait in slices in case this thread is cancelled in between.
			double waitMsecs = sleepUntilTime - emscripten_get_now();
			if (waitMsecs <= 0) {
				r = ETIMEDOUT;
				break;
			}
			if (waitMsecs > 100) waitMsecs = 100; // non-main threads can sleep in longer slices.
			if (is_main_thread && waitMsecs > 1) waitMsecs = 1; // main thread may need to run proxied calls, so sleep in very small slices to be responsive.
			r = -emscripten_futex_wait((void*)addr, val, waitMsecs);
		} while(r == ETIMEDOUT);
	} else {
		// Can wait in one go.
		r = -emscripten_futex_wait((void*)addr, val, msecsToSleep);
	}
#else
	r = -__futex4_cp(addr, FUTEX_WAIT|priv, val, top);
#endif
	if (r != EINTR && r != ETIMEDOUT && r != ECANCELED) r = 0;
	/* Mitigate bug in old kernels wrongly reporting EINTR for non-
	 * interrupting (SA_RESTART) signal handlers. This is only practical
	 * when NO interrupting signal handlers have been installed, and
	 * works by sigaction tracking whether that's the case. */
	if (r == EINTR && !__eintr_valid_flag) r = 0;

	return r;
}

int __timedwait(volatile int *addr, int val,
	clockid_t clk, const struct timespec *at, int priv)
{
	int cs, r;
	__pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
#ifdef __EMSCRIPTEN__
	emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_RUNNING, EM_THREAD_STATUS_WAITMUTEX);
#endif
	r = __timedwait_cp(addr, val, clk, at, priv);
#ifdef __EMSCRIPTEN__
	emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_WAITMUTEX, EM_THREAD_STATUS_RUNNING);
#endif
	__pthread_setcancelstate(cs, 0);

	return r;
}
