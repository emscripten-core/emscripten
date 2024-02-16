/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "pthread_impl.h"
#include <emscripten/threading.h>
#include <emscripten/heap.h>

static bool enabled = false;

#ifndef NDEBUG

void _emscripten_thread_profiler_init(pthread_t thread) {
  assert(thread);
  if (!enabled) {
    return;
  }
  thread->profilerBlock = emscripten_builtin_malloc(sizeof(thread_profiler_block));
  memset(thread->profilerBlock, 0, sizeof(thread_profiler_block));
  thread->profilerBlock->currentStatusStartTime = emscripten_get_now();
}

// Sets the current thread status, but only if it was in the given expected
// state before. This is used to allow high-level control flow "override" the
// thread status before low-level (futex wait) operations set it.
static void set_status_conditional(int expectedStatus, int newStatus) {
  if (!enabled) {
    return;
  }
  pthread_t thread = pthread_self();
  if (!thread) return; // AudioWorklets do not have a pthread block, but if user calls emscripten_futex_wait() in an AudioWorklet, it will call here via emscripten_set_current_thread_status().
  int prevStatus = thread->profilerBlock->threadStatus;

  if (prevStatus != newStatus && (prevStatus == expectedStatus || expectedStatus == -1)) {
    double now = emscripten_get_now();
    double startState = thread->profilerBlock->currentStatusStartTime;
    double duration = now - startState;

    thread->profilerBlock->timeSpentInStatus[prevStatus] += duration;
    thread->profilerBlock->threadStatus = newStatus;
    thread->profilerBlock->currentStatusStartTime = now;
  }
}

void emscripten_conditional_set_current_thread_status(int expectedStatus, int newStatus) {
  set_status_conditional(expectedStatus, newStatus);
}

void emscripten_set_current_thread_status(int newStatus) {
  set_status_conditional(-1, newStatus);
}

void _emscripten_thread_profiler_enable() {
  enabled = true;
  _emscripten_thread_profiler_init(pthread_self());
  emscripten_set_thread_name(pthread_self(), "Browser main thread");
}

#endif

void emscripten_set_thread_name(pthread_t thread, const char* name) {
  if (!enabled) {
    return;
  }
  strncpy(thread->profilerBlock->name, name, EM_THREAD_NAME_MAX-1);
}
