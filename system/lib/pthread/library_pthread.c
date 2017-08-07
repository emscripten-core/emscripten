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
#include "../internal/libc.h"
#include "../internal/pthread_impl.h"
#include <assert.h>

// With LLVM 3.6, C11 is the default compilation mode.
// gets() is deprecated under that standard, but emcc
// still provides it, so always include it in the build.
#if __STDC_VERSION__ >= 201112L
char *gets(char *);
#endif

// Extra pthread_attr_t field:
#define _a_transferredcanvases __u.__s[9]

void __pthread_testcancel();

int emscripten_pthread_attr_gettransferredcanvases(const pthread_attr_t *a, const char **str)
{
	*str = (const char *)a->_a_transferredcanvases;
	return 0;
}

int emscripten_pthread_attr_settransferredcanvases(pthread_attr_t *a, const char *str)
{
	a->_a_transferredcanvases = (int)str;
	return 0;
}

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
	if (_pthread_getcanceltype() == PTHREAD_CANCEL_ASYNCHRONOUS) __pthread_testcancel();
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

void __pthread_testcancel()
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

static void do_sleep(double msecs)
{
	int is_main_thread = emscripten_is_main_runtime_thread();
	double now = emscripten_get_now();
	double target = now + msecs;
#ifdef __EMSCRIPTEN__
	emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_RUNNING, EM_THREAD_STATUS_SLEEPING);
#endif
	while(now < target) {
		if (is_main_thread) emscripten_main_thread_process_queued_calls(); // Assist other threads by executing proxied operations that are effectively singlethreaded.
		__pthread_testcancel(); // pthreads spec: usleep is a cancellation point, so it must test if this thread is cancelled during the sleep.
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
}

int nanosleep(const struct timespec *req, struct timespec *rem)
{
	if (!req || req->tv_nsec < 0 || req->tv_nsec > 999999999L || req->tv_sec < 0) return EINVAL;
	do_sleep(req->tv_sec * 1000.0 + req->tv_nsec / 1e6);
	return 0;
}

int usleep(unsigned usec)
{
	do_sleep(usec / 1e3);
	return 0;
}

// Allocator and deallocator for em_queued_call objects.
static em_queued_call *em_queued_call_malloc()
{
	em_queued_call *call = (em_queued_call*)malloc(sizeof(em_queued_call));
	call->operationDone = 0;
	call->functionPtr = 0;
	return call;
}
static void em_queued_call_free(em_queued_call *call)
{
	free(call);
}

void emscripten_async_waitable_close(em_queued_call *call)
{
	em_queued_call_free(call);
}

