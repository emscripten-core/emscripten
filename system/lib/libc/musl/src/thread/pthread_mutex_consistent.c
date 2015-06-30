#include "pthread_impl.h"

int pthread_mutex_consistent(pthread_mutex_t *m)
{
	if (m->_m_type < 8) return EINVAL;
	if ((m->_m_lock & 0x3fffffff) != pthread_self()->tid)
		return EPERM;
	m->_m_type -= 8;
	return 0;
}
