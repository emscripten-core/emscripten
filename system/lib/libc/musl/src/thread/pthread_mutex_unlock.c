#include "pthread_impl.h"

void __vm_lock_impl(int);
void __vm_unlock_impl(void);

int pthread_mutex_unlock(pthread_mutex_t *m)
{
	pthread_t self;
	int waiters = m->_m_waiters;
	int cont;
	int robust = 0;

	if (m->_m_type != PTHREAD_MUTEX_NORMAL) {
		if (!m->_m_lock)
			return EPERM;
		self = pthread_self();
		if ((m->_m_lock&0x1fffffff) != self->tid)
			return EPERM;
		if ((m->_m_type&3) == PTHREAD_MUTEX_RECURSIVE && m->_m_count)
			return m->_m_count--, 0;
		if (m->_m_type >= 4) {
			robust = 1;
			self->robust_list.pending = &m->_m_next;
			*(void **)m->_m_prev = m->_m_next;
			if (m->_m_next) ((void **)m->_m_next)[-1] = m->_m_prev;
			__vm_lock_impl(+1);
		}
	}
	cont = a_swap(&m->_m_lock, 0);
	if (robust) {
		self->robust_list.pending = 0;
		__vm_unlock_impl();
	}
	if (waiters || cont<0)
		__wake(&m->_m_lock, 1, 0);
	return 0;
}
