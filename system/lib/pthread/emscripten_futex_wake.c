/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <atomic.h>
#include <emscripten/threading.h>

// Stores the memory address that the main thread is waiting on, if any. If
// the main thread is waiting, we wake it up before waking up any workers.
void* _emscripten_main_thread_futex;

// Returns the number of threads (>= 0) woken up, or the value -EINVAL on error.
// Pass count == INT_MAX to wake up all threads.
int emscripten_futex_wake(volatile void *addr, int count) {
  if (!addr || (((intptr_t)addr) & 3) != 0 || count < 0) {
    return -EINVAL;
  }
  if (count == 0) {
    return 0;
  }

  // See if main thread is waiting on this address? If so, wake it up by
  // resetting its wake location to zero.  Note that this is not a fair
  // procedure, since we always wake main thread first before any workers, so
  // this scheme does not adhere to real queue-based waiting.
  int main_thread_woken = 0;
  if (a_cas_p(&_emscripten_main_thread_futex, (void*)addr, 0) == addr) {
    // The main browser thread must never try to wake itself up!
    assert(!emscripten_is_main_browser_thread());
    if (count != INT_MAX) {
      --count;
      main_thread_woken = 1;
      if (count <= 0) {
        return 1;
      }
    }
  }

  // Wake any workers waiting on this address.
  int ret = __builtin_wasm_memory_atomic_notify((int*)addr, count);
  assert(ret >= 0);
  return ret + main_thread_woken;
}
