#define _GNU_SOURCE
#include <pthread.h>
#include <emscripten/threading.h>
#include <emscripten.h>
#include <sys/time.h>
#include <dirent.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include "../internal/pthread_impl.h"
#include <assert.h>

// With LLVM 3.6, C11 is the default compilation mode.
// gets() is deprecated under that standard, but emcc
// still provides it, so always include it in the build.
#if __STDC_VERSION__ >= 201112L
char *gets(char *);
#endif

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

double _pthread_msecs_until(const struct timespec *restrict at)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	double cur_t = t.tv_sec * 1e3 + t.tv_usec * 1e-3;
	double at_t = at->tv_sec * 1e3 + at->tv_nsec * 1e-6;
	double msecs = at_t - cur_t;
	return msecs;
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
				double msecs = INFINITY;
				if (threadCancelType == PTHREAD_CANCEL_ASYNCHRONOUS) {
					// Sleep in small slices so that we can test cancellation to honor PTHREAD_CANCEL_ASYNCHRONOUS.
					pthread_testcancel();
					msecs = 100;
				}
				emscripten_futex_wait(&mutex->_m_addr, 2, msecs);
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
				double msecs = _pthread_msecs_until(at);
				if (msecs <= 0) return ETIMEDOUT;

				// Sleep in small slices if thread type is PTHREAD_CANCEL_ASYNCHRONOUS
				// so that we can honor PTHREAD_CANCEL_ASYNCHRONOUS requests.
				if (threadCancelType == PTHREAD_CANCEL_ASYNCHRONOUS) {
					pthread_testcancel();
					if (msecs > 100) msecs = 100;
				}
				int ret = emscripten_futex_wait(&mutex->_m_addr, 2, msecs);
				if (ret == 0) break;
				else if (threadCancelType != PTHREAD_CANCEL_ASYNCHRONOUS || _pthread_msecs_until(at) <= 0) {
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

int _pthread_isduecanceled(struct pthread *pthread_ptr)
{
	return pthread_ptr->threadStatus == 2/*canceled*/;
}

void pthread_testcancel()
{
	struct pthread *self = pthread_self();
	if (self->canceldisable) return;
	if (_pthread_isduecanceled(self)) {
		EM_ASM( throw 'Canceled!'; );
	}
}

int pthread_getattr_np(pthread_t t, pthread_attr_t *a)
{
	*a = (pthread_attr_t){0};
	a->_a_detach = !!t->detached;
	a->_a_stackaddr = (uintptr_t)t->stack;
	a->_a_stacksize = t->stack_size - DEFAULT_STACK_SIZE;
	return 0;
}

static uint32_t dummyZeroAddress = 0;

int usleep(unsigned usec)
{
	int is_main_thread = emscripten_is_main_runtime_thread();
	double now = emscripten_get_now();
	double target = now + usec * 1e-3;
#ifdef __EMSCRIPTEN__
	emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_RUNNING, EM_THREAD_STATUS_SLEEPING);
#endif
	while(now < target) {
		if (is_main_thread) emscripten_main_thread_process_queued_calls(); // Assist other threads by executing proxied operations that are effectively singlethreaded.
		pthread_testcancel(); // pthreads spec: usleep is a cancellation point, so it must test if this thread is cancelled during the sleep.
		now = emscripten_get_now();
		double msecsToSleep = target - now;
		if (msecsToSleep > 1.0) {
			if (msecsToSleep > 100.0) msecsToSleep = 100.0;
			if (is_main_thread && msecsToSleep > 1) msecsToSleep = 1; // main thread may need to run proxied calls, so sleep in very small slices to be responsive.
			emscripten_futex_wait(&dummyZeroAddress, 0, msecsToSleep);
		}
	}
#ifdef __EMSCRIPTEN__
	emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_SLEEPING, EM_THREAD_STATUS_RUNNING);
#endif
	return 0;
}

