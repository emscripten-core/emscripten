#include "pthread_impl.h"
#ifndef __EMSCRIPTEN__ // XXX Emscripten PI mutexes are unsupported (see musl commit 54ca677983d47529bab8752315ac1a2b49888870)
#include "syscall.h"

static volatile int check_pi_result = -1;
#endif

int pthread_mutexattr_setprotocol(pthread_mutexattr_t *a, int protocol)
{
#ifdef __EMSCRIPTEN__ // XXX Emscripten PI mutexes are unsupported (see musl commit 54ca677983d47529bab8752315ac1a2b49888870)
	if (protocol) return ENOTSUP;
	return 0;
#else
	int r;
	switch (protocol) {
	case PTHREAD_PRIO_NONE:
		a->__attr &= ~8;
		return 0;
	case PTHREAD_PRIO_INHERIT:
		r = check_pi_result;
		if (r < 0) {
			volatile int lk = 0;
			r = -__syscall(SYS_futex, &lk, FUTEX_LOCK_PI, 0, 0);
			a_store(&check_pi_result, r);
		}
		if (r) return r;
		a->__attr |= 8;
		return 0;
	case PTHREAD_PRIO_PROTECT:
		return ENOTSUP;
	default:
		return EINVAL;
	}
#endif
}
