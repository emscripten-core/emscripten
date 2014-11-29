#include "pthread_impl.h"

struct cm {
	pthread_cond_t *c;
	pthread_mutex_t *m;
};

static void unwait(pthread_cond_t *c, pthread_mutex_t *m)
{
	/* Removing a waiter is non-trivial if we could be using requeue
	 * based broadcast signals, due to mutex access issues, etc. */

	if (c->_c_mutex == (void *)-1) {
		a_dec(&c->_c_waiters);
		if (c->_c_destroy) __wake(&c->_c_waiters, 1, 0);
		return;
	}

	while (a_swap(&c->_c_lock, 1))
		__wait(&c->_c_lock, &c->_c_lockwait, 1, 1);

	if (c->_c_waiters2) c->_c_waiters2--;
	else a_dec(&m->_m_waiters);

	a_store(&c->_c_lock, 0);
	if (c->_c_lockwait) __wake(&c->_c_lock, 1, 1);

	a_dec(&c->_c_waiters);
	if (c->_c_destroy) __wake(&c->_c_waiters, 1, 1);
}

static void cleanup(void *p)
{
	struct cm *cm = p;
	unwait(cm->c, cm->m);
	pthread_mutex_lock(cm->m);
}

int pthread_cond_timedwait(pthread_cond_t *restrict c, pthread_mutex_t *restrict m, const struct timespec *restrict ts)
{
	struct cm cm = { .c=c, .m=m };
	int r, e=0, seq;

	if (m->_m_type && (m->_m_lock&INT_MAX) != pthread_self()->tid)
		return EPERM;

	if (ts && ts->tv_nsec >= 1000000000UL)
		return EINVAL;

	pthread_testcancel();

	a_inc(&c->_c_waiters);

	if (c->_c_mutex != (void *)-1) {
		c->_c_mutex = m;
		while (a_swap(&c->_c_lock, 1))
			__wait(&c->_c_lock, &c->_c_lockwait, 1, 1);
		c->_c_waiters2++;
		a_store(&c->_c_lock, 0);
		if (c->_c_lockwait) __wake(&c->_c_lock, 1, 1);
	}

	seq = c->_c_seq;

	pthread_mutex_unlock(m);

	do e = __timedwait(&c->_c_seq, seq, c->_c_clock, ts, cleanup, &cm, 0);
	while (c->_c_seq == seq && (!e || e==EINTR));
	if (e == EINTR) e = 0;

	unwait(c, m);

	if ((r=pthread_mutex_lock(m))) return r;

	return e;
}