static void _do_call(em_queued_call *q)
{
	switch(q->function)
	{
		case EM_PROXIED_UTIME: q->returnValue.i = utime(q->args[0].cp, (struct utimbuf*)q->args[1].vp); break;
		case EM_PROXIED_UTIMES: q->returnValue.i = utimes(q->args[0].cp, (struct timeval*)q->args[1].vp); break;
		case EM_PROXIED_CHROOT: q->returnValue.i = chroot(q->args[0].cp); break;
		case EM_PROXIED_FPATHCONF: q->returnValue.i = fpathconf(q->args[0].i, q->args[1].i); break;
		case EM_PROXIED_CONFSTR: q->returnValue.i = confstr(q->args[0].i, q->args[1].cp, q->args[2].i); break;
		case EM_PROXIED_SYSCONF: q->returnValue.i = sysconf(q->args[0].i); break;
		case EM_PROXIED_SBRK: q->returnValue.vp = sbrk(q->args[0].i); break;
		case EM_PROXIED_ATEXIT: q->returnValue.i = atexit(q->args[0].vp); break;
		case EM_PROXIED_GETENV: q->returnValue.cp = getenv(q->args[0].cp); break;
		case EM_PROXIED_CLEARENV: q->returnValue.i = clearenv(); break;
		case EM_PROXIED_SETENV: q->returnValue.i = setenv(q->args[0].cp, q->args[1].cp, q->args[2].i); break;
		case EM_PROXIED_UNSETENV: q->returnValue.i = unsetenv(q->args[0].cp); break;
		case EM_PROXIED_PUTENV: q->returnValue.i = putenv(q->args[0].cp); break;
		case EM_PROXIED_TZSET: tzset(); break;
		case EM_PROXIED_PTHREAD_CREATE: q->returnValue.i = pthread_create(q->args[0].vp, q->args[1].vp, q->args[2].vp, q->args[3].vp); break;
		case EM_PROXIED_SYSCALL: q->returnValue.i = emscripten_syscall(q->args[0].i, q->args[1].vp); break;
		default: assert(0 && "Invalid Emscripten pthread _do_call opcode!");
	}
	q->operationDone = 1;
	emscripten_futex_wake(&q->operationDone, INT_MAX);
}

#define CALL_QUEUE_SIZE 128
static em_queued_call **call_queue = 0;
static int call_queue_length = 0; // Shared data synchronized by call_queue_lock.
static pthread_mutex_t call_queue_lock = PTHREAD_MUTEX_INITIALIZER;

void EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread(em_queued_call *call)
{
	assert(call);
	if (emscripten_is_main_runtime_thread()) {
		_do_call(call);
		return;
	}
	pthread_mutex_lock(&call_queue_lock);
	if (!call_queue) call_queue = malloc(sizeof(em_queued_call*) * CALL_QUEUE_SIZE); // Shared data synchronized by call_queue_lock.
	// Note: currently call_queue_length can be at most the number of pthreads that are currently running, so the queue can never get
	// full. However if/when the queue is extended to be asynchronous for void-returning functions later, this will need to be revised.
	assert(call_queue_length < CALL_QUEUE_SIZE);
	call_queue[call_queue_length] = call;
	++call_queue_length;
	if (call_queue_length == 1) {
		EM_ASM(postMessage({ cmd: 'processQueuedMainThreadWork' }));
	}
	pthread_mutex_unlock(&call_queue_lock);
	int r;
	emscripten_set_current_thread_status(EM_THREAD_STATUS_WAITPROXY);
	do {
		r = emscripten_futex_wait(&call->operationDone, 0, INFINITY);
	} while(r != 0 && call->operationDone == 0);
	emscripten_set_current_thread_status(EM_THREAD_STATUS_RUNNING);
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_1(int function, void *arg1)
{
	em_queued_call q = { function, 0 };
	q.args[0].vp = arg1;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_2(int function, void *arg1, void *arg2)
{
	em_queued_call q = { function, 0 };
	q.args[0].vp = arg1;
	q.args[1].vp = arg2;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_xprintf_varargs(int function, int param0, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	const int CAP = 128;
	char str[CAP];
	char *s = str;
	int len = vsnprintf(s, CAP, format, args);
	if (len >= CAP)
	{
		s = (char*)malloc(len+1);
		va_start(args, format);
		len = vsnprintf(s, len+1, format, args);
	}
	em_queued_call q = { function, 0 };
	q.args[0].vp = param0;
	q.args[1].vp = s;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	if (s != str) free(s);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_3(int function, void *arg1, void *arg2, void *arg3)
{
	em_queued_call q = { function, 0 };
	q.args[0].vp = arg1;
	q.args[1].vp = arg2;
	q.args[2].vp = arg3;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_4(int function, void *arg1, void *arg2, void *arg3, void *arg4)
{
	em_queued_call q = { function, 0 };
	q.args[0].vp = arg1;
	q.args[1].vp = arg2;
	q.args[2].vp = arg3;
	q.args[3].vp = arg4;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_5(int function, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5)
{
	em_queued_call q = { function, 0 };
	q.args[0].vp = arg1;
	q.args[1].vp = arg2;
	q.args[2].vp = arg3;
	q.args[3].vp = arg4;
	q.args[4].vp = arg5;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_6(int function, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6)
{
	em_queued_call q = { function, 0 };
	q.args[0].vp = arg1;
	q.args[1].vp = arg2;
	q.args[2].vp = arg3;
	q.args[3].vp = arg4;
	q.args[4].vp = arg5;
	q.args[5].vp = arg6;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_7(int function, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7)
{
	em_queued_call q = { function, 0 };
	q.args[0].vp = arg1;
	q.args[1].vp = arg2;
	q.args[2].vp = arg3;
	q.args[3].vp = arg4;
	q.args[4].vp = arg5;
	q.args[5].vp = arg6;
	q.args[6].vp = arg7;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

static int bool_inside_nested_process_queued_calls = 0;

void EMSCRIPTEN_KEEPALIVE emscripten_main_thread_process_queued_calls()
{
	assert(emscripten_is_main_runtime_thread() && "emscripten_main_thread_process_queued_calls must be called from the main thread!");
	if (!emscripten_is_main_runtime_thread()) return;

	// It is possible that when processing a queued call, the call flow leads back to calling this function in a nested fashion!
	// Therefore this scenario must explicitly be detected, and processing the queue must be avoided if we are nesting, or otherwise
	// the same queued calls would be processed again and again.
	if (bool_inside_nested_process_queued_calls) return;
	// This must be before pthread_mutex_lock(), since pthread_mutex_lock() can call back to this function.
	bool_inside_nested_process_queued_calls = 1;
	pthread_mutex_lock(&call_queue_lock);
	for (int i = 0; i < call_queue_length; ++i)
		_do_call(call_queue[i]);
	call_queue_length = 0;
	bool_inside_nested_process_queued_calls = 0;
	pthread_mutex_unlock(&call_queue_lock);
}

float EMSCRIPTEN_KEEPALIVE emscripten_atomic_load_f32(const void *addr)
{
	union {
		float f;
		uint32_t u;
	} u;
	u.u = emscripten_atomic_load_u32(addr);
	return u.f;
}

// Use an array of multiple interleaved spinlock mutexes to separate memory addresses to ease pressure when locking.
// This is outright horrible, but enables easily porting code that does require 64-bit atomics.
// Eventually in the long run we'd hope to have real support for 64-bit atomics in the browser, after
// which this emulation can be removed.
#define NUM_64BIT_LOCKS 256
static int emulated64BitAtomicsLocks[NUM_64BIT_LOCKS] = {};

uint32_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_exchange_u32(void/*uint32_t*/ *addr, uint32_t newVal)
{
	uint32_t oldVal, oldVal2;
	do {
		oldVal = emscripten_atomic_load_u32(addr);
		oldVal2 = emscripten_atomic_cas_u32(addr, oldVal, newVal);
	} while (oldVal != oldVal2);
	return oldVal;
}

#define SPINLOCK_ACQUIRE(addr) do { while(emscripten_atomic_exchange_u32((void*)(addr), 1)) /*nop*/; } while(0)
#define SPINLOCK_RELEASE(addr) emscripten_atomic_store_u32((void*)(addr), 0)

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_exchange_u64(void/*uint64_t*/ *addr, uint64_t newVal)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldValInMemory = *(uint64_t*)addr;
	*(uint64_t*)addr = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldValInMemory;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_cas_u64(void/*uint64_t*/ *addr, uint64_t oldVal, uint64_t newVal)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldValInMemory = *(uint64_t*)addr;
	if (oldValInMemory == oldVal)
		*(uint64_t*)addr = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldValInMemory;
}

double EMSCRIPTEN_KEEPALIVE emscripten_atomic_load_f64(const void *addr)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	double val = *(double*)addr;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return val;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_load_u64(const void *addr)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t val = *(uint64_t*)addr;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return val;
}

float EMSCRIPTEN_KEEPALIVE emscripten_atomic_store_f32(void *addr, float val)
{
	union {
		float f;
		uint32_t u;
	} u;
	u.f = val;
	return emscripten_atomic_store_u32(addr, u.u);
}

double EMSCRIPTEN_KEEPALIVE emscripten_atomic_store_f64(void *addr, double val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	double *a = (double*)addr;
	*a = val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return val;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_store_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t *a = (uint64_t*)addr;
	*a = val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return val;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_add_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t *a = (uint64_t *)addr;
	uint64_t newVal = *a + val;
	*a = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return newVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_add. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_add_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal + val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_sub_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t *a = (uint64_t *)addr;
	uint64_t newVal = *a - val;
	*a = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return newVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_sub. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_sub_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal - val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_and_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t *a = (uint64_t *)addr;
	uint64_t newVal = *a & val;
	*a = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return newVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_and. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_and_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal & val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_or_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t *a = (uint64_t *)addr;
	uint64_t newVal = *a | val;
	*a = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return newVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_or. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_or_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal | val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

uint64_t EMSCRIPTEN_KEEPALIVE emscripten_atomic_xor_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t *a = (uint64_t *)addr;
	uint64_t newVal = *a ^ val;
	*a = newVal;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return newVal;
}

// This variant is implemented for emulating GCC 64-bit __sync_fetch_and_xor. Not to be called directly.
uint64_t EMSCRIPTEN_KEEPALIVE _emscripten_atomic_fetch_and_xor_u64(void *addr, uint64_t val)
{
	uintptr_t m = (uintptr_t)addr >> 3;
	SPINLOCK_ACQUIRE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	uint64_t oldVal = *(uint64_t *)addr;
	*(uint64_t *)addr = oldVal ^ val;
	SPINLOCK_RELEASE(&emulated64BitAtomicsLocks[m&(NUM_64BIT_LOCKS-1)]);
	return oldVal;
}

int llvm_memory_barrier()
{
	emscripten_atomic_fence();
}

int llvm_atomic_load_add_i32_p0i32(int *ptr, int delta)
{
	return emscripten_atomic_add_u32(ptr, delta) - delta;
}

uint64_t __atomic_load_8(void *ptr, int memmodel)
{
	return emscripten_atomic_load_u64(ptr);
}

uint64_t __atomic_store_8(void *ptr, uint64_t value, int memmodel)
{
	return emscripten_atomic_store_u64(ptr, value);
}

uint64_t __atomic_exchange_8(void *ptr, uint64_t value, int memmodel)
{
	return emscripten_atomic_exchange_u64(ptr, value);
}

uint64_t __atomic_compare_exchange_8(void *ptr, uint64_t *expected, uint64_t desired, int weak, int success_memmodel, int failure_memmodel)
{
	return emscripten_atomic_cas_u64(ptr, *expected, desired);
}

uint64_t __atomic_fetch_add_8(void *ptr, uint64_t value, int memmodel)
{
	return _emscripten_atomic_fetch_and_add_u64(ptr, value);
}

uint64_t __atomic_fetch_sub_8(void *ptr, uint64_t value, int memmodel)
{
	return _emscripten_atomic_fetch_and_sub_u64(ptr, value);
}

uint64_t __atomic_fetch_and_8(void *ptr, uint64_t value, int memmodel)
{
	return _emscripten_atomic_fetch_and_and_u64(ptr, value);
}

uint64_t __atomic_fetch_or_8(void *ptr, uint64_t value, int memmodel)
{
	return _emscripten_atomic_fetch_and_or_u64(ptr, value);
}

uint64_t __atomic_fetch_xor_8(void *ptr, uint64_t value, int memmodel)
{
	return _emscripten_atomic_fetch_and_xor_u64(ptr, value);
}
