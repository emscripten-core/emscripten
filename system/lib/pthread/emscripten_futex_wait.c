/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "atomic.h"
#include "pthread_impl.h"
#include "threading_internal.h"

#include <assert.h>
#include <emscripten/threading.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

extern void* _emscripten_main_thread_futex;

static int futex_wait_main_browser_thread(volatile void* addr,
                                          uint32_t val,
                                          double timeout, bool cancelable) {
  // Atomics.wait is not available in the main browser thread, so simulate it
  // via busy spinning. Only the main browser thread is allowed to call into
  // this function. It is not thread-safe to be called from any other thread.
  assert(emscripten_is_main_browser_thread());

  double now = emscripten_get_now();
  double end = now + timeout;

  // Register globally which address the main thread is simulating to be waiting
  // on. When zero, the main thread is not waiting on anything, and on nonzero,
  // the contents of the address pointed by _emscripten_main_thread_futex tell
  // which address the main thread is simulating its wait on.  We need to be
  // careful of recursion here: If we wait on a futex, and then call
  // _emscripten_yield() below, that will call code that takes the proxying
  // mutex - which can once more reach this code in a nested call. To avoid
  // interference between the two (there is just a single
  // _emscripten_main_thread_futex at a time), unmark ourselves before calling
  // the potentially-recursive call. See below for how we handle the case of our
  // futex being notified during the time in between when we are not set as the
  // value of _emscripten_main_thread_futex.
  void* last_addr = a_cas_p(&_emscripten_main_thread_futex, 0, (void*)addr);
  // We must not have already been waiting.
  assert(last_addr == 0);

  while (1) {
#ifdef __EMSCRIPTEN_PTHREADS__
    if (cancelable && __pthread_self()->cancel) {
      __pthread_testcancel();
      return -ETIMEDOUT;
    }
#endif
    // Check for a timeout.
    now = emscripten_get_now();
    if (now > end) {
      // We timed out, so stop marking ourselves as waiting.
      last_addr = a_cas_p(&_emscripten_main_thread_futex, (void*)addr, 0);
      // The current value must have been our address which we set, or
      // in a race it was set to 0 which means another thread just allowed
      // us to run, but (tragically) that happened just a bit too late.
      assert(last_addr == addr || last_addr == 0);
      return -ETIMEDOUT;
    }
    // We are performing a blocking loop here, so we must handle proxied
    // events from pthreads, to avoid deadlocks.
    // Note that we have to do so carefully, as we may take a lock while
    // doing so, which can recurse into this function; stop marking
    // ourselves as waiting while we do so.
    last_addr = a_cas_p(&_emscripten_main_thread_futex, (void*)addr, 0);
    assert(last_addr == addr || last_addr == 0);
    if (last_addr == 0) {
      // We were told to stop waiting, so stop.
      break;
    }
    _emscripten_yield(now);

    // Check the value, as if we were starting the futex all over again.
    // This handles the following case:
    //
    //  * wait on futex A
    //  * recurse into _emscripten_yield(),
    //    which waits on futex B. that sets the _emscripten_main_thread_futex
    //    address to futex B, and there is no longer any mention of futex A.
    //  * a worker is done with futex A. it checks _emscripten_main_thread_futex
    //    but does not see A, so it does nothing special for the main thread.
    //  * a worker is done with futex B. it flips mainThreadMutex from B
    //    to 0, ending the wait on futex B.
    //  * we return to the wait on futex A. _emscripten_main_thread_futex is 0,
    //    but that is because of futex B being done - we can't tell from
    //    _emscripten_main_thread_futex whether A is done or not. therefore,
    //    check the memory value of the futex.
    //
    // That case motivates the design here. Given that, checking the memory
    // address is also necessary for other reasons: we unset and re-set our
    // address in _emscripten_main_thread_futex around calls to
    // _emscripten_yield(), and a worker could
    // attempt to wake us up right before/after such times.
    //
    // Note that checking the memory value of the futex is valid to do: we
    // could easily have been delayed (relative to the worker holding on
    // to futex A), which means we could be starting all of our work at the
    // later time when there is no need to block. The only "odd" thing is
    // that we may have caused side effects in that "delay" time. But the
    // only side effects we can have are to call
    // _emscripten_yield(). That is always ok to
    // do on the main thread (it's why it is ok for us to call it in the
    // middle of this function, and elsewhere). So if we check the value
    // here and return, it's the same is if what happened on the main thread
    // was the same as calling _emscripten_yield()
    // a few times before calling emscripten_futex_wait().
    if (__c11_atomic_load((_Atomic uint32_t*)addr, __ATOMIC_SEQ_CST) != val) {
      return -EWOULDBLOCK;
    }

    // Mark us as waiting once more, and continue the loop.
    last_addr = a_cas_p(&_emscripten_main_thread_futex, 0, (void*)addr);
    assert(last_addr == 0);
  }
  return 0;
}

int emscripten_futex_wait(volatile void *addr, uint32_t val, double max_wait_ms) {
  if ((((intptr_t)addr)&3) != 0) {
    return -EINVAL;
  }

  int ret;
  emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_RUNNING, EM_THREAD_STATUS_WAITFUTEX);

