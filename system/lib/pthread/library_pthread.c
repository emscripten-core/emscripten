#include <pthread.h>
#include <emscripten/threading.h>
#include <emscripten.h>
#include "../internal/pthread_impl.h"
#include <assert.h>

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	int c = emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 1);
	if (c != 0) {
		do {
			if (c == 2 || emscripten_atomic_cas_u32(&mutex->_m_addr, 1, 2) != 0)
				emscripten_futex_wait(&mutex->_m_addr, 2, 0);
		} while((c = emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 2)));
	}

	// The lock is now ours, mark this thread as the owner of this lock.
	assert(__pthread_self() != 0);
	assert(__pthread_self()->tid != 0);
	assert(mutex->_m_lock == 0);
	mutex->_m_lock = __pthread_self()->tid;

	return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	assert(__pthread_self() != 0);
	assert(__pthread_self()->tid == mutex->_m_lock);
	mutex->_m_lock = 0;

	if (emscripten_atomic_sub_u32((uint32_t*)&mutex->_m_addr, 1) != 1)
	{
		emscripten_atomic_store_u32((uint32_t*)&mutex->_m_addr, 0);
		emscripten_futex_wake((uint32_t*)&mutex->_m_addr, 1);
	}
	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	if (emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 1) == 0)
		return 0;
	else
		return EBUSY;
}

int pthread_mutex_timedlock(pthread_mutex_t *restrict mutex, const struct timespec *restrict at)
{
	double nsecs;
	int c = emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 1);
	if (c == 0)
		return 0;
	nsecs = at->tv_sec * 1e9 + (double)at->tv_nsec;

	do {
		if (c == 2 || emscripten_atomic_cas_u32(&mutex->_m_addr, 1, 2) != 0)
		{
			int ret = emscripten_futex_wait(&mutex->_m_addr, 2, nsecs);
			if (ret == 0) return 0;
			else return ETIMEDOUT;

		}
	} while((c = emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 2)));

	return 0;
}

int sched_get_priority_max(int policy)
{
	// Web workers do not actually support prioritizing threads,
	// but mimic values that Linux apparently reports, see
	// http://man7.org/linux/man-pages/man2/sched_get_priority_min.2.html
	if (policy == SCHED_FIFO || policy == SCHED_RR)
		return 99;
	else
		return 0;
}

int sched_get_priority_min(int policy)
{
	// Web workers do not actually support prioritizing threads,
	// but mimic values that Linux apparently reports, see
	// http://man7.org/linux/man-pages/man2/sched_get_priority_min.2.html
	if (policy == SCHED_FIFO || policy == SCHED_RR)
		return 1;
	else
		return 0;
}
