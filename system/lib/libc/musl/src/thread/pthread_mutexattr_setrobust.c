#include "pthread_impl.h"
#ifndef __EMSCRIPTEN__ // XXX Emscripten revert musl commit 5994de4e02a05c19a6bddadcfb687ab2e7511bd6
#include "syscall.h"

static volatile int check_robust_result = -1;
#endif

int pthread_mutexattr_setrobust(pthread_mutexattr_t *a, int robust)
{
	if (robust > 1U) return EINVAL;
#ifndef __EMSCRIPTEN__ // XXX Emscripten revert musl commit 5994de4e02a05c19a6bddadcfb687ab2e7511bd6
	if (robust) {
		int r = check_robust_result;
		if (r < 0) {
			void *p;
			size_t l;
			r = -__syscall(SYS_get_robust_list, 0, &p, &l);
			a_store(&check_robust_result, r);
		}
		if (r) return r;
		a->__attr |= 4;
		return 0;
	}
#endif
	a->__attr &= ~4;
	return 0;
}
