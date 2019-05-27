/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <assert.h>
#include <sys/timeb.h>

int main() {
  struct timeb tb;
  tb.timezone = 1;
  printf("*%d\n", ftime(&tb));
  assert(tb.time > 10000);
  assert(tb.timezone == 0);
  assert(tb.dstflag == 0);
  return 0;
}
