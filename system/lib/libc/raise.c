/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE // for sighandler_t
#include <stdbool.h>
#include <stddef.h>
#include <signal.h>

extern struct sigaction __sig_actions[_NSIG];
extern sigset_t __sig_pending;

bool __sig_is_blocked(int sig);

extern void __call_sighandler(sighandler_t handler, int sig);

int raise(int sig) {
  if (__sig_is_blocked(sig)) {
    sigaddset(&__sig_pending, sig);
    return 0;
  }
  if (__sig_actions[sig].sa_flags & SA_SIGINFO) {
    siginfo_t t = {0};
    __sig_actions[sig].sa_sigaction(sig, &t, NULL);
  } else {
    if (__sig_actions[sig].sa_handler == SIG_DFL || __sig_actions[sig].sa_handler == SIG_IGN) {

      return 0;
    }
    // Avoid a direct call to the handler, and instead call via JS so we can
    // avoid strict signature checking.
    // https://github.com/emscripten-core/posixtestsuite/issues/6
    __call_sighandler(__sig_actions[sig].sa_handler, sig);
  }
  return 0;
}
