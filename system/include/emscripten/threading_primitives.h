/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <emscripten/atomic.h>

#ifdef __cplusplus
extern "C" {
#endif

// Similar to emscripten_async_wait_callback_t but with a volatile first
// argument.
typedef void (*emscripten_async_wait_volatile_callback_t)(volatile void* address, uint32_t value, ATOMICS_WAIT_RESULT_T waitResult, void* userData);

#define emscripten_lock_t volatile uint32_t

// Use with syntax "emscripten_lock_t l = EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER;"
#define EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER 0

void emscripten_lock_init(emscripten_lock_t * _Nonnull lock);

// Attempts to acquire the specified lock. If the lock is free, then this
// function acquires the lock and immediately returns true. If the lock is
// not free at the time of the call, the calling thread is set to synchronously
// sleep for at most maxWaitNanoseconds long, until another thread releases the
// lock. If the lock is acquired within that period, the function returns
// true. If the lock is not acquired within the specified period, then the
// wait times out and false is returned.
// NOTE: This function can be only called in a Worker, and not on the main
//       browser thread, because the main browser thread cannot synchronously
//       sleep to wait for locks.

bool emscripten_lock_wait_acquire(emscripten_lock_t * _Nonnull lock, int64_t maxWaitNanoseconds);

// Similar to emscripten_lock_wait_acquire(), but instead of waiting for at most
// a specified timeout value, the thread will wait indefinitely long until the
// lock can be acquired.
// NOTE: The only way to abort this wait is to call
//       emscripten_terminate_wasm_worker() on the Worker.
// NOTE: This function can be only called in a Worker, and not on the main
//       browser thread, because the main browser thread cannot synchronously
//       sleep to wait for locks.
void emscripten_lock_waitinf_acquire(emscripten_lock_t * _Nonnull lock);

// Similar to emscripten_lock_wait_acquire(), but instead of placing the calling
// thread to sleep until the lock can be acquired, this function will burn CPU
// cycles attempting to acquire the lock, until the given timeout is met.
// This function can be called in both main thread and in Workers.
// NOTE: The wait period used for this function is specified in milliseconds
//       instead of nanoseconds, see
//       https://github.com/WebAssembly/threads/issues/175 for details.
// NOTE: If this function is called on the main thread, be sure to use a
//       reasonable max wait value, or otherwise a "slow script dialog"
//       notification can pop up, and can cause the browser to stop executing
//       the page.
bool emscripten_lock_busyspin_wait_acquire(emscripten_lock_t * _Nonnull lock, double maxWaitMilliseconds);

// Similar to emscripten_lock_wait_acquire(), but instead of placing the calling
// thread to sleep until the lock can be acquired, this function will burn CPU
// cycles indefinitely until the given lock can be acquired.
// This function can be called in both main thread and in Workers.
// NOTE: The only way to abort this wait is to call
//       emscripten_terminate_wasm_worker() on the Worker. If called on the main
//       thread, and the lock cannot be acquired within a reasonable time
//       period, this function will *HANG* the browser page content process, and
//       show up a "slow script dialog", and/or cause the browser to stop the
//       page. If you call this function on the main browser thread, be extra
//       careful to analyze that the given lock will be extremely fast to
//       acquire without contention from other threads.
void emscripten_lock_busyspin_waitinf_acquire(emscripten_lock_t * _Nonnull lock);

// Registers an *asynchronous* lock acquire operation. The calling thread will
// asynchronously try to obtain the given lock after the calling thread yields
// back to the event loop. If the attempt is successful within
// maxWaitMilliseconds period, then the given callback asyncWaitFinished is
// called with waitResult == ATOMICS_WAIT_OK. If the lock is not acquired within
// the timeout period, then the callback asyncWaitFinished is called with
// waitResult == ATOMICS_WAIT_TIMED_OUT.
// NOTE: Unlike function emscripten_lock_wait_acquire() which takes in the wait
// timeout parameter as int64 nanosecond units, this function takes in the wait
// timeout parameter as double millisecond units. See
// https://github.com/WebAssembly/threads/issues/175 for more information.
// NOTE: This function can be called in both main thread and in Workers.
// NOTE 2: This function will always acquire the lock asynchronously. That is,
//         the lock will only be attempted to acquire after current control flow
//         yields back to the browser, so that the Wasm call stack is empty.
//         This is to guarantee a uniform control flow. If you use this API in
//         a Worker, you cannot utilise an infinite loop programming model.
void emscripten_lock_async_acquire(emscripten_lock_t * _Nonnull lock,
                                   emscripten_async_wait_volatile_callback_t _Nonnull asyncWaitFinished,
                                   void *userData,
                                   double maxWaitMilliseconds);

// Attempts to acquire a lock, returning true if successful. If the lock is
// already held, this function will not sleep to wait until the lock is
// released, but immediately returns false.
// This function can be called on both main thread and in Workers.
bool emscripten_lock_try_acquire(emscripten_lock_t * _Nonnull lock);

// Unlocks the specified lock for another thread to access. Note that locks are
// extremely lightweight, there is no "lock owner" tracking: this function does
// not actually check whether the calling thread owns the specified lock, but
// any thread can call this function to release a lock on behalf of whichever
// thread owns it.  This function can be called on both main thread and in
// Workers.
void emscripten_lock_release(emscripten_lock_t * _Nonnull lock);

#define emscripten_semaphore_t volatile uint32_t

// Use with syntax emscripten_semaphore_t s = EMSCRIPTEN_SEMAPHORE_T_STATIC_INITIALIZER(num);
#define EMSCRIPTEN_SEMAPHORE_T_STATIC_INITIALIZER(num) ((int)(num))

void emscripten_semaphore_init(emscripten_semaphore_t * _Nonnull sem, int num);

// main thread, try acquire num instances, but do not sleep to wait if not
// available.
// Returns idx that was acquired or -1 if acquire failed.
int emscripten_semaphore_try_acquire(emscripten_semaphore_t * _Nonnull sem, int num);

// main thread, poll to try acquire num instances. Returns idx that was
// acquired. If you use this API in Worker, you cannot run an infinite loop.
void emscripten_semaphore_async_acquire(emscripten_semaphore_t * _Nonnull sem,
                                        int num,
                                        emscripten_async_wait_volatile_callback_t _Nonnull asyncWaitFinished,
                                        void *userData,
                                        double maxWaitMilliseconds);

// worker, sleep to acquire num instances. Returns idx that was acquired, or -1
// if timed out unable to acquire.
int emscripten_semaphore_wait_acquire(emscripten_semaphore_t * _Nonnull sem, int num, int64_t maxWaitNanoseconds);

// worker, sleep infinitely long to acquire num instances. Returns idx that was
// acquired.
int emscripten_semaphore_waitinf_acquire(emscripten_semaphore_t * _Nonnull sem, int num);

// Releases the given number of resources back to the semaphore. Note that the
// ownership of resources is completely conceptual - there is no actual checking
// that the calling thread had previously acquired that many resources, so
// programs need to keep check of their semaphore usage consistency themselves.
// Returns how many resources were available in the semaphore before the new
// resources were released back to the semaphore. (i.e. the index where the
// resource was put back to)
// [main thread or worker]
uint32_t emscripten_semaphore_release(emscripten_semaphore_t * _Nonnull sem, int num);

// Condition variable is an object that can be waited on, and another thread can
// signal, while coordinating an access to a related mutex.
#define emscripten_condvar_t volatile uint32_t

// Use with syntax emscripten_condvar_t cv = EMSCRIPTEN_CONDVAR_T_STATIC_INITIALIZER;
#define EMSCRIPTEN_CONDVAR_T_STATIC_INITIALIZER ((int)(0))

// Creates a new condition variable to the given memory location.
void emscripten_condvar_init(emscripten_condvar_t * _Nonnull condvar);

// Atomically performs the following:
// 1. releases the given lock. The lock should (but does not strictly need to)
//    be held by the calling thread prior to this call.
// 2. sleep the calling thread to wait for the specified condition variable to
//    be signaled.
// 3. once the sleep has finished (another thread has signaled the condition
//    variable), the calling thread wakes up and reacquires the lock prior to
//    returning from this function.
void emscripten_condvar_waitinf(emscripten_condvar_t * _Nonnull condvar, emscripten_lock_t * _Nonnull lock);

// Same as the above, except that an attempt to wait for the condition variable
// to become true is only performed for a maximum duration.
// On success (no timeout), this function will return true. If the wait times
// out, this function will return false. In this case,
// the calling thread will not try to reacquire the lock.
bool emscripten_condvar_wait(emscripten_condvar_t * _Nonnull condvar, emscripten_lock_t * _Nonnull lock, int64_t maxWaitNanoseconds);

// Asynchronously wait for the given condition variable to signal.
ATOMICS_WAIT_TOKEN_T emscripten_condvar_wait_async(emscripten_condvar_t * _Nonnull condvar,
                                                   emscripten_lock_t * _Nonnull lock,
                                                   emscripten_async_wait_callback_t _Nonnull asyncWaitFinished,
                                                   void *userData,
                                                   double maxWaitMilliseconds);

// Signals the given number of waiters on the specified condition variable.
// Pass numWaitersToSignal == EMSCRIPTEN_NOTIFY_ALL_WAITERS to wake all waiters
// ("broadcast" operation).
void emscripten_condvar_signal(emscripten_condvar_t * _Nonnull condvar, int64_t numWaitersToSignal);

#ifdef __cplusplus
}
#endif
