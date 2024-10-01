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
    // For `SIGCANCEL` there is no need to actually call raise to run the
    // handler function.  The calling thread (the one calling `pthread_cancel`)
    // will already have marked us as being cancelled.  All we need to do is
    // ensure that `pthread_testcancel` is eventually called and that will cause
    // this thread to exit.  We can't call `pthread_testcancel` here (since we
    // are being called from the proxy queue process and we don't want to leave
    // that in a bad state by unwinding).  Instead, we rely on
    // `pthread_testcancel` at the end of `_emscripten_check_mailbox`.  Before
    // we return, we do want to make sure we clear the keepalive state so that
    // the thread will exit even if it has a reason to stay alive.  TODO(sbc):
    // Is this the correct behaviour, should `pthread_cancel` instead wait for
    // threads to be done with outstanding work/event loops?
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

  // The job of pthread_kill is basically to run the (process-wide) signal
  // handler on the target thread.
  emscripten_proxy_async(emscripten_proxy_get_system_queue(), t, do_raise, (void*)(intptr_t)sig);
  return 0;
}
