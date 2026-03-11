#include "pthread_impl.h"

int __pthread_mutex_lock(pthread_mutex_t *m)
{
#if !defined(__EMSCRIPTEN__) || defined(NDEBUG)
	/* XXX EMSCRIPTEN always take the slow path in debug builds so we can trap rather than deadlock */
	if ((m->_m_type&15) == PTHREAD_MUTEX_NORMAL
	    && !a_cas(&m->_m_lock, 0, EBUSY))
		return 0;
#endif

	return __pthread_mutex_timedlock(m, 0);
}

weak_alias(__pthread_mutex_lock, pthread_mutex_lock);
