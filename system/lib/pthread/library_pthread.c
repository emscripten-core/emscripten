#include <pthread.h>
#include <emscripten/threading.h>
#include <emscripten.h>
#include "../internal/pthread_impl.h"
#include <assert.h>

int pthread_mutex_lock(pthread_mutex_t *m)
{
	int c = emscripten_atomic_cas_u32(&m->_m_lock, 0, 1);
	if (c == 0)
		return 0;

	do {
		if (c == 2 || emscripten_atomic_cas_u32(&m->_m_lock, 1, 2) != 0)
			emscripten_futex_wait(&m->_m_lock, 2, 0);
	} while((c = emscripten_atomic_cas_u32(&m->_m_lock, 0, 2)));

	return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	if (emscripten_atomic_sub_u32((uint32_t*)&mutex->_m_lock, 1) != 1)
	{
		emscripten_atomic_store_u32((uint32_t*)&mutex->_m_lock, 0);
		emscripten_futex_wake((uint32_t*)&mutex->_m_lock, 1);
	}
	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *m)
{
	if (emscripten_atomic_cas_u32(&m->_m_lock, 0, 1) == 0)
		return 0;
	else
		return EBUSY;
}

int pthread_mutex_timedlock(pthread_mutex_t *restrict m, const struct timespec *restrict at)
{
	double nsecs;
	int c = emscripten_atomic_cas_u32(&m->_m_lock, 0, 1);
	if (c == 0)
		return 0;
	nsecs = at->tv_sec * 1e9 + (double)at->tv_nsec;

	do {
		if (c == 2 || emscripten_atomic_cas_u32(&m->_m_lock, 1, 2) != 0)
		{
			int ret = emscripten_futex_wait(&m->_m_lock, 2, nsecs);
			if (ret == 0) return 0;
			else return ETIMEDOUT;

		}
	} while((c = emscripten_atomic_cas_u32(&m->_m_lock, 0, 2)));

	return 0;
}

int sched_get_priority_max(int policy)
{
	// Web workers do not support prioritizing threads.
	return 0;
}

int sched_get_priority_min(int policy)
{
	// Web workers do not support prioritizing threads.
	return 0;
}
