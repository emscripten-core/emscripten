#include "pthread_impl.h"

int pthread_rwlock_unlock(pthread_rwlock_t *rw)
{
	int val, cnt, waiters, new;

	do {
		val = rw->_rw_lock;
		cnt = val & 0x7fffffff;
		waiters = rw->_rw_waiters;
		new = (cnt == 0x7fffffff || cnt == 1) ? 0 : val-1;
	} while (a_cas(&rw->_rw_lock, val, new) != val);

	if (!new && (waiters || val<0))
		__wake(&rw->_rw_lock, cnt, 0);

	return 0;
}
