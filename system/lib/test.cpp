// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <time.h>

int clock_gettime(clockid_t clk_id, struct timespec *tp) {
  printf("BAD\n");
  return 0;
}

/*
int main() {
  printf("ok\n");
}
*/

