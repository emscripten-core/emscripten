/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <threading_internal.h>
#include <emscripten/proxying.h>
#include <emscripten/console.h>
#include <emscripten_internal.h>

#include "pthread_impl.h"

static void proxied_raise(void* arg) {
  raise((intptr_t)arg);
}

int pthread_kill(pthread_t t, int sig) {
  if (sig < 0 || sig >= _NSIG) {
    return EINVAL;
  }
  if (!t || !_emscripten_thread_is_valid(t)) {
    return ESRCH;
  }
  if (sig == 0) return 0; // signal == 0 is a no-op.

  // The job of pthread_kill is basically to run the (process-wide) signal
  // handler on the target thread.
  if (pthread_equal(pthread_self(), t)) {
    raise(sig);
  } else {
    emscripten_proxy_async(emscripten_proxy_get_system_queue(), t, proxied_raise, (void*)(intptr_t)sig);
  }
  return 0;
}
