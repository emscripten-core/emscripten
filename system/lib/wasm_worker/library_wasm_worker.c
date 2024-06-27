#include <assert.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <emscripten/heap.h>
#include <emscripten/stack.h>
#include <emscripten/console.h>
#include <malloc.h>
#include <sys/param.h> // For MAX()

#include "emscripten_internal.h"

#ifndef __EMSCRIPTEN_WASM_WORKERS__
#error __EMSCRIPTEN_WASM_WORKERS__ should be defined when building this file!
#endif

#define ROUND_UP(x, ALIGNMENT) (((x)+ALIGNMENT-1)&-ALIGNMENT)
#define SBRK_ALIGN (__alignof__(max_align_t))
#define STACK_ALIGN __BIGGEST_ALIGNMENT__

// Options:
// #define STACK_OVERFLOW_CHECK 0/1/2 : set to the current stack overflow check mode

void __wasm_init_tls(void *memory);

__attribute__((constructor(48)))
static void emscripten_wasm_worker_main_thread_initialize() {
	uintptr_t* sbrk_ptr = emscripten_get_sbrk_ptr();
	assert((*sbrk_ptr % STACK_ALIGN) == 0);
	if (__builtin_wasm_tls_align() > STACK_ALIGN) {
		*sbrk_ptr = ROUND_UP(*sbrk_ptr, __builtin_wasm_tls_align());
	}
	__wasm_init_tls((void*)*sbrk_ptr);
	*sbrk_ptr += ROUND_UP(__builtin_wasm_tls_size(), SBRK_ALIGN);
}

emscripten_wasm_worker_t emscripten_create_wasm_worker(void *stackPlusTLSAddress, size_t stackPlusTLSSize)
{
	assert(stackPlusTLSAddress != 0);
	assert((uintptr_t)stackPlusTLSAddress % STACK_ALIGN == 0);
	assert(stackPlusTLSSize > 0);
	assert(stackPlusTLSSize % STACK_ALIGN == 0);

	// Guard against a programming oopsie: The target Worker's stack cannot be part of the calling
	// thread's stack.
	assert(emscripten_stack_get_base() <= (uintptr_t)stackPlusTLSAddress || emscripten_stack_get_end() >= (uintptr_t)stackPlusTLSAddress + stackPlusTLSSize
		&& "When creating a Wasm Worker, its stack should be located either in global data or on the heap, not on the calling thread's own stack!");

#ifndef NDEBUG
	// The Worker's TLS area will be spliced off from the stack region, so the
	// stack needs to be at least as large as the TLS region.
	uint32_t tlsSize = ROUND_UP(__builtin_wasm_tls_size(), __builtin_wasm_tls_align());
	assert(stackPlusTLSSize > tlsSize);
#endif
	// The TLS region lives at the start of the stack region (the lowest address
	// of the stack).  Since the TLS data alignment may be larger than stack
	// alignment, we may need to round up the lowest stack address to meet this
	// requirment.
	if (__builtin_wasm_tls_align() > STACK_ALIGN) {
		uintptr_t tlsBase = (uintptr_t)stackPlusTLSAddress;
		tlsBase = ROUND_UP(tlsBase, __builtin_wasm_tls_align());
		size_t padding = tlsBase - (uintptr_t)stackPlusTLSAddress;
		stackPlusTLSAddress = (void*)tlsBase;
		stackPlusTLSSize -= padding;
	}
	return _emscripten_create_wasm_worker(stackPlusTLSAddress, stackPlusTLSSize);
}

emscripten_wasm_worker_t emscripten_malloc_wasm_worker(size_t stackSize)
{
	// Add the TLS size to the provided stackSize so that the allocation
	// will always be large enough to hold the worker TLS data.
	stackSize += ROUND_UP(__builtin_wasm_tls_size(), STACK_ALIGN);
	void* stackPlusTLSAddress = emscripten_builtin_memalign(MAX(__builtin_wasm_tls_align(), STACK_ALIGN), stackSize);
	return emscripten_create_wasm_worker(stackPlusTLSAddress, stackSize);
}

void emscripten_wasm_worker_sleep(int64_t nsecs)
{
	int32_t addr = 0;
	emscripten_atomic_wait_u32(&addr, 0, nsecs);
}

void emscripten_lock_init(emscripten_lock_t *lock)
{
	emscripten_atomic_store_u32((void*)lock, EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER);
}

bool emscripten_lock_wait_acquire(emscripten_lock_t *lock, int64_t maxWaitNanoseconds)
{
	emscripten_lock_t val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
	if (!val) return true;
	int64_t waitEnd = (int64_t)(emscripten_performance_now() * 1e6) + maxWaitNanoseconds;
	while(maxWaitNanoseconds > 0)
	{
		emscripten_atomic_wait_u32((int32_t*)lock, val, maxWaitNanoseconds);
		val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
		if (!val) return true;
		maxWaitNanoseconds = waitEnd - (int64_t)(emscripten_performance_now() * 1e6);
	}
	return false;
}

void emscripten_lock_waitinf_acquire(emscripten_lock_t *lock)
{
	emscripten_lock_t val;
	do
	{
		val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
		if (val)
			emscripten_atomic_wait_u32((int32_t*)lock, val, ATOMICS_WAIT_DURATION_INFINITE);
	} while(val);
}

