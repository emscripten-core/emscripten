/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <pthread.h>
#include "emscripten/threading.h"
#include "threading_internal.h"
#include "pthread_impl.h"

void _emscripten_thread_init(pthread_t ptr,
                             int is_main,
                             int is_runtime,
                             int can_block,
                             int default_stacksize,
                             int start_profiling) {
  __set_thread_state(ptr, is_main, is_runtime, can_block);
  // Set `__default_stacksize` just once when the main runtime thread is
  // started.  The value of `DEFAULT_PTHREAD_STACK_SIZE` is passed in here as
  // `default_stacksize`.
  if (is_runtime && default_stacksize) {
    __default_stacksize = default_stacksize;
  }
#ifndef NDEBUG
  if (start_profiling) {
    _emscripten_thread_profiler_enable();
  }
  emscripten_set_current_thread_status(EM_THREAD_STATUS_RUNNING);
#endif
}
