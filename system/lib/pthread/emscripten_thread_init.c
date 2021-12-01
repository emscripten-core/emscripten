/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <pthread.h>
#include "emscripten/threading.h"
#include "threading_internal.h"

void __set_thread_state(pthread_t ptr, int is_main, int is_runtime, int can_block);

void _emscripten_thread_init(pthread_t ptr,
                             int is_main,
                             int is_runtime,
                             int can_block,
                             int start_profiling) {
  __set_thread_state(ptr, is_main, is_runtime, can_block);
#ifndef NDEBUG
  if (start_profiling) {
    _emscripten_thread_profiler_enable();
  }
  emscripten_set_current_thread_status(EM_THREAD_STATUS_RUNNING);
#endif
}
