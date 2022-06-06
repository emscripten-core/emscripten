/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/threading.h>

#include "pthread_impl.h"
#include "lock.h"

int __pthread_kill_js(pthread_t t, int sig);

int pthread_kill(pthread_t t, int sig) {
  if (sig < 0 || sig >= _NSIG) {
    return EINVAL;
  }
  if (t == emscripten_main_browser_thread_id()) {
    if (sig == 0) return 0; // signal == 0 is a no-op.
    return ESRCH;
  }
  if (!t || !_emscripten_thread_is_valid(t)) {
    return ESRCH;
  }
  if (sig == 0) return 0; // signal == 0 is a no-op.
  return __pthread_kill_js(t, sig);
}
