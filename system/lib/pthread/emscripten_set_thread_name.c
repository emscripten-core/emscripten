/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <string.h>

#include <emscripten/proxying.h>

#include "threading_internal.h"
#include "pthread_impl.h"

#ifndef NDEBUG
// JS function for setting thread name within devtools.
void _emscripten_set_js_thread_name(const char* name);

static void set_js_thread_name(void* arg) {
  const char* name = (const char*)arg;
  _emscripten_set_js_thread_name(name);
}
#endif

void emscripten_set_thread_name(pthread_t thread, const char* name) {
  if (thread->profilerBlock) {
    strncpy(thread->profilerBlock->name, name, EM_THREAD_NAME_MAX-1);
  }
#ifndef NDEBUG
  if (thread == pthread_self()) {
    _emscripten_set_js_thread_name(name);
  } else {
    emscripten_proxy_sync(emscripten_proxy_get_system_queue(),
                          thread,
                          set_js_thread_name,
                          (void*)name);
  }
#endif
}
