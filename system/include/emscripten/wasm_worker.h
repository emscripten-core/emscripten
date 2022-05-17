#pragma once

#include <stdint.h>
#include <emscripten/html5.h>

#ifdef __cplusplus
extern "C" {
#endif

#define emscripten_wasm_worker_t int
#define EMSCRIPTEN_WASM_WORKER_ID_PARENT 0

/* Creates a new Worker() that is attached to executing this WebAssembly.Instance and WebAssembly.Memory.

   emscripten_malloc_wasm_worker: Creates a new Worker, dynamically allocating stack and TLS for it. Unfortunately
                                  due to the asynchronous no-notifications nature of how Worker API specification teardown
                                  behaves, the dynamically allocated memory can never be freed, so use this function
                                  only in scenarios where the page does not need to deinitialize/tear itself down.

   emscripten_create_wasm_worker: Creates a Wasm Worker on given placed stack address.
                                  Use this function to manually manage the memory that a Worker should use.
                                  This function does not use any dynamic memory allocation.

   Returns an ID that represents the given Worker. If not building with Wasm workers enabled (-sWASM_WORKERS=0),
   these functions will return 0 to denote failure.
   Note that the Worker will be loaded up asynchronously, and initially will not be executing any code. Use
   emscripten_wasm_worker_post_function_*() set of functions to start executing code on the Worker. */
emscripten_wasm_worker_t emscripten_malloc_wasm_worker(uint32_t stackSize);
emscripten_wasm_worker_t emscripten_create_wasm_worker(void *stackLowestAddress, uint32_t stackSize);

// Terminates the given Wasm Worker some time after it has finished executing its current, or possibly some subsequent
// posted functions. Note that this function is not C++ RAII safe, but you must manually coordinate to release any
// resources from the given Worker that it may have allocated from the heap or may have stored on its TLS slots.
// There are no TLS destructors that would execute.
// Exists, but is a no-op if not building with Wasm Workers enabled (-sWASM_WORKERS=0)
void emscripten_terminate_wasm_worker(emscripten_wasm_worker_t id);

// Note the comment on emscripten_terminate_wasm_worker(id) about thread destruction.
// Exists, but is a no-op if not building with Wasm Workers enabled (-sWASM_WORKERS=0)
void emscripten_terminate_all_wasm_workers(void);

// Returns EM_TRUE if the current thread is executing a Wasm Worker, EM_FALSE otherwise.
// Note that calling this function can be relatively slow as it incurs a Wasm->JS transition,
// so avoid calling it in hot paths.
EM_BOOL emscripten_current_thread_is_wasm_worker(void);

// Returns a unique ID that identifies the calling Wasm Worker. Similar to pthread_self().
// The main browser thread will return 0 as the ID. First Wasm Worker will return 1, and so on.
uint32_t emscripten_wasm_worker_self_id(void);

/* emscripten_wasm_worker_post_function_*: Post a pointer to a C/C++ function to be executed on the
  target Wasm Worker (via sending a postMessage() to the target thread). Notes:
 - If running inside a Wasm Worker, specify worker ID 0 to pass a message to the parent thread.
 - When specifying non-zero ID, the target worker must have been created by the calling thread. That is,
   a Wasm Worker can only send a message to its parent or its children, but not to its siblings.
 - The target function pointer will be executed on the target Worker only after it yields back to its
   event loop. If the target Wasm Worker executes an infinite loop that never yields, then the function
   pointer will never be called.
 - Passing messages between threads with this family of functions is relatively slow and has a really high
   latency cost compared to direct coordination using atomics and synchronization primitives like mutexes
   and synchronization primitives. Additionally these functions will generate garbage on the JS heap.
   Therefore avoid using these functions where performance is critical. */
void emscripten_wasm_worker_post_function_v(emscripten_wasm_worker_t id, void (*funcPtr)(void));
void emscripten_wasm_worker_post_function_vi(emscripten_wasm_worker_t id, void (*funcPtr)(int), int arg0);
void emscripten_wasm_worker_post_function_vii(emscripten_wasm_worker_t id, void (*funcPtr)(int, int), int arg0, int arg1);
void emscripten_wasm_worker_post_function_viii(emscripten_wasm_worker_t id, void (*funcPtr)(int, int, int), int arg0, int arg1, int arg2);
void emscripten_wasm_worker_post_function_vd(emscripten_wasm_worker_t id, void (*funcPtr)(double), double arg0);
void emscripten_wasm_worker_post_function_vdd(emscripten_wasm_worker_t id, void (*funcPtr)(double, double), double arg0, double arg1);
void emscripten_wasm_worker_post_function_vddd(emscripten_wasm_worker_t id, void (*funcPtr)(double, double, double), double arg0, double arg1, double arg2);
void emscripten_wasm_worker_post_function_sig(emscripten_wasm_worker_t id, void *funcPtr, const char *sig, ...);

#define ATOMICS_WAIT_RESULT_T int

// Numbering dictated by https://github.com/WebAssembly/threads/blob/master/proposals/threads/Overview.md#wait
#define ATOMICS_WAIT_OK 0
#define ATOMICS_WAIT_NOT_EQUAL 1
#define ATOMICS_WAIT_TIMED_OUT 2

#define ATOMICS_WAIT_DURATION_INFINITE -1ll

// Issues the wasm 'memory.atomic.wait32' instruction:
// If the given memory address contains value 'expectedValue', puts the calling thread to sleep to wait for that address to be notified.
// Returns one of the ATOMICS_WAIT_* return codes.
// NOTE: This function takes in the wait value in int64_t nanosecond units. Pass in maxWaitNanoseconds = -1 (or ATOMICS_WAIT_DURATION_INFINITE) to wait infinitely long.
static __attribute__((always_inline)) ATOMICS_WAIT_RESULT_T emscripten_wasm_wait_i32(int32_t *address, int expectedValue, int64_t maxWaitNanoseconds)
{
	return __builtin_wasm_memory_atomic_wait32(address, expectedValue, maxWaitNanoseconds);
}

// Issues the wasm 'memory.atomic.wait64' instruction:
// If the given memory address contains value 'expectedValue', puts the calling thread to sleep to wait for that address to be notified.
// Returns one of the ATOMICS_WAIT_* return codes.
// NOTE: This function takes in the wait value in int64_t nanosecond units. Pass in maxWaitNanoseconds = -1 (or ATOMICS_WAIT_DURATION_INFINITE) to wait infinitely long.
static __attribute__((always_inline)) ATOMICS_WAIT_RESULT_T emscripten_wasm_wait_i64(int64_t *address, int64_t expectedValue, int64_t maxWaitNanoseconds)
{
	return __builtin_wasm_memory_atomic_wait64(address, expectedValue, maxWaitNanoseconds);
}

#define EMSCRIPTEN_NOTIFY_ALL_WAITERS (-1LL)

// Issues the wasm 'memory.atomic.notify' instruction:
// Notifies the given number of threads waiting on a location.
// Pass count == EMSCRIPTEN_NOTIFY_ALL_WAITERS to notify all waiters on the given location.
// Returns the number of threads that were woken up.
// Note: this function is used to notify both waiters waiting on an i32 and i64 addresses.
static __attribute__((always_inline)) int64_t emscripten_wasm_notify(int32_t *address, int64_t count)
{
	return __builtin_wasm_memory_atomic_notify(address, count);
}

#define EMSCRIPTEN_WAIT_ASYNC_INFINITY __builtin_inf()

// Represents a pending 'Atomics.waitAsync' wait operation.
#define ATOMICS_WAIT_TOKEN_T int

#define EMSCRIPTEN_IS_VALID_WAIT_TOKEN(token) ((token) <= 0)

// Issues the JavaScript 'Atomics.waitAsync' instruction:
// performs an asynchronous wait operation on the main thread. If the given 'address' contains 'value', issues a
// deferred wait that will invoke the specified callback function 'asyncWaitFinished' once that
// address has been notified by another thread.
// NOTE: Unlike functions emscripten_wasm_wait_i32() and emscripten_wasm_wait_i64() which take in the
// wait timeout parameter as int64 nanosecond units, this function takes in the wait timeout parameter
// as double millisecond units. See https://github.com/WebAssembly/threads/issues/175 for more information.
// Pass in maxWaitMilliseconds == EMSCRIPTEN_WAIT_ASYNC_INFINITY (==__builtin_inf()) to wait infinitely long.
// Returns one of:
//  - ATOMICS_WAIT_NOT_EQUAL if the waitAsync operation could not be registered since the memory value did not
//    contain the value 'value'.
//  - ATOMICS_WAIT_TIMED_OUT if the waitAsync operation timeout parameter was <= 0.
//  - Any other value: denotes a 'wait token' that can be passed to function emscripten_atomic_cancel_wait_async()
//    to unregister an asynchronous wait. You can use the macro EMSCRIPTEN_IS_VALID_WAIT_TOKEN(retval) to check
//    if this function returned a valid wait token.
ATOMICS_WAIT_TOKEN_T emscripten_atomic_wait_async(int32_t *address,
                                                  uint32_t value,
                                                  void (*asyncWaitFinished)(int32_t *address, uint32_t value, ATOMICS_WAIT_RESULT_T waitResult, void *userData),
                                                  void *userData,
                                                  double maxWaitMilliseconds);

// Unregisters a pending Atomics.waitAsync operation that was established via a call to emscripten_atomic_wait_async()
// in the calling thread. Pass in the wait token handle that was received as the return value from the wait function.
// Returns EMSCRIPTEN_RESULT_SUCCESS if the cancellation was successful, or EMSCRIPTEN_RESULT_INVALID_PARAM if the
// asynchronous wait has already resolved prior and the callback has already been called.
// NOTE: Because of needing to work around issue https://github.com/WebAssembly/threads/issues/176, calling this
// function has an effect of introducing spurious wakeups to any other threads waiting on the
// same address that the async wait denoted by the token does. This means that in order to safely use this function,
// the mechanisms used in any wait code on that address must be written to be spurious wakeup safe. (this is the
// case for all the synchronization primitives declared in this header, but if you are rolling out your own, you
// need to be aware of this). If https://github.com/tc39/proposal-cancellation/issues/29 is resolved, then the
// spurious wakeups can be avoided.
EMSCRIPTEN_RESULT emscripten_atomic_cancel_wait_async(ATOMICS_WAIT_TOKEN_T waitToken);

// Cancels all pending async waits in the calling thread. Because of https://github.com/WebAssembly/threads/issues/176,
// if you are using asynchronous waits in your application, and need to be able to let GC reclaim Wasm heap memory
// when deinitializing an application, you *must* call this function to help the GC unpin all necessary memory.
// Otherwise, you can wrap the Wasm content in an iframe and unload the iframe to let GC occur. (navigating away
// from the page or closing that tab will also naturally reclaim the memory)
int emscripten_atomic_cancel_all_wait_asyncs(void);

// Cancels all pending async waits in the calling thread to the given memory address.
// Returns the number of async waits canceled.
int emscripten_atomic_cancel_all_wait_asyncs_at_address(int32_t *address);

// Sleeps the calling wasm worker for the given nanoseconds. Calling this function on the main thread
// either results in a TypeError exception (Firefox), or a silent return without waiting (Chrome),
// see https://github.com/WebAssembly/threads/issues/174
void emscripten_wasm_worker_sleep(int64_t nanoseconds);

// Returns the value of navigator.hardwareConcurrency, i.e. the number of logical threads available for
// the user agent. NOTE: If the execution environment does not support navigator.hardwareConcurrency,
// this function will return zero to signal no support. (If the value 1 is returned, then it means that
// navigator.hardwareConcurrency is supported, but there is only one logical thread of concurrency available)
int emscripten_navigator_hardware_concurrency(void);

// Returns the value of the expression "Atomics.isLockFree(byteWidth)": true if the given memory access
// width can be accessed atomically, and false otherwise. Generally will return true
// on 1, 2 and 4 byte accesses. On 8 byte accesses, behavior differs across browsers, see
//  - https://bugzilla.mozilla.org/show_bug.cgi?id=1246139
//  - https://bugs.chromium.org/p/chromium/issues/detail?id=1167449
int emscripten_atomics_is_lock_free(int byteWidth);

#define emscripten_lock_t volatile uint32_t

// Use with syntax "emscripten_lock_t l = EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER;"
#define EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER 0

void emscripten_lock_init(emscripten_lock_t *lock);

// Attempts to acquire the specified lock. If the lock is free, then this function acquires
// the lock and immediately returns EM_TRUE. If the lock is not free at the time of the call,
// the calling thread is set to synchronously sleep for at most maxWaitNanoseconds long, until
// another thread releases the lock. If the lock is acquired within that period, the function
// returns EM_TRUE. If the lock is not acquired within the specified period, then the wait
// times out and EM_FALSE is returned.
// NOTE: This function can be only called in a Worker, and not on the main browser thread,
//       because the main browser thread cannot synchronously sleep to wait for locks.
EM_BOOL emscripten_lock_wait_acquire(emscripten_lock_t *lock, int64_t maxWaitNanoseconds);

// Similar to emscripten_lock_wait_acquire(), but instead of waiting for at most a specified
// timeout value, the thread will wait indefinitely long until the lock can be acquired.
// NOTE: The only way to abort this wait is to call emscripten_terminate_wasm_worker() on the
//       Worker.
// NOTE: This function can be only called in a Worker, and not on the main browser thread,
//       because the main browser thread cannot synchronously sleep to wait for locks.
void emscripten_lock_waitinf_acquire(emscripten_lock_t *lock);

// Similar to emscripten_lock_wait_acquire(), but instead of placing the calling thread to
// sleep until the lock can be acquired, this function will burn CPU cycles attempting to
// acquire the lock, until the given timeout is met.
// This function can be called in both main thread and in Workers.
// NOTE: The wait period used for this function is specified in milliseconds instead of
//       nanoseconds, see https://github.com/WebAssembly/threads/issues/175 for details.
// NOTE: If this function is called on the main thread, be sure to use a reasonable max wait
//       value, or otherwise a "slow script dialog" notification can pop up, and can cause the
//       browser to stop executing the page.
EM_BOOL emscripten_lock_busyspin_wait_acquire(emscripten_lock_t *lock, double maxWaitMilliseconds);

// Similar to emscripten_lock_wait_acquire(), but instead of placing the calling thread to
// sleep until the lock can be acquired, this function will burn CPU cycles indefinitely until
// the given lock can be acquired.
// This function can be called in both main thread and in Workers.
// NOTE: The only way to abort this wait is to call emscripten_terminate_wasm_worker() on the
//       Worker. If called on the main thread, and the lock cannot be acquired within a reasonable
//       time period, this function will *HANG* the browser page content process, and show up
//       a "slow script dialog", and/or cause the browser to stop the page. If you call this
//       function on the main browser thread, be extra careful to analyze that the given lock will
//       be extremely fast to acquire without contention from other threads.
void emscripten_lock_busyspin_waitinf_acquire(emscripten_lock_t *lock);

// Registers an *asynchronous* lock acquire operation. The calling thread will asynchronously
// try to obtain the given lock after the calling thread yields back to the event loop. If the
// attempt is successful within maxWaitMilliseconds period, then the given callback asyncWaitFinished
// is called with waitResult == ATOMICS_WAIT_OK. If the lock is not acquired within the timeout
// period, then the callback asyncWaitFinished is called with waitResult == ATOMICS_WAIT_TIMED_OUT.
// NOTE: Unlike function emscripten_lock_wait_acquire() which takes in the
// wait timeout parameter as int64 nanosecond units, this function takes in the wait timeout parameter
// as double millisecond units. See https://github.com/WebAssembly/threads/issues/175 for more information.
// NOTE: This function can be called in both main thread and in Workers. If you use this API in Worker,
//       you cannot utilise an infinite loop programming model.
void emscripten_lock_async_acquire(emscripten_lock_t *lock,
                                   void (*asyncWaitFinished)(volatile void *address, uint32_t value, ATOMICS_WAIT_RESULT_T waitResult, void *userData),
                                   void *userData,
                                   double maxWaitMilliseconds);

// Attempts to acquire a lock, returning EM_TRUE if successful. If the lock is already held,
// this function will not sleep to wait until the lock is released, but immediately returns
// EM_FALSE.
// This function can be called on both main thread and in Workers.
EM_BOOL emscripten_lock_try_acquire(emscripten_lock_t *lock);

// Unlocks the specified lock for another thread to access. Note that locks are extermely
// lightweight, there is no "lock owner" tracking: this function does not actually check
// whether the calling thread owns the specified lock, but any thread can call this function
// to release a lock on behalf of whichever thread owns it.
// This function can be called on both main thread and in Workers.
void emscripten_lock_release(emscripten_lock_t *lock);

#define emscripten_semaphore_t volatile uint32_t

// Use with syntax emscripten_semaphore_t s = EMSCRIPTEN_SEMAPHORE_T_STATIC_INITIALIZER(num);
#define EMSCRIPTEN_SEMAPHORE_T_STATIC_INITIALIZER(num) ((int)(num))

void emscripten_semaphore_init(emscripten_semaphore_t *sem, int num);

// main thread, try acquire num instances, but do not sleep to wait if not available.
// Returns idx that was acquired or -1 if acquire failed.
int emscripten_semaphore_try_acquire(emscripten_semaphore_t *sem, int num);

// main thread, poll to try acquire num instances. Returns idx that was acquired. If you use this API in Worker, you cannot run an infinite loop.
void emscripten_semaphore_async_acquire(emscripten_semaphore_t *sem,
                                        int num,
                                        void (*asyncWaitFinished)(volatile void *address, uint32_t idx, ATOMICS_WAIT_RESULT_T result, void *userData),
                                        void *userData,
                                        double maxWaitMilliseconds);

// worker, sleep to acquire num instances. Returns idx that was acquired, or -1 if timed out unable to acquire.
int emscripten_semaphore_wait_acquire(emscripten_semaphore_t *sem, int num, int64_t maxWaitNanoseconds);

// worker, sleep infinitely long to acquire num instances. Returns idx that was acquired.
int emscripten_semaphore_waitinf_acquire(emscripten_semaphore_t *sem, int num);

// Releases the given number of resources back to the semaphore. Note
// that the ownership of resources is completely conceptual - there is
// no actual checking that the calling thread had previously acquired
// that many resouces, so programs need to keep check of their
// semaphore usage consistency themselves.
// Returns how many resources were available in the semaphore before the
// new resources were released back to the semaphore. (i.e. the index where
// the resource was put back to)
// [main thread or worker]
uint32_t emscripten_semaphore_release(emscripten_semaphore_t *sem, int num);

// Condition variable is an object that can be waited on, and another thread can signal, while coordinating an access to a related mutex.
#define emscripten_condvar_t volatile uint32_t

// Use with syntax emscripten_condvar_t cv = EMSCRIPTEN_CONDVAR_T_STATIC_INITIALIZER;
#define EMSCRIPTEN_CONDVAR_T_STATIC_INITIALIZER ((int)(0))

// Creates a new condition variable to the given memory location.
void emscripten_condvar_init(emscripten_condvar_t *condvar);

// Atomically performs the following:
// 1. releases the given lock. The lock should (but does not strictly need to) be held by the calling thread prior to this call.
// 2. sleep the calling thread to wait for the specified condition variable to be signaled.
// 3. once the sleep has finished (another thread has signaled the condition variable), the calling thread wakes up
//    and reacquires the lock prior to returning from this function.
void emscripten_condvar_waitinf(emscripten_condvar_t *condvar, emscripten_lock_t *lock);

// Same as the above, except that an attempt to wait for the condition variable to become true is only performed for a maximum duration.
// On success (no timeout), this function will return EM_TRUE. If the wait times out, this function will return EM_FALSE. In this case,
// the calling thread will not try to reacquire the lock.
EM_BOOL emscripten_condvar_wait(emscripten_condvar_t *condvar, emscripten_lock_t *lock, int64_t maxWaitNanoseconds);

// Asynchronously wait for the given condition variable to signal.
ATOMICS_WAIT_TOKEN_T emscripten_condvar_wait_async(emscripten_condvar_t *condvar,
                                                  emscripten_lock_t *lock,
                                                  void (*asyncWaitFinished)(int32_t *address, uint32_t value, ATOMICS_WAIT_RESULT_T waitResult, void *userData),
                                                  void *userData,
                                                  double maxWaitMilliseconds);

// Signals the given number of waiters on the specified condition variable.
// Pass numWaitersToSignal == EMSCRIPTEN_NOTIFY_ALL_WAITERS to wake all waiters ("broadcast" operation).
void emscripten_condvar_signal(emscripten_condvar_t *condvar, int64_t numWaitersToSignal);

#ifdef __cplusplus
}
#endif
