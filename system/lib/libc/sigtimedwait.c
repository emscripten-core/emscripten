/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <signal.h>
#include <errno.h>
#include "syscall.h"
#include "libc.h"

extern sigset_t __sig_pending;

int sigtimedwait(const sigset_t *restrict mask, siginfo_t *restrict si, const struct timespec *restrict timeout) {
  for (int sig = 0; sig < _NSIG; sig++) {
    if (sigismember(mask, sig) && sigismember(&__sig_pending, sig)) {
      if (si) {
        siginfo_t t = {0};
        *si = t;
      }
      sigdelset(&__sig_pending, sig);
      return sig;
    }
  }

  errno = EINVAL;
  return -1;
}
