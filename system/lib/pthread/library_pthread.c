/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

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
		EM_ASM(throw 'Canceled!');
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

void emscripten_thread_sleep(double msecs)
{
	double now = emscripten_get_now();
	double target = now + msecs;

	__pthread_testcancel(); // pthreads spec: sleep is a cancellation point, so must test if this thread is cancelled during the sleep.
	emscripten_current_thread_process_queued_calls();

	// If we have less than this many msecs left to wait, busy spin that instead.
	const double minimumTimeSliceToSleep = 0.1;

	// main thread may need to run proxied calls, so sleep in very small slices to be responsive.
	const double maxMsecsSliceToSleep = emscripten_is_main_browser_thread() ? 1 : 100;

	emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_RUNNING, EM_THREAD_STATUS_SLEEPING);
	now = emscripten_get_now();
	while(now < target)
	{
		// Keep processing the main loop of the calling thread.
		__pthread_testcancel(); // pthreads spec: sleep is a cancellation point, so must test if this thread is cancelled during the sleep.
		emscripten_current_thread_process_queued_calls();

		now = emscripten_get_now();
		double msecsToSleep = target - now;
		if (msecsToSleep > maxMsecsSliceToSleep) msecsToSleep = maxMsecsSliceToSleep;
		if (msecsToSleep >= minimumTimeSliceToSleep) emscripten_futex_wait(&dummyZeroAddress, 0, msecsToSleep);
		now = emscripten_get_now();
	};

	emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_SLEEPING, EM_THREAD_STATUS_RUNNING);
}

int nanosleep(const struct timespec *req, struct timespec *rem)
{
	if (!req || req->tv_nsec < 0 || req->tv_nsec > 999999999L || req->tv_sec < 0) return EINVAL;
	emscripten_thread_sleep(req->tv_sec * 1000.0 + req->tv_nsec / 1e6);
	return 0;
}

int usleep(unsigned usec)
{
	emscripten_thread_sleep(usec / 1e3);
	return 0;
}

// Allocator and deallocator for em_queued_call objects.
static em_queued_call *em_queued_call_malloc()
{
	em_queued_call *call = (em_queued_call*)malloc(sizeof(em_queued_call));
	assert(call); // Not a programming error, but use assert() in debug builds to catch OOM scenarios.
	if (call)
	{
		call->operationDone = 0;
		call->functionPtr = 0;
		call->satelliteData = 0;
	}
	return call;
}
static void em_queued_call_free(em_queued_call *call)
{
	if (call) free(call->satelliteData);
	free(call);
}

void emscripten_async_waitable_close(em_queued_call *call)
{
	em_queued_call_free(call);
}

