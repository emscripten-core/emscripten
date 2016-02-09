#include "pthread_impl.h"

int pthread_cond_broadcast(pthread_cond_t *c)
{
	pthread_mutex_t *m;

	if (!c->_c_waiters) return 0;

	a_inc(&c->_c_seq);

#ifdef __EMSCRIPTEN__
	// XXX Emscripten: TODO: This is suboptimal but works naively correctly for now. The Emscripten-specific code path below
	// has a bug and does not work for some reason. Figure it out and remove this code block.
	__wake(&c->_c_seq, -1, 0);
	return 0;
#endif

	/* If cond var is process-shared, simply wake all waiters. */
	if (c->_c_mutex == (void *)-1) {
		__wake(&c->_c_seq, -1, 0);
		return 0;
	}

	/* Block waiters from returning so we can use the mutex. */
	while (a_swap(&c->_c_lock, 1))
		__wait(&c->_c_lock, &c->_c_lockwait, 1, 1);
	if (!c->_c_waiters)
		goto out;
	m = c->_c_mutex;

	/* Move waiter count to the mutex */
	a_fetch_add(&m->_m_waiters, c->_c_waiters2);
	c->_c_waiters2 = 0;

#ifdef __EMSCRIPTEN__
	int futexResult;
	do {
		// We want to wake one and requeue all others, without comparing the value, but SAB spec doesn't
		// have requeue without comparing, so implement it by spinning instead.
		futexResult = emscripten_futex_wake_or_requeue(&c->_c_seq, !m->_m_type || (m->_m_lock&INT_MAX)!=pthread_self()->tid,
			&m->_m_lock, c->_c_seq);
	} while(futexResult == -EAGAIN);
#else
	/* Perform the futex requeue, waking one waiter unless we know
	 * that the calling thread holds the mutex. */
	__syscall(SYS_futex, &c->_c_seq, FUTEX_REQUEUE,
		!m->_m_type || (m->_m_lock&INT_MAX)!=pthread_self()->tid,
		INT_MAX, &m->_m_lock);
#endif

out:
	a_store(&c->_c_lock, 0);
	if (c->_c_lockwait) __wake(&c->_c_lock, 1, 0);

	return 0;
}
