// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten.h>

int main() {
  emscripten_sleep(1);

  // We should not get here - the unwind will fail as we did not list all the
  // right functions - this function should be instrumented, but will not be.
  puts("We should not get here!");
  return 1;
}
