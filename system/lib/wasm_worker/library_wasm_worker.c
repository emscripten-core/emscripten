#include <assert.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/threading.h>
#include <emscripten/heap.h>
#include <malloc.h>

// Options:
// #define WASM_WORKER_NO_TLS 0/1 : set to 1 to disable TLS compilation support for a small code size gain
// #define STACK_OVERFLOW_CHECK 0/1/2 : set to the current stack overflow check mode

// Internal implementation function in JavaScript side that emscripten_create_wasm_worker() calls to
// to perform the wasm worker creation.
emscripten_wasm_worker_t _emscripten_create_wasm_worker_with_tls(void *stackLowestAddress, uint32_t stackSize, void *tlsAddress, uint32_t tlsSize);
emscripten_wasm_worker_t _emscripten_create_wasm_worker_no_tls(void *stackLowestAddress, uint32_t stackSize);

void __wasm_init_tls(void *memory);

#if !WASM_WORKER_NO_TLS
__attribute__((constructor(48)))
static void emscripten_wasm_worker_main_thread_initialize() {
	uintptr_t* sbrk_ptr = emscripten_get_sbrk_ptr();
	__wasm_init_tls((void*)*sbrk_ptr);
	*sbrk_ptr += __builtin_wasm_tls_size();
}
#endif

emscripten_wasm_worker_t emscripten_create_wasm_worker_with_tls(void *stackLowestAddress, uint32_t stackSize, void *tlsAddress, uint32_t tlsSize)
{
#if WASM_WORKER_NO_TLS
	return emscripten_create_wasm_worker_no_tls(stackLowestAddress, stackSize);
#else
	assert((uintptr_t)stackLowestAddress % 16 == 0);
	assert(stackSize % 16 == 0);
	assert(__builtin_wasm_tls_align() != 0 || __builtin_wasm_tls_size() == 0); // Internal consistency check: Clang can report __builtin_wasm_tls_align to be zero if there is no TLS used - double check it never reports zero if it is used.
	assert(__builtin_wasm_tls_align() == 0 || (uintptr_t)tlsAddress % __builtin_wasm_tls_align() == 0 && "TLS memory address not aligned in a call to emscripten_create_wasm_worker_with_tls()! Please allocate memory with alignment from __builtin_wasm_tls_align() when creating a Wasm Worker!");
	assert(tlsSize != 0 || __builtin_wasm_tls_size() == 0 && "Program code contains TLS: please use function emscripten_create_wasm_worker_with_tls() to create a Wasm Worker!");
	assert(tlsSize == __builtin_wasm_tls_size() && "TLS size mismatch! Please reserve exactly __builtin_wasm_tls_size() TLS memory in a call to emscripten_create_wasm_worker_with_tls()");
	assert(tlsAddress != 0 || tlsSize == 0);
	return _emscripten_create_wasm_worker_with_tls((void*)stackLowestAddress, stackSize, tlsAddress, tlsSize);
#endif
}

emscripten_wasm_worker_t emscripten_create_wasm_worker_no_tls(void *stackLowestAddress, uint32_t stackSize)
{
#if !WASM_WORKER_NO_TLS
	return emscripten_create_wasm_worker_with_tls(stackLowestAddress, stackSize, 0, 0);
#else
	assert((uintptr_t)stackLowestAddress % 16 == 0);
	assert(stackSize % 16 == 0);
	assert(__builtin_wasm_tls_size() == 0 && "Cannot disable TLS with -sWASM_WORKERS_NO_TLS=1 when compiling code that does require TLS! Rebuild with -sWASM_WORKERS_NO_TLS=1 removed, or remove uses of TLS from the codebase.");
	return _emscripten_create_wasm_worker_no_tls((void*)stackLowestAddress, stackSize);
#endif
}

emscripten_wasm_worker_t emscripten_malloc_wasm_worker(uint32_t stackSize)
{
#if WASM_WORKER_NO_TLS
	return emscripten_create_wasm_worker_no_tls(memalign(16, stackSize), stackSize);
#else
	uint32_t tlsSize = __builtin_wasm_tls_size();
	return emscripten_create_wasm_worker_with_tls(memalign(16, stackSize), stackSize, memalign(__builtin_wasm_tls_align(), tlsSize), tlsSize);
#endif
}

void emscripten_wasm_worker_sleep(int64_t nsecs)
{
	int32_t addr = 0;
	emscripten_wasm_wait_i32(&addr, 0, nsecs);
}

void emscripten_lock_init(emscripten_lock_t *lock)
{
	emscripten_atomic_store_u32((void*)lock, EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER);
}

EM_BOOL emscripten_lock_wait_acquire(emscripten_lock_t *lock, int64_t maxWaitNanoseconds)
{
	emscripten_lock_t val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
	if (!val) return EM_TRUE;
	int64_t waitEnd = (int64_t)(emscripten_performance_now() * 1e6) + maxWaitNanoseconds;
	while(maxWaitNanoseconds > 0)
	{
		emscripten_wasm_wait_i32((int32_t*)lock, val, maxWaitNanoseconds);
		val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
		if (!val) return EM_TRUE;
		maxWaitNanoseconds = waitEnd - (int64_t)(emscripten_performance_now() * 1e6);
	}
	return EM_FALSE;
}

void emscripten_lock_waitinf_acquire(emscripten_lock_t *lock)
{
	emscripten_lock_t val;
	do
	{
		val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
		if (val)
			emscripten_wasm_wait_i32((int32_t*)lock, val, ATOMICS_WAIT_DURATION_INFINITE);
	} while(val);
}

EM_BOOL emscripten_lock_busyspin_wait_acquire(emscripten_lock_t *lock, double maxWaitMilliseconds)
{
	emscripten_lock_t val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
	if (!val) return EM_TRUE;

	double t = emscripten_performance_now();
	double waitEnd = t + maxWaitMilliseconds;
	while(t < waitEnd)
	{
		val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
		if (!val) return EM_TRUE;
		t = emscripten_performance_now();
	}
	return EM_FALSE;
}

void emscripten_lock_busyspin_waitinf_acquire(emscripten_lock_t *lock)
{
	emscripten_lock_t val;
	do
	{
		val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
	} while(val);
}

EM_BOOL emscripten_lock_try_acquire(emscripten_lock_t *lock)
{
	emscripten_lock_t val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
	return !val;
}

void emscripten_lock_release(emscripten_lock_t *lock)
{
	emscripten_atomic_store_u32((void*)lock, 0);
	emscripten_wasm_notify((int32_t*)lock, 1);
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
			ATOMICS_WAIT_RESULT_T waitResult = emscripten_wasm_wait_i32((int32_t*)sem, val, maxWaitNanoseconds);
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
			emscripten_wasm_wait_i32((int32_t*)sem, val, ATOMICS_WAIT_DURATION_INFINITE);
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
	emscripten_wasm_notify((int*)sem, num);
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
	emscripten_wasm_wait_i32((int32_t*)condvar, val, ATOMICS_WAIT_DURATION_INFINITE);
	emscripten_lock_waitinf_acquire(lock);
}

int emscripten_condvar_wait(emscripten_condvar_t *condvar, emscripten_lock_t *lock, int64_t maxWaitNanoseconds)
{
	int val = emscripten_atomic_load_u32((void*)condvar);
	emscripten_lock_release(lock);
	int waitValue = emscripten_wasm_wait_i32((int32_t*)condvar, val, maxWaitNanoseconds);
	if (waitValue == ATOMICS_WAIT_TIMED_OUT)
		return EM_FALSE;

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
	emscripten_wasm_notify((int*)condvar, numWaitersToSignal);
}
