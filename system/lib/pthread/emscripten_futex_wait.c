/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <emscripten/threading.h>
#include <stdlib.h>
#include <stdio.h>
#include "atomic.h"
#include "threading_internal.h"

extern void* _emscripten_main_thread_futex;

int _emscripten_thread_supports_atomics_wait(void);

static int futex_wait_busy(volatile void *addr, uint32_t val, double timeout) {
  // Atomics.wait is not available in the main browser thread, so simulate it via busy spinning.
  double now = emscripten_get_now();
  double end = now + timeout;

  // Register globally which address the main thread is simulating to be
  // waiting on. When zero, the main thread is not waiting on anything, and on
  // nonzero, the contents of the address pointed by __emscripten_main_thread_futex
  // tell which address the main thread is simulating its wait on.
  // We need to be careful of recursion here: If we wait on a futex, and
  // then call _emscripten_yield() below, that will call code that takes the
  // proxying mutex - which can once more reach this code in a nested call. To
  // avoid interference between the two (there is just a single
  // __emscripten_main_thread_futex at a time), unmark ourselves before calling
  // the potentially-recursive call. See below for how we handle the case of our
  // futex being notified during the time in between when we are not set as the
  // value of __emscripten_main_thread_futex.
  void* last_addr = a_cas_p(&_emscripten_main_thread_futex, 0, (void*)addr);
  // We must not have already been waiting.
  assert(last_addr == 0);

  while (1) {
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
    _emscripten_yield();

    // Check the value, as if we were starting the futex all over again.
    // This handles the following case:
    //
    //  * wait on futex A
    //  * recurse into _emscripten_yield(),
    //    which waits on futex B. that sets the __emscripten_main_thread_futex address to
    //    futex B, and there is no longer any mention of futex A.
    //  * a worker is done with futex A. it checks __emscripten_main_thread_futex but does
    //    not see A, so it does nothing special for the main thread.
    //  * a worker is done with futex B. it flips mainThreadMutex from B
    //    to 0, ending the wait on futex B.
    //  * we return to the wait on futex A. __emscripten_main_thread_futex is 0, but that
    //    is because of futex B being done - we can't tell from
    //    __emscripten_main_thread_futex whether A is done or not. therefore, check the
    //    memory value of the futex.
    //
    // That case motivates the design here. Given that, checking the memory
    // address is also necessary for other reasons: we unset and re-set our
    // address in __emscripten_main_thread_futex around calls to
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
    if (__c11_atomic_load((_Atomic uintptr_t*)addr, __ATOMIC_SEQ_CST) != val) {
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

  _emscripten_yield();

  int ret;
  emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_RUNNING, EM_THREAD_STATUS_WAITFUTEX);

  // For threads that cannot block (i.e. the main browser thread) we can't use
  // __builtin_wasm_memory_atomic_wait32 so we call out the JS function that
  // will busy wait.
  if (!_emscripten_thread_supports_atomics_wait()) {
    ret = futex_wait_busy(addr, val, max_wait_ms);
    emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_WAITFUTEX, EM_THREAD_STATUS_RUNNING);
    return ret;
  }

  // -1 (or any negative number) means wait indefinitely.
  int64_t max_wait_ns = -1;
  if (max_wait_ms != INFINITY) {
    max_wait_ns = (int64_t)(max_wait_ms*1000*1000);
  }
  ret = __builtin_wasm_memory_atomic_wait32((int*)addr, val, max_wait_ns);
  emscripten_conditional_set_current_thread_status(EM_THREAD_STATUS_WAITFUTEX, EM_THREAD_STATUS_RUNNING);

  // memory.atomic.wait32 returns:
  //   0 => "ok", woken by another agent.
  //   1 => "not-equal", loaded value != expected value
  //   2 => "timed-out", the timeout expired
  if (ret == 1) {
    return -EWOULDBLOCK;
  }
  if (ret == 2) {
    return -ETIMEDOUT;
  }
  assert(ret == 0);
  return 0;
}
