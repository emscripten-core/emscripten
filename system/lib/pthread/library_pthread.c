#include <pthread.h>
#include <emscripten/threading.h>
#include <emscripten.h>
#include <sys/time.h>
#include <math.h>
#include "../internal/pthread_impl.h"
#include <assert.h>

int _pthread_getcanceltype()
{
	return pthread_self()->cancelasync;
}

static void inline __pthread_mutex_locked(pthread_mutex_t *mutex)
{
	// The lock is now ours, mark this thread as the owner of this lock.
	assert(mutex);
	assert(mutex->_m_lock == 0);
	mutex->_m_lock = pthread_self()->tid;
	if (_pthread_getcanceltype() == PTHREAD_CANCEL_ASYNCHRONOUS) pthread_testcancel();
}

double _pthread_nsecs_until(const struct timespec *restrict at)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	double cur_t = t.tv_sec * 1e9 + t.tv_usec * 1e3;
	double at_t = at->tv_sec * 1e9 + at->tv_nsec;
	double nsecs = at_t - cur_t;
	return nsecs;
}

#if 0
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	if (!mutex) return EINVAL;
	assert(pthread_self() != 0);
	assert(pthread_self()->tid != 0);

	if (mutex->_m_lock == pthread_self()->tid) {
		if ((mutex->_m_type&3) == PTHREAD_MUTEX_RECURSIVE) {
			if ((unsigned)mutex->_m_count >= INT_MAX) return EAGAIN;
			++mutex->_m_count;
			return 0;
		} else if ((mutex->_m_type&3) == PTHREAD_MUTEX_ERRORCHECK) {
			return EDEADLK;
		}
	}

	int threadCancelType = _pthread_getcanceltype();

	int c = emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 1);
	if (c != 0) {
		do {
			if (c == 2 || emscripten_atomic_cas_u32(&mutex->_m_addr, 1, 2) != 0) {
				double nsecs = INFINITY;
				if (threadCancelType == PTHREAD_CANCEL_ASYNCHRONOUS) {
					// Sleep in small slices so that we can test cancellation to honor PTHREAD_CANCEL_ASYNCHRONOUS.
					pthread_testcancel();
					nsecs = 100 * 1000 * 1000;
				}
				emscripten_futex_wait(&mutex->_m_addr, 2, nsecs);
			}
		} while((c = emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 2)));
	}

	__pthread_mutex_locked(mutex);
	return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	if (!mutex) return EINVAL;
	assert(pthread_self() != 0);

	if (mutex->_m_type != PTHREAD_MUTEX_NORMAL) {
		if (mutex->_m_lock != pthread_self()->tid) return EPERM;
		if ((mutex->_m_type&3) == PTHREAD_MUTEX_RECURSIVE && mutex->_m_count) {
			--mutex->_m_count;
			return 0;
		}
	}

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
	if (!mutex) return EINVAL;
	if (mutex->_m_lock == pthread_self()->tid) {
		if ((mutex->_m_type&3) == PTHREAD_MUTEX_RECURSIVE) {
			if ((unsigned)mutex->_m_count >= INT_MAX) return EAGAIN;
			++mutex->_m_count;
			return 0;
		} else if ((mutex->_m_type&3) == PTHREAD_MUTEX_ERRORCHECK) {
			return EDEADLK;
		}
	}

	if (emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 1) == 0) {
		__pthread_mutex_locked(mutex);
		return 0;
	}
	else
		return EBUSY;
}

int pthread_mutex_timedlock(pthread_mutex_t *restrict mutex, const struct timespec *restrict at)
{
	if (!mutex || !at) return EINVAL;
	if (mutex->_m_lock == pthread_self()->tid) {
		if ((mutex->_m_type&3) == PTHREAD_MUTEX_RECURSIVE) {
			if ((unsigned)mutex->_m_count >= INT_MAX) return EAGAIN;
			++mutex->_m_count;
			return 0;
		} else if ((mutex->_m_type&3) == PTHREAD_MUTEX_ERRORCHECK) {
			return EDEADLK;
		}
	}

	int threadCancelType = _pthread_getcanceltype();
	int c = emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 1);
	if (c != 0) {
		do {
			if (c == 2 || emscripten_atomic_cas_u32(&mutex->_m_addr, 1, 2) != 0)
			{
				if (at->tv_nsec < 0 || at->tv_nsec >= 1000000000) return EINVAL;
				double nsecs = _pthread_nsecs_until(at);
				if (nsecs <= 0) return ETIMEDOUT;

				// Sleep in small slices if thread type is PTHREAD_CANCEL_ASYNCHRONOUS
				// so that we can honor PTHREAD_CANCEL_ASYNCHRONOUS requests.
				if (threadCancelType == PTHREAD_CANCEL_ASYNCHRONOUS) {
					pthread_testcancel();
					if (nsecs > 100 * 1000 * 1000) nsecs = 100 * 1000 * 1000;
				}
				int ret = emscripten_futex_wait(&mutex->_m_addr, 2, nsecs);
				if (ret == 0) break;
				else if (threadCancelType != PTHREAD_CANCEL_ASYNCHRONOUS || _pthread_nsecs_until(at) <= 0) {
					return ETIMEDOUT;
				}
			}
		} while((c = emscripten_atomic_cas_u32(&mutex->_m_addr, 0, 2)));
	}

	__pthread_mutex_locked(mutex);
	return 0;
}
#endif

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

int pthread_setcancelstate(int new, int *old)
{
	if (new > 1U) return EINVAL;
	struct pthread *self = pthread_self();
	if (old) *old = self->canceldisable;
	self->canceldisable = new;
	return 0;
}

void pthread_testcancel()
{
	struct pthread *self = pthread_self();
	if (self->canceldisable) return;
	if (self->threadStatus == 2/*canceled*/) {
		EM_ASM( throw 'Canceled!'; );
	}
}

static uint32_t dummyZeroAddress = 0;

int usleep(unsigned usec)
{
	double now = emscripten_get_now();
	double target = now + usec / 1000.0;
	while(now < target) {
		double nsecsToSleep = (target - now) * 1e6;
		if (nsecsToSleep > 1e6) {
			if (nsecsToSleep > 100 * 1000 * 1000) nsecsToSleep = 100 * 1000 * 1000;
			pthread_testcancel();
			emscripten_futex_wait(&dummyZeroAddress, 0, nsecsToSleep);
		}
		now = emscripten_get_now();
	}
	return 0;
}
