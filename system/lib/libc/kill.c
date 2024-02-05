/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int kill(pid_t pid, int sig) {
  if (pid == getpid()) {
    return raise(sig);
  }
  errno = EPERM;
  return -1;
}
