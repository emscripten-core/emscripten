#include "pthread_impl.h"

int pthread_mutex_trylock(pthread_mutex_t *m)
{
	int tid, old, own;
	pthread_t self;

	if (m->_m_type == PTHREAD_MUTEX_NORMAL)
		return a_cas(&m->_m_lock, 0, EBUSY) & EBUSY;

	self = pthread_self();
	tid = self->tid;

	if (m->_m_type >= 4) {
		if (!self->robust_list.off)
			__syscall(SYS_set_robust_list,
				&self->robust_list, 3*sizeof(long));
		self->robust_list.off = (char*)&m->_m_lock-(char *)&m->_m_next;
		self->robust_list.pending = &m->_m_next;
	}

	old = m->_m_lock;
	own = old & 0x7fffffff;
	if (own == tid && (m->_m_type&3) == PTHREAD_MUTEX_RECURSIVE) {
		if ((unsigned)m->_m_count >= INT_MAX) return EAGAIN;
		m->_m_count++;
		return 0;
	}

	if ((own && !(own & 0x40000000)) || a_cas(&m->_m_lock, old, tid)!=old)
		return EBUSY;

	if (m->_m_type < 4) return 0;

	if (m->_m_type >= 8) {
		m->_m_lock = 0;
		return ENOTRECOVERABLE;
	}
	m->_m_next = self->robust_list.head;
	m->_m_prev = &self->robust_list.head;
	if (self->robust_list.head)
		self->robust_list.head[-1] = &m->_m_next;
	self->robust_list.head = &m->_m_next;
	self->robust_list.pending = 0;
	if (own) {
		m->_m_count = 0;
		m->_m_type += 8;
		return EOWNERDEAD;
	}

	return 0;
}