bool emscripten_lock_busyspin_wait_acquire(emscripten_lock_t *lock, double maxWaitMilliseconds)
{
	emscripten_lock_t val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
	if (!val) return true;

	double t = emscripten_performance_now();
	double waitEnd = t + maxWaitMilliseconds;
	while(t < waitEnd)
	{
		val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
		if (!val) return true;
		t = emscripten_performance_now();
	}
	return false;
}

void emscripten_lock_busyspin_waitinf_acquire(emscripten_lock_t *lock)
{
	emscripten_lock_t val;
	do
	{
		val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
	} while(val);
}

bool emscripten_lock_try_acquire(emscripten_lock_t *lock)
{
	emscripten_lock_t val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
	return !val;
}

void emscripten_lock_release(emscripten_lock_t *lock)
{
	emscripten_atomic_store_u32((void*)lock, 0);
	emscripten_atomic_notify((int32_t*)lock, 1);
}

void emscripten_semaphore_init(emscripten_semaphore_t *sem, int num)
{
	emscripten_atomic_store_u32((void*)sem, num);
}

int emscripten_semaphore_try_acquire(emscripten_semaphore_t *sem, int num)
{
	uint32_t val = num;
	for(;;)
	{
		uint32_t ret = emscripten_atomic_cas_u32((void*)sem, val, val - num);
		if (ret == val) return val - num;
		if (ret < num) return -1;
		val = ret;
	}
}

int emscripten_semaphore_wait_acquire(emscripten_semaphore_t *sem, int num, int64_t maxWaitNanoseconds)
{
	int val = emscripten_atomic_load_u32((void*)sem);
	for(;;)
	{
		while(val < num)
		{
			// TODO: Shave off maxWaitNanoseconds
			ATOMICS_WAIT_RESULT_T waitResult = emscripten_atomic_wait_u32((int32_t*)sem, val, maxWaitNanoseconds);
			if (waitResult == ATOMICS_WAIT_TIMED_OUT) return -1;
			val = emscripten_atomic_load_u32((void*)sem);
		}
		int ret = (int)emscripten_atomic_cas_u32((void*)sem, val, val - num);
		if (ret == val) return val - num;
		val = ret;
	}
}

int emscripten_semaphore_waitinf_acquire(emscripten_semaphore_t *sem, int num)
{
	int val = emscripten_atomic_load_u32((void*)sem);
	for(;;)
	{
		while(val < num)
		{
			emscripten_atomic_wait_u32((int32_t*)sem, val, ATOMICS_WAIT_DURATION_INFINITE);
			val = emscripten_atomic_load_u32((void*)sem);
		}
		int ret = (int)emscripten_atomic_cas_u32((void*)sem, val, val - num);
		if (ret == val) return val - num;
		val = ret;
	}
}

uint32_t emscripten_semaphore_release(emscripten_semaphore_t *sem, int num)
{
	uint32_t ret = emscripten_atomic_add_u32((void*)sem, num);
	emscripten_atomic_notify((int*)sem, num);
	return ret;
}

void emscripten_condvar_init(emscripten_condvar_t *condvar)
{
	*condvar = EMSCRIPTEN_CONDVAR_T_STATIC_INITIALIZER;
}

void emscripten_condvar_waitinf(emscripten_condvar_t *condvar, emscripten_lock_t *lock)
{
	int val = emscripten_atomic_load_u32((void*)condvar);
	emscripten_lock_release(lock);
	emscripten_atomic_wait_u32((int32_t*)condvar, val, ATOMICS_WAIT_DURATION_INFINITE);
	emscripten_lock_waitinf_acquire(lock);
}

bool emscripten_condvar_wait(emscripten_condvar_t *condvar, emscripten_lock_t *lock, int64_t maxWaitNanoseconds)
{
	int val = emscripten_atomic_load_u32((void*)condvar);
	emscripten_lock_release(lock);
	int waitValue = emscripten_atomic_wait_u32((int32_t*)condvar, val, maxWaitNanoseconds);
	if (waitValue == ATOMICS_WAIT_TIMED_OUT)
		return false;

	return emscripten_lock_wait_acquire(lock, maxWaitNanoseconds);
}

ATOMICS_WAIT_TOKEN_T emscripten_condvar_wait_async(emscripten_condvar_t *condvar,
                                                  emscripten_lock_t *lock,
                                                  void (*asyncWaitFinished)(int32_t *address, uint32_t value, ATOMICS_WAIT_RESULT_T waitResult, void *userData),
                                                  void *userData,
                                                  double maxWaitMilliseconds)
{
	int val = emscripten_atomic_load_u32((void*)condvar);
	emscripten_lock_release(lock);
	return emscripten_atomic_wait_async((int32_t *)condvar, val, asyncWaitFinished, userData, maxWaitMilliseconds);
}

void emscripten_condvar_signal(emscripten_condvar_t *condvar, int64_t numWaitersToSignal)
{
	emscripten_atomic_add_u32((void*)condvar, 1);
	emscripten_atomic_notify((int*)condvar, numWaitersToSignal);
}
