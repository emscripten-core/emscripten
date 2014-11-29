#include "pthread_impl.h"

int pthread_cond_signal(pthread_cond_t *c)
{
	if (!c->_c_waiters) return 0;
	a_inc(&c->_c_seq);
	if (c->_c_waiters) __wake(&c->_c_seq, 1, 0);
	return 0;
}
