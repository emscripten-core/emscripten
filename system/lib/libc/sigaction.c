/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include "libc.h"

struct sigaction __sig_actions[_NSIG];

int __sigaction(int sig, const struct sigaction *restrict sa, struct sigaction *restrict old) {
  if (sig < 0 || sig >= _NSIG) {
    errno = EINVAL;
    return -1;
  }

  if (old) {
    *old = __sig_actions[sig];
  }

  if (sa) {
    __sig_actions[sig] = *sa;
  }

  return 0;
}

weak_alias(__sigaction, sigaction);
