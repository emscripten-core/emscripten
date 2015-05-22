#include "pthread_impl.h"

int pthread_cond_broadcast(pthread_cond_t *c)
{
	pthread_mutex_t *m;

	if (!c->_c_waiters) return 0;

	a_inc(&c->_c_seq);

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

	/* Perform the futex requeue, waking one waiter unless we know
	 * that the calling thread holds the mutex. */
	__syscall(SYS_futex, &c->_c_seq, FUTEX_REQUEUE,
		!m->_m_type || (m->_m_lock&INT_MAX)!=pthread_self()->tid,
		INT_MAX, &m->_m_lock);

out:
	a_store(&c->_c_lock, 0);
	if (c->_c_lockwait) __wake(&c->_c_lock, 1, 0);

	return 0;
}
