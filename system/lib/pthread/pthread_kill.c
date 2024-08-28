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
#include "lock.h"

void do_raise(void* arg) {
  int sig = (intptr_t)arg;
  if (sig == SIGCANCEL) {
    // For SIGCANCEL is simply enough to run the proxied function since we call
    // pthread_testcancel at the end of _emscripten_check_mailbox.  We
    // can't/don't call pthread_testcancel or pthread_exit here because the
    // proxying system itself is on the stack and we want to exit in clean
    // state. (e.g. without holding any locks).
    _emscripten_runtime_keepalive_clear();
    return;
  }
  raise((intptr_t)sig);
}

int pthread_kill(pthread_t t, int sig) {
  if (sig < 0 || sig >= _NSIG) {
    return EINVAL;
  }
  if (t == emscripten_main_runtime_thread_id()) {
    if (sig == 0) return 0; // signal == 0 is a no-op.
    return ESRCH;
  }
  if (!t || !_emscripten_thread_is_valid(t)) {
    return ESRCH;
  }
  if (sig == 0) return 0; // signal == 0 is a no-op.

  // The job of pthread_kill is basically the run the (process-wide) signal
  // handler on the target thread.
  emscripten_proxy_async(emscripten_proxy_get_system_queue(), t, do_raise, (void*)(intptr_t)sig);
  return 0;
}