static void _do_call(em_queued_call *q)
{
	assert(EM_FUNC_SIG_NUM_FUNC_ARGUMENTS(q->functionEnum) <= EM_QUEUED_CALL_MAX_ARGS);

	switch(q->functionEnum)
	{
		case EM_PROXIED_PTHREAD_CREATE: q->returnValue.i = pthread_create(q->args[0].vp, q->args[1].vp, q->args[2].vp, q->args[3].vp); break;
		case EM_PROXIED_SYSCALL: q->returnValue.i = emscripten_syscall(q->args[0].i, q->args[1].vp); break;
		case EM_PROXIED_CREATE_CONTEXT: q->returnValue.i = emscripten_webgl_create_context(q->args[0].cp, q->args[1].vp); break;
		case EM_PROXIED_RESIZE_OFFSCREENCANVAS: q->returnValue.i = emscripten_set_canvas_element_size(q->args[0].cp, q->args[1].i, q->args[2].i); break;
		case EM_FUNC_SIG_V: ((em_func_v)q->functionPtr)(); break;
		case EM_FUNC_SIG_VI: ((em_func_vi)q->functionPtr)(q->args[0].i); break;
		case EM_FUNC_SIG_VF: ((em_func_vf)q->functionPtr)(q->args[0].f); break;
		case EM_FUNC_SIG_VII: ((em_func_vii)q->functionPtr)(q->args[0].i, q->args[1].i); break;
		case EM_FUNC_SIG_VIF: ((em_func_vif)q->functionPtr)(q->args[0].i, q->args[1].f); break;
		case EM_FUNC_SIG_VFF: ((em_func_vff)q->functionPtr)(q->args[0].f, q->args[1].f); break;
		case EM_FUNC_SIG_VIII: ((em_func_viii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_FUNC_SIG_VIIF: ((em_func_viif)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].f); break;
		case EM_FUNC_SIG_VIFF: ((em_func_viff)q->functionPtr)(q->args[0].i, q->args[1].f, q->args[2].f); break;
		case EM_FUNC_SIG_VFFF: ((em_func_vfff)q->functionPtr)(q->args[0].f, q->args[1].f, q->args[2].f); break;
		case EM_FUNC_SIG_VIIII: ((em_func_viiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i); break;
		case EM_FUNC_SIG_VIIFI: ((em_func_viifi)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].f, q->args[3].i); break;
		case EM_FUNC_SIG_VIFFF: ((em_func_vifff)q->functionPtr)(q->args[0].i, q->args[1].f, q->args[2].f, q->args[3].f); break;
		case EM_FUNC_SIG_VFFFF: ((em_func_vffff)q->functionPtr)(q->args[0].f, q->args[1].f, q->args[2].f, q->args[3].f); break;
		case EM_FUNC_SIG_VIIIII: ((em_func_viiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i); break;
		case EM_FUNC_SIG_VIFFFF: ((em_func_viffff)q->functionPtr)(q->args[0].i, q->args[1].f, q->args[2].f, q->args[3].f, q->args[4].f); break;
		case EM_FUNC_SIG_VIIIIII: ((em_func_viiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i); break;
		case EM_FUNC_SIG_VIIIIIII: ((em_func_viiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].i); break;
		case EM_FUNC_SIG_VIIIIIIII: ((em_func_viiiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].i, q->args[7].i); break;
		case EM_FUNC_SIG_VIIIIIIIII: ((em_func_viiiiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].i, q->args[7].i, q->args[8].i); break;
		case EM_FUNC_SIG_VIIIIIIIIII: ((em_func_viiiiiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].i, q->args[7].i, q->args[8].i, q->args[9].i); break;
		case EM_FUNC_SIG_VIIIIIIIIIII: ((em_func_viiiiiiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].i, q->args[7].i, q->args[8].i, q->args[9].i, q->args[10].i); break;
		case EM_FUNC_SIG_I: q->returnValue.i = ((em_func_i)q->functionPtr)(); break;
		case EM_FUNC_SIG_II: q->returnValue.i = ((em_func_ii)q->functionPtr)(q->args[0].i); break;
		case EM_FUNC_SIG_III: q->returnValue.i = ((em_func_iii)q->functionPtr)(q->args[0].i, q->args[1].i); break;
		case EM_FUNC_SIG_IIII: q->returnValue.i = ((em_func_iiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i); break;
		case EM_FUNC_SIG_IIIII: q->returnValue.i = ((em_func_iiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i); break;
		case EM_FUNC_SIG_IIIIII: q->returnValue.i = ((em_func_iiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i); break;
		case EM_FUNC_SIG_IIIIIII: q->returnValue.i = ((em_func_iiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i); break;
		case EM_FUNC_SIG_IIIIIIII: q->returnValue.i = ((em_func_iiiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].i); break;
		case EM_FUNC_SIG_IIIIIIIII: q->returnValue.i = ((em_func_iiiiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].i, q->args[7].i); break;
		case EM_FUNC_SIG_IIIIIIIIII: q->returnValue.i = ((em_func_iiiiiiiiii)q->functionPtr)(q->args[0].i, q->args[1].i, q->args[2].i, q->args[3].i, q->args[4].i, q->args[5].i, q->args[6].i, q->args[7].i, q->args[8].i); break;
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

typedef struct CallQueue
{
	void *target_thread;
	em_queued_call **call_queue;
	int call_queue_head; // Shared data synchronized by call_queue_lock.
	int call_queue_tail;
	struct CallQueue *next;
} CallQueue;

// Currently global to the queue, but this can be improved to be per-queue specific. (TODO: with lockfree list operations on callQueue_head, or removing the list by moving this data to pthread_t)
static pthread_mutex_t call_queue_lock = PTHREAD_MUTEX_INITIALIZER;
static CallQueue *callQueue_head = 0;

static CallQueue *GetQueue(void *target) // Not thread safe, call while having call_queue_lock obtained.
{
	assert(target);
	CallQueue *q = callQueue_head;
	while(q && q->target_thread != target)
		q = q->next;
	return q;
}

static CallQueue *GetOrAllocateQueue(void *target) // Not thread safe, call while having call_queue_lock obtained.
{
	CallQueue *q = GetQueue(target);
	if (q) return q;

	q = (CallQueue *)malloc(sizeof(CallQueue));
	q->target_thread = target;
	q->call_queue = 0;
	q->call_queue_head = 0;
	q->call_queue_tail = 0;
	q->next = 0;
	if (callQueue_head)
	{
		CallQueue *last = callQueue_head;
		while(last->next) last = last->next;
		last->next = q;
	}
	else
	{
		callQueue_head = q;
	}
	return q;
}

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

static pthread_t main_browser_thread_id_ = 0;

void EMSCRIPTEN_KEEPALIVE emscripten_register_main_browser_thread_id(pthread_t main_browser_thread_id)
{
	main_browser_thread_id_ = main_browser_thread_id;
}

pthread_t EMSCRIPTEN_KEEPALIVE emscripten_main_browser_thread_id()
{
	return main_browser_thread_id_;
}

static void EMSCRIPTEN_KEEPALIVE emscripten_async_queue_call_on_thread(pthread_t target_thread, em_queued_call *call)
{
	assert(call);

// #if PTHREADS_DEBUG // TODO: Create a debug version of pthreads library
//	EM_ASM_INT({dump('thread ' + _pthread_self() + ' (ENVIRONMENT_IS_WORKER: ' + ENVIRONMENT_IS_WORKER + '), queueing call of function enum=' + $0 + '/ptr=' + $1 + ' on thread ' + $2 + '\n' + new Error().stack)}, call->functionEnum, call->functionPtr, target_thread);
// #endif

	// Can't be a null pointer here, but can't be EM_CALLBACK_THREAD_CONTEXT_MAIN_BROWSER_THREAD either.
	assert(target_thread);
	if (target_thread == EM_CALLBACK_THREAD_CONTEXT_MAIN_BROWSER_THREAD) target_thread = emscripten_main_browser_thread_id();

	// If we are the target recipient of this message, we can just call the operation directly.
	if (target_thread == EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD || target_thread == pthread_self()) {
		_do_call(call);
		return;
	}

	// Add the operation to the call queue of the main runtime thread.
	pthread_mutex_lock(&call_queue_lock);
	CallQueue *q = GetOrAllocateQueue(target_thread);
	if (!q->call_queue) q->call_queue = malloc(sizeof(em_queued_call*) * CALL_QUEUE_SIZE); // Shared data synchronized by call_queue_lock.

	int head = emscripten_atomic_load_u32((void*)&q->call_queue_head);
	int tail = emscripten_atomic_load_u32((void*)&q->call_queue_tail);
	int new_tail = (tail + 1) % CALL_QUEUE_SIZE;

	while(new_tail == head) { // Queue is full?
		pthread_mutex_unlock(&call_queue_lock);

		// If queue of the main browser thread is full, then we wait. (never drop messages for the main browser thread)
		if (target_thread == emscripten_main_browser_thread_id())
		{
			emscripten_futex_wait((void*)&q->call_queue_head, head, INFINITY);
			pthread_mutex_lock(&call_queue_lock);
			head = emscripten_atomic_load_u32((void*)&q->call_queue_head);
			tail = emscripten_atomic_load_u32((void*)&q->call_queue_tail);
			new_tail = (tail + 1) % CALL_QUEUE_SIZE;
		}
		else
		{
			// For the queues of other threads, just drop the message.
// #if DEBUG TODO: a debug build of pthreads library?
//			EM_ASM(console.error('Pthread queue overflowed, dropping queued message to thread. ' + new Error().stack));
// #endif
			em_queued_call_free(call);
			return;
		}
	}

	q->call_queue[tail] = call;

	// If the call queue was empty, the main runtime thread is likely idle in the browser event loop,
	// so send a message to it to ensure that it wakes up to start processing the command we have posted.
	if (head == tail) {
		if (target_thread == emscripten_main_browser_thread_id())
		{
			EM_ASM(postMessage({ cmd: 'processQueuedMainThreadWork' }));
		}
		else
		{
			int success = EM_ASM_INT({
				if (!ENVIRONMENT_IS_PTHREAD) {
					if (!PThread.pthreads[$0] || !PThread.pthreads[$0].worker) {
// #if DEBUG
//						Module.printErr('Cannot send message to thread with ID ' + $0 + ', unknown thread ID!');
// #endif
						return 0;
					}
					PThread.pthreads[$0].worker.postMessage({ cmd: 'processThreadQueue' });
				} else {
					postMessage({ targetThread: $0, cmd: 'processThreadQueue' });
				}
				return 1;
			}, target_thread);

			// Failed to dispatch the thread, delete the crafted message.
			if (!success) {
				em_queued_call_free(call);
				pthread_mutex_unlock(&call_queue_lock);
				return;
			}

			// TODO: Need to postMessage() to a specific target Worker that is hosting target_thread....
		}
	}

	emscripten_atomic_store_u32((void*)&q->call_queue_tail, new_tail);

	pthread_mutex_unlock(&call_queue_lock);
}

void EMSCRIPTEN_KEEPALIVE emscripten_async_run_in_main_thread(em_queued_call *call)
{
	emscripten_async_queue_call_on_thread(emscripten_main_browser_thread_id(), call);
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

void EMSCRIPTEN_KEEPALIVE emscripten_current_thread_process_queued_calls()
{
// #if PTHREADS_DEBUG == 2
//	EM_ASM(console.error('thread ' + _pthread_self() + ': emscripten_current_thread_process_queued_calls(), ' + new Error().stack));
// #endif

	// TODO: Under certain conditions we may want to have a nesting guard also for pthreads (and it will certainly be cleaner that way), but
	// we don't yet have TLS variables outside pthread_set/getspecific, so convert this to TLS after TLS is implemented.
	static int bool_main_thread_inside_nested_process_queued_calls = 0;

	if (emscripten_is_main_browser_thread())
	{
		// It is possible that when processing a queued call, the call flow leads back to calling this function in a nested fashion!
		// Therefore this scenario must explicitly be detected, and processing the queue must be avoided if we are nesting, or otherwise
		// the same queued calls would be processed again and again.
		if (bool_main_thread_inside_nested_process_queued_calls) return;
		// This must be before pthread_mutex_lock(), since pthread_mutex_lock() can call back to this function.
		bool_main_thread_inside_nested_process_queued_calls = 1;
	}

	pthread_mutex_lock(&call_queue_lock);
	CallQueue *q = GetQueue(pthread_self());
	if (!q)
	{
		pthread_mutex_unlock(&call_queue_lock);
		if (emscripten_is_main_browser_thread()) bool_main_thread_inside_nested_process_queued_calls = 0;
		return;
	}

	int head = emscripten_atomic_load_u32((void*)&q->call_queue_head);
	int tail = emscripten_atomic_load_u32((void*)&q->call_queue_tail);
	while (head != tail)
	{
		// Assume that the call is heavy, so unlock access to the call queue while it is being performed.
		pthread_mutex_unlock(&call_queue_lock);
		_do_call(q->call_queue[head]);
		pthread_mutex_lock(&call_queue_lock);

		head = (head + 1) % CALL_QUEUE_SIZE;
		emscripten_atomic_store_u32((void*)&q->call_queue_head, head);
		tail = emscripten_atomic_load_u32((void*)&q->call_queue_tail);
	}
	pthread_mutex_unlock(&call_queue_lock);

	// If the queue was full and we had waiters pending to get to put data to queue, wake them up.
	emscripten_futex_wake((void*)&q->call_queue_head, 0x7FFFFFFF);

	if (emscripten_is_main_browser_thread()) bool_main_thread_inside_nested_process_queued_calls = 0;
}

void EMSCRIPTEN_KEEPALIVE emscripten_main_thread_process_queued_calls()
{
	if (!emscripten_is_main_browser_thread()) return;

	emscripten_current_thread_process_queued_calls();
}

int emscripten_sync_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr, ...)
{
	int numArguments = EM_FUNC_SIG_NUM_FUNC_ARGUMENTS(sig);
	em_queued_call q = { sig, func_ptr };

	EM_FUNC_SIGNATURE argumentsType = sig & EM_FUNC_SIG_ARGUMENTS_TYPE_MASK;
	va_list args;
	va_start(args, func_ptr);
	for(int i = 0; i < numArguments; ++i)
	{
		switch((argumentsType & EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_MASK))
		{
			case EM_FUNC_SIG_PARAM_I: q.args[i].i = va_arg(args, int); break;
			case EM_FUNC_SIG_PARAM_I64: q.args[i].i64 = va_arg(args, int64_t); break;
			case EM_FUNC_SIG_PARAM_F: q.args[i].f = (float)va_arg(args, double); break;
			case EM_FUNC_SIG_PARAM_D: q.args[i].d = va_arg(args, double); break;
		}
		argumentsType >>= EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_SHIFT;
	}
	va_end(args);
	emscripten_sync_run_in_main_thread(&q);
	return q.returnValue.i;
}

void emscripten_async_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr, ...)
{
	int numArguments = EM_FUNC_SIG_NUM_FUNC_ARGUMENTS(sig);
	em_queued_call *q = em_queued_call_malloc();
	if (!q) return;
	q->functionEnum = sig;
	q->functionPtr = func_ptr;

	EM_FUNC_SIGNATURE argumentsType = sig & EM_FUNC_SIG_ARGUMENTS_TYPE_MASK;
	va_list args;
	va_start(args, func_ptr);
	for(int i = 0; i < numArguments; ++i)
	{
		switch((argumentsType & EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_MASK))
		{
			case EM_FUNC_SIG_PARAM_I: q->args[i].i = va_arg(args, int); break;
			case EM_FUNC_SIG_PARAM_I64: q->args[i].i64 = va_arg(args, int64_t); break;
			case EM_FUNC_SIG_PARAM_F: q->args[i].f = (float)va_arg(args, double); break;
			case EM_FUNC_SIG_PARAM_D: q->args[i].d = va_arg(args, double); break;
		}
		argumentsType >>= EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_SHIFT;
	}
	va_end(args);
	// 'async' runs are fire and forget, where the caller detaches itself from the call object after returning here,
	// and it is the callee's responsibility to free up the memory after the call has been performed.
	q->calleeDelete = 1;
	emscripten_async_run_in_main_thread(q);
}

em_queued_call *emscripten_async_waitable_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr, ...)
{
	int numArguments = EM_FUNC_SIG_NUM_FUNC_ARGUMENTS(sig);
	em_queued_call *q = em_queued_call_malloc();
	if (!q) return;
	q->functionEnum = sig;
	q->functionPtr = func_ptr;

	EM_FUNC_SIGNATURE argumentsType = sig & EM_FUNC_SIG_ARGUMENTS_TYPE_MASK;
	va_list args;
	va_start(args, func_ptr);
	for(int i = 0; i < numArguments; ++i)
	{
		switch((argumentsType & EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_MASK))
		{
			case EM_FUNC_SIG_PARAM_I: q->args[i].i = va_arg(args, int); break;
			case EM_FUNC_SIG_PARAM_I64: q->args[i].i64 = va_arg(args, int64_t); break;
			case EM_FUNC_SIG_PARAM_F: q->args[i].f = (float)va_arg(args, double); break;
			case EM_FUNC_SIG_PARAM_D: q->args[i].d = va_arg(args, double); break;
		}
		argumentsType >>= EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_SHIFT;
	}
	va_end(args);
	// 'async waitable' runs are waited on by the caller, so the call object needs to remain alive for the caller to
	// access it after the operation is done. The caller is responsible in cleaning up the object after done.
	q->calleeDelete = 0;
	emscripten_async_run_in_main_thread(q);
	return q;
}

void EMSCRIPTEN_KEEPALIVE emscripten_async_queue_on_thread_(pthread_t targetThread, EM_FUNC_SIGNATURE sig, void *func_ptr, void *satellite, ...)
{
	int numArguments = EM_FUNC_SIG_NUM_FUNC_ARGUMENTS(sig);
	em_queued_call *q = em_queued_call_malloc();
	assert(q);
	if (!q) return;
	q->functionEnum = sig;
	q->functionPtr = func_ptr;
	q->satelliteData = satellite;

	EM_FUNC_SIGNATURE argumentsType = sig & EM_FUNC_SIG_ARGUMENTS_TYPE_MASK;
	va_list args;
	va_start(args, satellite);
	for(int i = 0; i < numArguments; ++i)
	{
		switch((argumentsType & EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_MASK))
		{
			case EM_FUNC_SIG_PARAM_I: q->args[i].i = va_arg(args, int); break;
			case EM_FUNC_SIG_PARAM_I64: q->args[i].i64 = va_arg(args, int64_t); break;
			case EM_FUNC_SIG_PARAM_F: q->args[i].f = (float)va_arg(args, double); break;
			case EM_FUNC_SIG_PARAM_D: q->args[i].d = va_arg(args, double); break;
		}
		argumentsType >>= EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_SHIFT;
	}
	va_end(args);

	// 'async' runs are fire and forget, where the caller detaches itself from the call object after returning here,
	// and it is the callee's responsibility to free up the memory after the call has been performed.
	q->calleeDelete = 1;
	emscripten_async_queue_call_on_thread(targetThread, q);
}

int llvm_memory_barrier()
{
	emscripten_atomic_fence();
}

int llvm_atomic_load_add_i32_p0i32(int *ptr, int delta)
{
	return emscripten_atomic_add_u32(ptr, delta);
}

typedef struct main_args
{
  int argc;
  char **argv;
} main_args;

extern int __call_main(int argc, char **argv);

void *__emscripten_thread_main(void *param)
{
  emscripten_set_thread_name(pthread_self(), "Application main thread"); // This is the main runtime thread for the application.
  main_args *args = (main_args*)param;
  return (void*)__call_main(args->argc, args->argv);
}

static main_args _main_arguments;

// TODO: Create a separate library of this to be able to drop EMSCRIPTEN_KEEPALIVE from this definition.
int EMSCRIPTEN_KEEPALIVE proxy_main(int argc, char **argv)
{
  if (emscripten_has_threading_support())
  {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // TODO: Read this from -s STACK_SIZE parameter, and make actual main browser thread stack something tiny, or create a -s PROXY_THREAD_STACK_SIZE parameter.
#define EMSCRIPTEN_PTHREAD_STACK_SIZE (128*1024)

    pthread_attr_setstacksize(&attr, (EMSCRIPTEN_PTHREAD_STACK_SIZE));
    // TODO: Add a -s PROXY_CANVASES_TO_THREAD=parameter or similar to allow configuring this
#ifdef EMSCRIPTEN_PTHREAD_TRANSFERRED_CANVASES
    // If user has defined EMSCRIPTEN_PTHREAD_TRANSFERRED_CANVASES, then transfer those canvases over to the pthread.
    emscripten_pthread_attr_settransferredcanvases(&attr, (EMSCRIPTEN_PTHREAD_TRANSFERRED_CANVASES));
#else
    // Otherwise by default, transfer whatever is set to Module.canvas.
    if (EM_ASM_INT(return !!(Module['canvas']))) emscripten_pthread_attr_settransferredcanvases(&attr, "#canvas");
#endif
    _main_arguments.argc = argc;
    _main_arguments.argv = argv;
    pthread_t thread;
    int rc = pthread_create(&thread, &attr, __emscripten_thread_main, (void*)&_main_arguments);
    pthread_attr_destroy(&attr);
    if (rc)
    {
      // Proceed by running main() on the main browser thread as a fallback.
      return __call_main(_main_arguments.argc, _main_arguments.argv);
    }
    EM_ASM(Module['noExitRuntime'] = true);
    return 0;
  }
  else
  {
    return __call_main(_main_arguments.argc, _main_arguments.argv);
  }
}

weak_alias(__pthread_testcancel, pthread_testcancel);
