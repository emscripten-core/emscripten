#include "pthread_impl.h"

int __pthread_mutex_timedlock(pthread_mutex_t *restrict m, const struct timespec *restrict at)
{
	if ((m->_m_type&15) == PTHREAD_MUTEX_NORMAL
	    && !a_cas(&m->_m_lock, 0, EBUSY))
		return 0;

	int r, t, priv = (m->_m_type & 128) ^ 128;

	r = pthread_mutex_trylock(m);
	if (r != EBUSY) return r;
	
	int spins = 100;
	while (spins-- && m->_m_lock && !m->_m_waiters) a_spin();

	while ((r=pthread_mutex_trylock(m)) == EBUSY) {
		if (!(r=m->_m_lock) || ((r&0x40000000) && (m->_m_type&4)))
			continue;
		if ((m->_m_type&3) == PTHREAD_MUTEX_ERRORCHECK
		 && (r&0x7fffffff) == __pthread_self()->tid)
			return EDEADLK;
#ifdef __EMSCRIPTEN__
		// If no time is given, do not sleep forever - there is a possible race
		// condition here. We need to keep iterating in this loop, since _m_lock
		// may be set to the value we want *just* before the __timedwait. __timedwait
		// does not check if the value is what we want, it just checks for a wake on
		// that address, so it would wait forever.
		// Instead of picking some arbitrary time to wait, just keep busy-waiting.
		if (!at) {
			r = pthread_mutex_trylock(m);
			if (r != EBUSY) return r;
		}
		continue;
#endif
		a_inc(&m->_m_waiters);
		t = r | 0x80000000;
		a_cas(&m->_m_lock, r, t);
		r = __timedwait(&m->_m_lock, t, CLOCK_REALTIME, at, priv);
		a_dec(&m->_m_waiters);
		if (r && r != EINTR) break;
	}
	return r;
}

weak_alias(__pthread_mutex_timedlock, pthread_mutex_timedlock);
