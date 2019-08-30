#include "stdio_impl.h"
#include "pthread_impl.h"

int __lockfile(FILE *f)
{
#ifndef __EMSCRIPTEN__ // XXX EMSCRIPTEN: for now no pthreads; ignore locking
	int owner, tid = __pthread_self()->tid;
	if (f->lock == tid)
		return 0;
	while ((owner = a_cas(&f->lock, 0, tid)))
		__wait(&f->lock, &f->waiters, owner, 1);
#endif
	return 1;
}

void __unlockfile(FILE *f)
{
#ifndef __EMSCRIPTEN__
	a_store(&f->lock, 0);

	/* The following read is technically invalid under situations
	 * of self-synchronized destruction. Another thread may have
	 * called fclose as soon as the above store has completed.
	 * Nonetheless, since FILE objects always live in memory
	 * obtained by malloc from the heap, it's safe to assume
	 * the dereferences below will not fault. In the worst case,
	 * a spurious syscall will be made. If the implementation of
	 * malloc changes, this assumption needs revisiting. */

	if (f->waiters) __wake(&f->lock, 1, 1);
#endif
}
