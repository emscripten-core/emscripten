/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE // for sigorset/sigandset
#include <stdbool.h>
#include <threads.h>
#include <signal.h>
#include <errno.h>
#include "libc.h"

#define SST_SIZE (_NSIG/8/sizeof(long))

static thread_local sigset_t __sig_mask;
sigset_t __sig_pending;

static int siginvertset(sigset_t *dest, const sigset_t *src) {
  unsigned long i = 0, *d = (void*) dest, *s = (void*) src;
  for(; i < SST_SIZE; i++) d[i] = ~s[i];
  return 0;
}

bool __sig_is_blocked(int sig) {
  return sigismember(&__sig_mask, sig);
}

int pthread_sigmask(int how, const sigset_t *restrict set, sigset_t *restrict old) {
  if (old) {
    *old = __sig_mask;
  }

  switch (how) {
    case SIG_SETMASK:
      __sig_mask = *set;
      break;
    case SIG_BLOCK:
      sigorset(&__sig_mask, &__sig_mask, set);
      break;
    case SIG_UNBLOCK: {
      sigset_t tmp;
      siginvertset(&tmp, set);
      sigandset(&__sig_mask, &__sig_mask, &tmp);
      break;
    }
    default:
      return EINVAL;
  }

  // These two signals can never be blocked.
  sigdelset(&__sig_mask, SIGKILL);
  sigdelset(&__sig_mask, SIGSTOP);

  // Raise any pending signals that are now unblocked.
  for (int sig = 0; sig < _NSIG; sig++) {
    if (sigismember(&__sig_pending, sig) && !sigismember(&__sig_mask, sig)) {
      sigdelset(&__sig_pending, sig);
      raise(sig);
    }
  }

  return 0;
}

int sigpending(sigset_t *set) {
  *set = __sig_pending;
  return 0;
}