#ifdef __EMSCRIPTEN_PTHREADS__
  pthread_t self = __pthread_self();
  bool cancelable = self->canceldisable != PTHREAD_CANCEL_DISABLE;
#else
  bool cancelable = false;
#endif

  // For the main browser thread and audio worklets we can't use
  // __builtin_wasm_memory_atomic_wait32 so we have busy wait instead.
  if (!_emscripten_thread_supports_atomics_wait()) {
    ret = futex_wait_main_browser_thread(addr, val, max_wait_ms, cancelable);
    emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_WAITFUTEX, EM_THREAD_STATUS_RUNNING);
    return ret;
  }

  // -1 (or any negative number) means wait indefinitely.
  int64_t max_wait_ns = ATOMICS_WAIT_DURATION_INFINITE;
  if (max_wait_ms != INFINITY) {
    max_wait_ns = (int64_t)(max_wait_ms * 1e6);
  }

#ifdef __EMSCRIPTEN_PTHREADS__
  // When building with pthread support there are two conditions under which we
  // need to limit the amount of time we spend in atomic.wait.
  // 1. We are the main runtime thread.  In this case we need to be able to
  //    process proxied events from workers.  Note that this is not always
  //    the same as being the main browser thread.  For example, when running
  //    under node or when launching an emscripten-built program in a Web
  //    Worker.  In this case we limit our wait slices to 1ms intervals.
  // 2. When the current thread has async cancellation enabled.  In this case
  //    we limit the wait duration to 100ms intervals.
  int64_t wakeup_interval = 0;
  bool is_runtime_thread = emscripten_is_main_runtime_thread();
  if (is_runtime_thread) {
    // If the current thread is the main runtime thread then only wait in 1ms slices.
    wakeup_interval = 1 * 1000000;
  }
  else if (cancelable) {
    // If the current thread is async cancellable then only wait in 100ms slices.
    wakeup_interval = 100 * 1000000;
  }

  // When wakeup_interval is set, we use remainder_ns to track how many ns
  // remain of the intiial max_wait_ns.
  int64_t remainder_ns = 0;
  if (wakeup_interval) {
    remainder_ns = max_wait_ns;
    if (remainder_ns < 0) {
      max_wait_ns = wakeup_interval;
    } else {
      max_wait_ns = MIN(remainder_ns, wakeup_interval);
    }
  }

  do {
#endif
    // Pass 0 here, which means we don't have access to the current time in this
    // function.  This tells _emscripten_yield to call emscripten_get_now if (and
    // only if) it needs to know the time.
    _emscripten_yield(0);

#ifdef EMSCRIPTEN_DYNAMIC_LINKING
    // After the main thread queues dlopen events, it checks if the target threads
    // are sleeping.
    // If `sleeping` is set then the main thread knows that event will be
    // processed after the sleep (before any other user code).  In this case the
    // main thread does not wait for any kind of response form the thread.
    // If `sleeping` is not set then we know we should wait for the thread process
    // the queue, either from the call here directly after setting `sleeping` to
    // 1, or from another callsite (e.g. the one in `emscripten_yield`).
    if (!is_runtime_thread) {
      self->sleeping = 1;
      _emscripten_process_dlopen_queue();
    }
#endif
    ret = __builtin_wasm_memory_atomic_wait32((int*)addr, val, max_wait_ns);
#ifdef EMSCRIPTEN_DYNAMIC_LINKING
    if (!is_runtime_thread) {
      self->sleeping = 0;
      _emscripten_process_dlopen_queue();
    }
#endif
#ifdef __EMSCRIPTEN_PTHREADS__
    if (cancelable && ret == ATOMICS_WAIT_TIMED_OUT && self->cancel) {
      __pthread_testcancel();
      // If __pthread_testcancel does return here it means that canceldisable
      // must be set to PTHREAD_CANCEL_MASKED.  In this case we emulate the
      // behaviour of the futex syscall and return ECANCELLED here.
      // See pthread_cond_timedwait.c for the only use of this flag.
      emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_WAITFUTEX, EM_THREAD_STATUS_RUNNING);
      return -ECANCELED;
    }
    // If remainder_ns is negative it means we want wait forever, and we don't
    // need to decrement remainder_ns in that case.
    if (wakeup_interval && remainder_ns > 0) {
      remainder_ns -= wakeup_interval;
      if (remainder_ns <= 0) {
        break;
      }
      max_wait_ns = MIN(remainder_ns, wakeup_interval);
    }
  } while (wakeup_interval && ret == ATOMICS_WAIT_TIMED_OUT);
#endif

  emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_WAITFUTEX, EM_THREAD_STATUS_RUNNING);

  if (ret == ATOMICS_WAIT_NOT_EQUAL) {
    return -EWOULDBLOCK;
  }
  if (ret == ATOMICS_WAIT_TIMED_OUT) {
    return -ETIMEDOUT;
  }
  assert(ret == ATOMICS_WAIT_OK);
  return 0;
}