static void _do_call(em_queued_call *q)
{
	switch(q->functionEnum)
	{
		case EM_PROXIED_UTIME: q->returnValue.i = utime(q->args[0].cp, (struct utimbuf*)q->args[1].vp); break;
		case EM_PROXIED_UTIMES: q->returnValue.i = utimes(q->args[0].cp, (struct timeval*)q->args[1].vp); break;
		case EM_PROXIED_CHROOT: q->returnValue.i = chroot(q->args[0].cp); break;
		case EM_PROXIED_FPATHCONF: q->returnValue.i = fpathconf(q->args[0].i, q->args[1].i); break;
		case EM_PROXIED_CONFSTR: q->returnValue.i = confstr(q->args[0].i, q->args[1].cp, q->args[2].i); break;
		case EM_PROXIED_SYSCONF: q->returnValue.i = sysconf(q->args[0].i); break;
		case EM_PROXIED_ATEXIT: q->returnValue.i = atexit(q->args[0].vp); break;
		case EM_PROXIED_GETENV: q->returnValue.cp = getenv(q->args[0].cp); break;
		case EM_PROXIED_CLEARENV: q->returnValue.i = clearenv(); break;
		case EM_PROXIED_SETENV: q->returnValue.i = setenv(q->args[0].cp, q->args[1].cp, q->args[2].i); break;
		case EM_PROXIED_UNSETENV: q->returnValue.i = unsetenv(q->args[0].cp); break;
		case EM_PROXIED_PUTENV: q->returnValue.i = putenv(q->args[0].cp); break;
		case EM_PROXIED_TZSET: tzset(); break;
		case EM_PROXIED_PTHREAD_CREATE: q->returnValue.i = pthread_create(q->args[0].vp, q->args[1].vp, q->args[2].vp, q->args[3].vp); break;
		case EM_PROXIED_SYSCALL: q->returnValue.i = emscripten_syscall(q->args[0].i, q->args[1].vp); break;
		case EM_FUNC_SIG_V: ((em_func_v)q->functionPtr)(); break;
		case EM_FUNC_SIG_VI: ((em_func_vi)q->functionPtr)(q->args[0].i); break;
		case EM_FUNC_SIG_VII: ((em_func_vii)q->functionPtr)(q->args[0].i, q->args[1].i); break;
		case EM_FUNC_SIG_VIII: ((em_func_viii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_FUNC_SIG_I: q->returnValue.i = ((em_func_i)q->functionPtr)(); break;
		case EM_FUNC_SIG_II: q->returnValue.i = ((em_func_ii)q->functionPtr)(q->args[0].i); break;
		case EM_FUNC_SIG_III: q->returnValue.i = ((em_func_iii)q->functionPtr)(q->args[0].i, q->args[1].i); break;
		case EM_FUNC_SIG_IIII: q->returnValue.i = ((em_func_iiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i); break;
		default: assert(0 && "Invalid Emscripten pthread _do_call opcode!");
	}

	// If the caller is detached from this operation, it is the main thread's responsibility to free up the call object.
	if (q->calleeDelete) {
		emscripten_async_waitable_close(q);
		// No need to wake a listener, nothing is listening to this since the call object is detached.
	} else {
		// The caller owns this call object, it is listening to it and will free it up.
		q->operationDone = 1;
		emscripten_futex_wake(&q->operationDone, INT_MAX);
	}
}

#define CALL_QUEUE_SIZE 128
static em_queued_call **call_queue = 0;
static volatile int call_queue_head = 0; // Shared data synchronized by call_queue_lock.
static volatile int call_queue_tail = 0;
static pthread_mutex_t call_queue_lock = PTHREAD_MUTEX_INITIALIZER;

EMSCRIPTEN_RESULT emscripten_wait_for_call_v(em_queued_call *call, double timeoutMSecs)
{
	int r;

	int done = emscripten_atomic_load_u32(&call->operationDone);
	if (!done) {
		double now = emscripten_get_now();
		double waitEndTime = now + timeoutMSecs;
		emscripten_set_current_thread_status(EM_THREAD_STATUS_WAITPROXY);
		while(!done && now < waitEndTime) {
			r = emscripten_futex_wait(&call->operationDone, 0, waitEndTime - now);
			done = emscripten_atomic_load_u32(&call->operationDone);
			now = emscripten_get_now();
		}
		emscripten_set_current_thread_status(EM_THREAD_STATUS_RUNNING);
	}
	if (done) return EMSCRIPTEN_RESULT_SUCCESS;
	else return EMSCRIPTEN_RESULT_TIMED_OUT;
}

EMSCRIPTEN_RESULT emscripten_wait_for_call_i(em_queued_call *call, double timeoutMSecs, int *outResult)
{
	EMSCRIPTEN_RESULT res = emscripten_wait_for_call_v(call, timeoutMSecs);
	if (res == EMSCRIPTEN_RESULT_SUCCESS && outResult) *outResult = call->returnValue.i;
	return res;
}

void EMSCRIPTEN_KEEPALIVE emscripten_async_run_in_main_thread(em_queued_call *call)
{
	assert(call);
	// If we are the main Emscripten runtime thread, we can just call the operation directly.
	if (emscripten_is_main_runtime_thread()) {
		_do_call(call);
		return;
	}

	// Add the operation to the call queue of the main runtime thread.
	pthread_mutex_lock(&call_queue_lock);
	if (!call_queue) call_queue = malloc(sizeof(em_queued_call*) * CALL_QUEUE_SIZE); // Shared data synchronized by call_queue_lock.

	int head = emscripten_atomic_load_u32((void*)&call_queue_head);
	int tail = emscripten_atomic_load_u32((void*)&call_queue_tail);
	int new_tail = (tail + 1) % CALL_QUEUE_SIZE;

	while(new_tail == head) { // Queue is full?
		pthread_mutex_unlock(&call_queue_lock);
		emscripten_futex_wait((void*)&call_queue_head, head, INFINITY);
		pthread_mutex_lock(&call_queue_lock);
		head = emscripten_atomic_load_u32((void*)&call_queue_head);
		tail = emscripten_atomic_load_u32((void*)&call_queue_tail);
		new_tail = (tail + 1) % CALL_QUEUE_SIZE;
	}

	call_queue[tail] = call;

	// If the call queue was empty, the main runtime thread is likely idle in the browser event loop,
	// so send a message to it to ensure that it wakes up to start processing the command we have posted.
	if (head == tail) {
		EM_ASM(postMessage({ cmd: 'processQueuedMainThreadWork' }));
	}

	emscripten_atomic_store_u32((void*)&call_queue_tail, new_tail);

	pthread_mutex_unlock(&call_queue_lock);
}

void EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread(em_queued_call *call)
{
	emscripten_async_run_in_main_thread(call);

	// Enter to wait for the operation to complete.
	emscripten_wait_for_call_v(call, INFINITY);
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_0(int function)
{
	em_queued_call q = { function };
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_1(int function, void *arg1)
{
	em_queued_call q = { function };
	q.args[0].vp = arg1;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_2(int function, void *arg1, void *arg2)
{
	em_queued_call q = { function };
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
	em_queued_call q = { function };
	q.args[0].vp = (void*)param0;
	q.args[1].vp = s;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	if (s != str) free(s);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_3(int function, void *arg1, void *arg2, void *arg3)
{
	em_queued_call q = { function };
	q.args[0].vp = arg1;
	q.args[1].vp = arg2;
	q.args[2].vp = arg3;
	q.returnValue.vp = 0;
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.vp;
}

void * EMSCRIPTEN_KEEPALIVE emscripten_sync_run_in_main_thread_4(int function, void *arg1, void *arg2, void *arg3, void *arg4)
{
	em_queued_call q = { function };
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
	em_queued_call q = { function };
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
	em_queued_call q = { function };
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
	em_queued_call q = { function };
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
	int head = emscripten_atomic_load_u32((void*)&call_queue_head);
	int tail = emscripten_atomic_load_u32((void*)&call_queue_tail);
	while (head != tail)
	{
		// Assume that the call is heavy, so unlock access to the call queue while it is being performed.
		pthread_mutex_unlock(&call_queue_lock);
		_do_call(call_queue[head]);
		pthread_mutex_lock(&call_queue_lock);

		head = (head + 1) % CALL_QUEUE_SIZE;
		emscripten_atomic_store_u32((void*)&call_queue_head, head);
		tail = emscripten_atomic_load_u32((void*)&call_queue_tail);
	}
	pthread_mutex_unlock(&call_queue_lock);

	// If the queue was full and we had waiters pending to get to put data to queue, wake them up.
	emscripten_futex_wake((void*)&call_queue_head, 0x7FFFFFFF);

	bool_inside_nested_process_queued_calls = 0;
}

int emscripten_sync_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr, ...)
{
	va_list args;
	va_start(args, func_ptr);
	int numArguments = EM_FUNC_SIG_NUM_FUNC_ARGUMENTS(sig);
	em_queued_call q = { sig, func_ptr };
	for(int i = 0; i < numArguments; ++i)
		q.args[i].i = va_arg(args, int);
	va_end(args);
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.i;
}

void emscripten_async_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr, ...)
{
	va_list args;
	va_start(args, func_ptr);
	int numArguments = EM_FUNC_SIG_NUM_FUNC_ARGUMENTS(sig);
	em_queued_call *q = em_queued_call_malloc();
	q->functionEnum = sig;
	q->functionPtr = func_ptr;
	for(int i = 0; i < numArguments; ++i)
		q->args[i].i = va_arg(args, int);
	va_end(args);
	// 'async' runs are fire and forget, where the caller detaches itself from the call object after returning here,
	// and it is the callee's responsibility to free up the memory after the call has been performed.
	q->calleeDelete = 1;
	emscripten_async_run_in_main_thread(q);
}

em_queued_call *emscripten_async_waitable_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr, ...)
{
	va_list args;
	va_start(args, func_ptr);
	int numArguments = EM_FUNC_SIG_NUM_FUNC_ARGUMENTS(sig);
	em_queued_call *q = em_queued_call_malloc();
	q->functionEnum = sig;
	q->functionPtr = func_ptr;
	for(int i = 0; i < numArguments; ++i)
		q->args[i].i = va_arg(args, int);
	va_end(args);
	// 'async waitable' runs are waited on by the caller, so the call object needs to remain alive for the caller to
	// access it after the operation is done. The caller is responsible in cleaning up the object after done.
	q->calleeDelete = 0;
	emscripten_async_run_in_main_thread(q);
	return q;
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

weak_alias(__pthread_testcancel, pthread_testcancel);
