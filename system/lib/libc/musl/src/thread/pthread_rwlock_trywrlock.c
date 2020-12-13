#include "pthread_impl.h"

int __pthread_rwlock_trywrlock(pthread_rwlock_t *rw)
{
	if (a_cas(&rw->_rw_lock, 0, 0x7fffffff)) return EBUSY;
#ifdef __EMSCRIPTEN__
	/// XXX Emscripten: The spec allows detecting when multiple write locks would deadlock, which we do here to avoid hangs.
	/// Mark this thread to own the write lock, to ignore multiple attempts to lock.
	rw->_rw_wr_owner = __pthread_self()->tid;
#endif
	return 0;
}

weak_alias(__pthread_rwlock_trywrlock, pthread_rwlock_trywrlock);
