// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

int main() {
  int rtn;
  struct itimerval val;
  memset(&val, 0, sizeof(val));

  val.it_value.tv_sec = 1;
  rtn = setitimer(ITIMER_REAL, &val, NULL);

  // In standalone mode we cannot use timers, and error.
  assert(rtn == -1);
  assert(errno == ENOTSUP);

  printf("done\n");
  return 0;
}
