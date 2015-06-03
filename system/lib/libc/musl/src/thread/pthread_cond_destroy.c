#include "pthread_impl.h"

int pthread_cond_destroy(pthread_cond_t *c)
{
	int priv = c->_c_mutex != (void *)-1;
	int cnt;
	c->_c_destroy = 1;
	if (c->_c_waiters)
		__wake(&c->_c_seq, -1, priv);
	while ((cnt = c->_c_waiters))
		__wait(&c->_c_waiters, 0, cnt, priv);
	return 0;
}
